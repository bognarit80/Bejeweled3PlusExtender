#include "sandboxmode.h"

#include <Engine.h>
#include <Extender/util.h>
#include "mods.h"
#include "gamefunctions.h"

//default config values

bool cfgvalues::allowSpeedBonus = false;
bool cfgvalues::wantCalmEffects = false;
bool cfgvalues::forceSwaps = true;
bool cfgvalues::hasCheats = true;
bool cfgvalues::wantEliteTechnique = false;
bool cfgvalues::hasLargeExplosions = false;
bool cfgvalues::isTimedMode = false;
bool cfgvalues::supportsReplays = true;
bool cfgvalues::isBackgroundOnly = false;
std::string cfgvalues::music = "Default Music";
float cfgvalues::speedFactor = 1.0;
float cfgvalues::gameSpeed = 1.0;

namespace
{
    //An array of strings passed to the BASS command parser, required to be in wstring format
    const std::wstring musicsArray[] = { L"Classic", L"Zen", L"Speed", L"Speed_lose", L"Poker", L"Butterflies", L"Icestorm", L"BuriedTreasure", L"QuestTimeBased", L"QuestTurnBased", L"QuestBomb", L"LoadingScreen", L"MainMenu", L"QuestMenu", L"QuestMenu_fanfare" };
    std::wstring theSong = L"QuestTurnBased"; //default music for this quest type
    //Value changes to true when the player enters Sandbox mode, and back to false once dialog has been displayed.
    bool wantWarningDialog = false;

    void showSpeedWarningDialog()
    {
        if (wantWarningDialog && cfgvalues::gameSpeed > 1.1 && !cfgvalues::isFixless)
        {
            std::wstring header(L"WARNING");
            std::wstring warningText(L"GameSpeed is set too high and will probably freeze your game. Use values lower than 1.1 or better, use SpeedFactor to control the speed of the board.");
            std::wstring okButton(L"OK");
            DoDialog(GetGApp(), 0, true, &header, &warningText, &okButton, 3);
        }
        wantWarningDialog = false;
    }
}

namespace sandboxMode 
{
    NAKEDDEF(SpeedModeFactorOverride)
    {
        __asm
        {
            cmp edx, 0x82C284; //check if we're sandbox mode
            jz change;

            fld1;  //we aren't, so set the speed to 1 (default)
            jmp goback;

        change:
            fld cfgvalues::speedFactor;  //we are, so set the speed to desired

        goback:
            push 0x72AA86;
            ret;
        }
    }

    NAKEDDEF(GameSpeedOverride)
    {
        __asm
        {
            cmp edx, 0x82C284; //check if we're sandbox mode
            jz change;

            fld1; //we aren't, so set speed to default
            jmp goback;

        change:
            fld cfgvalues::gameSpeed;

        goback:
            push 0x72AA96;
            ret;
        }
    }

    NAKEDDEF(GetMusicNameOverride)
    {
        __asm
        {
            mov esi, [esp + 0Ch];
            mov ecx, [ecx];
            cmp ecx, 0x82C284;
            jnz goback;

            mov ecx, esi;
#ifdef _DEBUG
            push [theSong + 4]; //std::wstring seems to construct differently on debug vs release
#else
            push [theSong];
#endif // _DEBUG
            push 0x6CA293;
            ret;

        goback:
            mov ecx, esi;
            push 0x6CA28A;
            ret;
        }
    }

    NAKEDDEF(UpdateSwappingOverride)
    {
        __asm
        {
            mov eax, [ebx];
            cmp eax, 0x82C284; //check if sandbox
            jz skip;

            //original code
            inc dword ptr[edi + 0F4h];
            push 0x74BFA9;
            ret;

        skip:
            push 0x74C006; //go back to where mHoldingSwap is set to 0
            ret;
        }
    }

    NAKEDDEF(SetupBackgroundOverride)
    {
        __asm
        {
            mov esi, [ebp];
            cmp esi, 0x82C284;
            jz sandbox; //check if sandbox, only push the background on sandbox mode

            mov dword ptr[eax + 60h], 0FFFFFFFFh; //ZOrder to -1, default
            jmp goback;

        sandbox:
            mov dword ptr[eax + 60h], 3; //ZOrder to int32_max only on sandbox

        goback:
            push 0x73A3C7;
            ret;
        }
    }

    NAKEDDEF(DoNewSandboxGameOverride)
    {
        __asm
        {
            mov wantWarningDialog, 1;

            push 32B8h; //original line
            push 0x76586C;
            ret;
        }
    }

    NAKEDDEF(DoNewConfigGameOverride)
    {
        __asm
        {
            call showSpeedWarningDialog;

            mov ecx, [esp + 0B0h]; //original line
            push 0x765BAB;
            ret;
        }
    }
}

void initSandboxModeProps(CodeInjection::FuncInterceptor* hook)
{
    //search for our music name in the array, if not found, default is QuestTurnBased
    for (std::wstring wstr : musicsArray)
    {
        std::wstring cfgMusic(cfgvalues::music.begin(), cfgvalues::music.end());
        if (wstr == cfgMusic) 
        {
            theSong = wstr;
            break;
        }
    }
    inject_jmp(0x6CA284, reinterpret_cast<void*>(sandboxMode::GetMusicNameOverride));


    if (cfgvalues::allowSpeedBonus)
    {
        //AllowSpeedBonus
        inject_bytes(0x82C3F0, { 0xA0, 0xB8, 0x72 });
    }

    if (cfgvalues::wantCalmEffects) 
    {
        //WantsCalmEffects
        inject_bytes(0x82C448, { 0xA0, 0xB8, 0x72 });
    }
    
    if (cfgvalues::forceSwaps)
    {
        //ForceSwaps
        inject_bytes(0x82C408, { 0xA0, 0xB8, 0x72 });
    }

    if (cfgvalues::hasLargeExplosions)
    {
        //HasLargeExplosions
        inject_bytes(0x82C404, { 0xA0, 0xB8, 0x72 });
    }

    if (cfgvalues::isTimedMode)
    {
        //GetTimeLimit
        inject_bytes(0x82C424, { 0xA0, 0xB8, 0x72 });
    }

    if (cfgvalues::supportsReplays)
    {
        //SupportsReplays
        inject_bytes(0x82C440, { 0xA0, 0xB8, 0x72 });
    }

    if (cfgvalues::hasCheats || cfgvalues::gameSpeed != 1.0 || cfgvalues::speedFactor != 1.0)
    {
        inject_jmp(0x72AA80, reinterpret_cast<void*>(sandboxMode::SpeedModeFactorOverride));
        inject_byte(0x72AA86, 0xC3); //put a ret instruction because our jmp has overridden the existing one

        inject_jmp(0x72AA90, reinterpret_cast<void*>(sandboxMode::GameSpeedOverride));
        inject_byte(0x72AA96, 0xC3); //put a ret instruction because our jmp has overriden the existing one
    }

    inject_jmp(0x765867, reinterpret_cast<void*>(sandboxMode::DoNewSandboxGameOverride));
    inject_jmp(0x765BA4, reinterpret_cast<void*>(sandboxMode::DoNewConfigGameOverride));
    
    if (cfgvalues::wantEliteTechnique) 
    {
        inject_jmp(0x74BFA3, reinterpret_cast<void*>(sandboxMode::UpdateSwappingOverride));
        //inject_byte(0x74BFB3, 0xEB); //skip a check if swap has been held for more than 400 ticks
    }

    if (cfgvalues::isBackgroundOnly)
    {
        inject_jmp(0x73A3C0, reinterpret_cast<void*>(sandboxMode::SetupBackgroundOverride));
    }
   
    //these two are always enabled, no reason to disable them really

    //WantAnnihalatorReplacement
    inject_bytes(0x82C43C, { 0xA0, 0xB8, 0x72 });

    //WantsTutorialReplays
    inject_bytes(0x82C44C, { 0xA0, 0xB8, 0x72 });

    //GemCountPopups
    inject_bytes(0x82C650, { 0x60, 0x3B, 0x74 });

    //the rest don't make much sense

    //WantLevelBasedBackground
    /*inject_byte(0x82C414, 0xA0);
    inject_byte(0x82C415, 0xB8);
    inject_byte(0x82C416, 0x72);*/

    //WantDrawScore
    /*inject_byte(0x82C604, 0x30);
    inject_byte(0x82C605, 0x99);
    inject_byte(0x82C606, 0x40);*/

    //WantCountdownBar
    /*inject_byte(0x82C60C, 0x30);
    inject_byte(0x82C60D, 0x99);
    inject_byte(0x82C60E, 0x40);*/

    //GetMusicName
    //inject_byte(0x82C3E8, 0x30);
    //inject_byte(0x82C3E9, 0x16);
    //inject_byte(0x82C3EA, 0x73);

    printf("Sandbox Mode Configs initialized successfully!\n");
}