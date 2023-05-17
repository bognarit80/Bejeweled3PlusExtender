#include "miscpatches.h"

#include <Engine.h>
#include <Extender/util.h>
#include "mods.h"
#include "gamefunctions.h"
#include <windows.h>

namespace
{
    //the offset for bomb counters, same for all langs that aren't cn or jp
    int bombOffset = -19200;

    //the offset for doom gem draw for normal and high graphics, -47 and -34 respectively
    int doomOffset = 0;

    //the selected localised string for elite badges
    const wchar_t* elite = L"^FFFF00^Elite^FFAACC^";

    //a map of all localised strings for elite badge text, as I can't do it through PopLoc as easily
    const std::map<std::string, const wchar_t*> eliteBadgeLocalisationMap =
    {
        {"en", L"^FFFF00^Elite^FFAACC^"},
        {"bp", L"^FFFF00^Elite^FFAACC^"},
        {"cn", L"^FFFF00^精英^FFAACC^"},
        {"de", L"^FFFF00^Elite^FFAACC^"},
        {"es", L"^FFFF00^Honorífica^FFAACC^"},
        {"fr", L"^FFFF00^d'Élite^FFAACC^"},
        {"it", L"^FFFF00^Speciali^FFAACC^"},
        {"jp", L"^FFFF00^エリート^FFAACC^"},
        {"nl", L"^FFFF00^Elite^FFAACC^"},
        {"sv", L"^FFFF00^Elit^FFAACC^"}
    };

    bool profileExists(std::wstring* profileName)
    {
        wchar_t* buffer;
        size_t len;
        _wdupenv_s(&buffer, &len, L"APPDATA");
        if (!buffer)
            return false;
        std::wstring fullPath(buffer);
        fullPath.erase(fullPath.find(L"Roaming"));
        fullPath += L"Local\\PopCap Games\\Bejeweled3\\users\\";
        fullPath += *profileName;
        const wchar_t* path = fullPath.c_str();
        int attribs = GetFileAttributesW(path);

        return (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY));
    }
}

namespace misc_patches
{
    NAKEDDEF(DecrementAllDoomGemsOverride)
    {
        __asm
        {
            test dword ptr[eax + 228h], 100h; //check if doom
            jnz decrement;

            test dword ptr[eax + 228h], 20h; //check if match bomb
            jnz decrement;

            //isn't doom or matchbomb, go back to code where the next gem is retrieved
            push 0x72BC76;
            ret;

            //found it, go back to code where DecrementCounterGem is called
        decrement:
            push 0x72BC67;
            ret;
        }
    }

    NAKEDDEF(StampOverlayOverride)
    {
        __asm
        {
            mov edx, bombOffset;

            push 0x70F121;
            ret;
        }
    }

    NAKEDDEF(DrawDoomGemYOverride)
    {
        __asm
        {
            //scale offset with the artres because popcap forgor
            sub eax, doomOffset;

            mov[esp + 8], eax; //original code
            fild dword ptr[esp + 8];
            push 0x72E4DE;
            ret
        }
    }

    NAKEDDEF(DrawDoomGemXOverride)
    {
        __asm
        {
            //scale offset with the artres because popcap forgor
            sub eax, doomOffset;

            mov[esp + 8], eax; //original code
            fild dword ptr[esp + 8];
            push 0x72E559;
            ret
        }
    }

    NAKEDDEF(GetBadgeLevelNameOverride)
    {
        __asm
        {
            push 15h; //size

            //original code
            mov[esp + 20h], cx;
            xor edx, edx;

            push[elite];
            push 0x769510;
            ret;
        }
    }

    NAKEDDEF(DrawSpeedBonusTextOverride)
    {
        __asm
        {
            mov esi, [ebx];
            cmp esi, 0x82D424; //check if InfernoBoard
            jz good;

            push 96h; //default PopLoc string id for "SPEED +%" text
            jmp goback;

        good:
            push 265h; //PopLoc string ID for ice storm's "SPEED +%" text

        goback:
            push 0x74109D;
            ret;
        }
    }

    NAKEDDEF(ShowHintOverride)
    {
        __asm
        {
            cmp dword ptr[esi + 26A8h], 0;
            jz nocooldown;

            push 0x74DD74;
            ret;

            //original code
        nocooldown:
            push ebx;
            mov ebx, [esp + 1Ch];
            push 0x74DCE8;
            ret;
        }
    }

    NAKEDDEF(DialogKeyDownOverride)
    {
        __asm
        {
            cmp dword ptr[esp + 4], 20h; //check if we pressed space
            jz confirmed;

            cmp dword ptr[esp + 4], 0Dh; //check if we pressed enter
            jnz checkfurther;

        confirmed:
            cmp dword ptr[ecx + 0C8h], 0; //this->YesButton == NULL?
            jz checkfurther;

            mov     eax, [ecx + 0C8h];
            mov     ecx, [eax + 138h];
            mov     edx, [ecx];
            mov     eax, [eax + 0B8h];
            mov     edx, [edx + 8];
            mov     dword ptr[esp + 4], eax;
            jmp     edx; //call ButtonDepress with YesButton ID

        checkfurther:
            cmp     dword ptr[esp + 4], 1Bh; //check if we pressed esc, original code
            jnz goback;

            //returning further to skip the check for isEscapable,
            //really no reason why a dialog shouldn't be escapeable
            push 0x75ACF0; 
            ret;

            //not enter nor esc, don't do anything
        goback:
            push 0x75AD3C;
            ret;
        }
    }

    NAKEDDEF(CreateProfileOverride)
    {
        __asm
        {
            //preserve registers
            push ecx;
            push ebp;
            call profileExists;
            pop ebp;
            pop ecx;
            cmp al, 1;
            jz found;

            //not found, go back to original code
            mov esi, ecx;
            cmp[ebp + 14h], edi;
            push 0x70B1D9;
            ret;
            
        found:
            push 0x70B1EF;
            ret;
        }
    }

    NAKEDDEF(RenameProfileOverride)
    {
        __asm
        {
            push ecx;
            push edi;
            call profileExists;
            pop edi;
            pop ecx;
            xor edi, edi; //this line must execute in both cases
            cmp al, 1;
            jz found;

            shr ecx, 1Fh; //original code
            add ecx, edx;
            push 0x763C1F;
            ret;

        found:
            push 0x763E1E;
            ret;
        }
    }

} // namespace misc_patches

void initMiscPatches(CodeInjection::FuncInterceptor* hook)
{
    //fix for bomb counters being off-center, uses offsets below. chinese is default so don't apply any changes
    if (cfgvalues::lang != "cn")
        inject_jmp(0x70F11B, reinterpret_cast<void*>(misc_patches::StampOverlayOverride));

    //jp font has different offset
    if (cfgvalues::lang == "jp")
        bombOffset = -12800;
    else if (cfgvalues::artRes == ArtRes::ULTRA) //offset fix for ultra, not applied to jp, hence the else if
        bombOffset -= 9600;

    //fix for matchless swaps not decrementing normal match bombs
    inject_jmp(0x72BC5B, reinterpret_cast<void*>(misc_patches::DecrementAllDoomGemsOverride));

    if (cfgvalues::questPack != "DEFAULT" && cfgvalues::questPack != "QMP2")
        inject_bytes(0x7C9091, { 'e', 'm', 's' }); //QMP poker fix, QMP2 doesn't need it

    //fix for doomgem being off center on normal and high graphics
    switch (cfgvalues::artRes)
    {
    case ArtRes::ULTRA:
        doomOffset = 0;
        break;
    case ArtRes::NORMAL:
        doomOffset = 47;
        break;
    default:
        doomOffset = 34;
    }
    inject_jmp(0x72E4D6, reinterpret_cast<void*>(misc_patches::DrawDoomGemYOverride));
    inject_jmp(0x72E551, reinterpret_cast<void*>(misc_patches::DrawDoomGemXOverride));

    //add Elite text to badge level
    auto localisedString = eliteBadgeLocalisationMap.find(cfgvalues::lang);
    if (localisedString != eliteBadgeLocalisationMap.end())
    {
        elite = localisedString->second;
    }
    inject_jmp(0x769503, reinterpret_cast<void*>(misc_patches::GetBadgeLevelNameOverride));

    //redirect Ice Storm speed bonus text to a different PopLoc string
    inject_bytes(0x740E18, { 0x63, 0x02, 0x00, 0x00 }); //%d CHAIN for 1 chain
    inject_bytes(0x740BA2, { 0x64, 0x02, 0x00, 0x00 }); //%d CHAIN
    inject_jmp(0x741098, reinterpret_cast<void*>(misc_patches::DrawSpeedBonusTextOverride)); //S +%d

    //fix extra modes being uppercase in Records screen (taking the same string as PlayMenu)
    inject_bytes(0x69FC16, { 0x66, 0x02, 0x00, 0x00 }); //Match Bomb
    inject_bytes(0x69FCAB, { 0x67, 0x02, 0x00, 0x00 }); //Time Bomb

    //fix ads, always return false in ShouldShowAd
    inject_byte(0x473699, 0xEB);
    inject_byte(0x4736D9, 0xEB);
    inject_byte(0x762CE0, 0xEB); //loading ad specifically causes an infinite loop, skip the check
    
    //add cooldown to hint presses on keyboard too
    inject_jmp(0x74DCE3, reinterpret_cast<void*>(misc_patches::ShowHintOverride));

    //add enter to confirm dialogs
    inject_jmp(0x75ACE0, reinterpret_cast<void*>(misc_patches::DialogKeyDownOverride));

    //prevent profile clashing with Standard by checking the actual users folder before creating/renaming the profile
    inject_jmp(0x70B1D4, reinterpret_cast<void*>(misc_patches::CreateProfileOverride));
    inject_jmp(0x763C18, reinterpret_cast<void*>(misc_patches::RenameProfileOverride));

    puts("Misc Patches initialized successfully!");
}