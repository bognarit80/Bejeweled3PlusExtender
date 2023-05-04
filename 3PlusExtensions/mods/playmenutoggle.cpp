#include "playmenutoggle.h"

#include <Engine.h>
#include <Extender/util.h>
#include "mods.h"

//The key that toggles between Secret and Extra modes when pressed, default is the / key next to left shift
unsigned int playmenu_toggle::toggleKey = 191;

namespace
{
    //while the S key is held, switch the secret modes to the extra modes
    bool isSecretKeyToggled = false;

    //all the save files, index 0 to 5 won't be hit here but are required for proper indexing in asm code
    const char* saveFileNames[] = { "poker.sav", "butterfly.sav", "ice_storm.sav", "diamond_mine.sav", "time_bomb.sav", "real_time_bomb.sav", "avala.sav", "balance_e.sav", "stratamax.sav", "sandbox_mode.sav", "time_bomb.sav", "real_time_bomb.sav" };
} // namespace

namespace playmenu_toggle
{
    NAKEDDEF(PlayMenuUpdateOverride)
    {
        __asm
        {
            add eax, 450h; //original code

            cmp isSecretKeyToggled, 0;
            jz goback;

            mov ecx, 0DDh; //red to 221
            mov edx, 70h; //green to 112

        goback:
            push 0x698AF6;
            ret;
        }
    }

    NAKEDDEF(PlayMenuUpdateOverrideFonts)
    {
        __asm
        {
            cmp isSecretKeyToggled, 0;
            jz goback;

            mov[eax], 0DDh;
            mov[eax + 4], 00h;
            mov[eax + 8], 088h;

        goback:
            mov edx, [eax]; //original code
            mov ecx, [ebp + esi * 4 + 0C4h];
            push 0x6988BF;
            ret;
        }
    }

    NAKEDDEF(WidgetManagerKeyDownOverride)
    {
        __asm
        {
            mov eax, [eax + 0A0h]; //original code

            test eax, eax; //if focusWidget is null, to fix going back to play menu
            jz checkkey;

            cmp[eax], 0x8332CC; //check if focusWidget is PlayMenu
            jnz goback;

        checkkey:
            cmp ecx, toggleKey; //check if the desired key was pressed
            jnz goback;

            xor isSecretKeyToggled, 1;

        goback:
            push 0x585F4F;
            ret;
        }
    }

    NAKEDDEF(GetSavedGameNameOverride)
    {
        __asm
        {
            cmp isSecretKeyToggled, 0;
            jz goback;

            cmp eax, 0Bh; //we added 6 more cases to switch, change default to any above 11
            ja goback;

            mov esi, [esp + 0Ch];
            push ds : saveFileNames[eax * 4]; //eax is the switch case, access the same element in the array
            push 0x6CA35B;
            ret;

        goback:
            mov esi, [esp + 0Ch]; //original code
            push edx;
            push 0x6CA36E;
            ret;
        }
    }

    NAKEDDEF(DoNewConfigGameOverride)
    {
        __asm
        {
            mov edi, [esp + 0C4h]; //original code

            cmp isSecretKeyToggled, 0;
            jz goback;

            cmp [ebx], 0x832FD4; //don't change if origin is QuestMenu
            jz goback;

            add edi, 6; //redirect the questid to the extra modes from secrep.cfg

        goback:
            push 0x765440;
            ret;
        }
    }
} // namespace playmenu_toggle

void initPlayMenuToggle(CodeInjection::FuncInterceptor* hook)
{
    if (!cfgvalues::isFixless && !cfgvalues::hasExtraModes) //we don't want the toggle to work on these two
    {
        inject_jmp(0x698AF1, reinterpret_cast<void*>(playmenu_toggle::PlayMenuUpdateOverride));
        inject_jmp(0x6988B6, reinterpret_cast<void*>(playmenu_toggle::PlayMenuUpdateOverrideFonts));
        inject_jmp(0x585F49, reinterpret_cast<void*>(playmenu_toggle::WidgetManagerKeyDownOverride));
        inject_jmp(0x6CA369, reinterpret_cast<void*>(playmenu_toggle::GetSavedGameNameOverride));
        inject_jmp(0x765439, reinterpret_cast<void*>(playmenu_toggle::DoNewConfigGameOverride));
	    puts("Play Menu Toggle initialized successfully!");
    }
    else
    {
        puts("Play Menu Toggle initialized, but isn't enabled because Extra Modes or Fixless are enabled!");
    }
}