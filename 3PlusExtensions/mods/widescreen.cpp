#include "widescreen.h"

#include <Engine.h>
#include <Extender/util.h>
#include "mods.h"
#include "gamefunctions.h"

//default value;
bool cfgvalues::wantExtendedWidescreen = false;

namespace
{
    //default values for 1200 artres

    //the offsets applied to backgrounds to align properly to extended resolution
    int theY = -68;
    int theXDiff = 260;

    //added to the background width and height to extend to 16:9
    int theWidth = 220;
    int theHeight = 138;
}

//extend the window created upon startup
static void setWide()
{
    if (cfgvalues::artRes == ArtRes::NORMAL)
    {
        if (cfgvalues::wantExtendedWidescreen)
        {
            inject_bytes(0x5C3BAD, { 0x2A, 0x04 }); //width
            inject_bytes(0x5C3BB5, { 0x58, 0x02 }); //height
        }
        else
        {
            inject_bytes(0x5C3BAD, { 0xC0, 0x03 });
            inject_bytes(0x5C3BB5, { 0x58, 0x02 });
        }
    }
    else
    {
        if (cfgvalues::wantExtendedWidescreen)
            inject_bytes(0x5C3BAD, { 0x56, 0x05 }); //width
        else
            inject_bytes(0x5C3BAD, { 0xCC, 0x04 }); 
    }
}

namespace widescreen_fix
{
    NAKEDDEF(Bej3AppOverride)
    {
        __asm
        {
            mov     byte ptr[ebp + 708h], 1; //mWidescreenAware = true
            mov     byte ptr[ebp + 738h], 1; //mAllowWindowResize = true
            mov     byte ptr[ebp + 6D5h], 1; //mEnableMaximizeWindow = true

            push 0x75EF51;
            ret;
        }
    }
    NAKEDDEF(HandleCmdLineParamOverride)
    {
        __asm
        {
            //original code
            mov     byte ptr[edi + 71Ch], 1;

            //extend the window created upon startup, called only if -wide is present
            call setWide;

            push 0x762446;
            ret;
        }
    }

    NAKEDDEF(BackgroundConstructorOverride)
    {
        __asm
        {
            fst qword ptr[edi + 140h]; //original line
            mov[edi + 145h], 0D0h; //change the DrawScale to around 1.1
            mov[edi + 146h], 0F1h;

            //go back to original code
            push 0x76F380;
            ret;
        }
    }

    NAKEDDEF(BackgroundDrawOverride)
    {
        __asm
        {
            mov     edx, [edi + 0A0h]; //original line
            push    theY;

            //go back to original code
            push 0x73A427;
            ret;
        }
    }

    NAKEDDEF(FlatBackgroundDrawOverride)
    {
        __asm
        {
            push ecx;
            mov ecx, theWidth;
            add[esp], ecx;
            mov ecx, theHeight;
            add[esp + 4], ecx;
            push 0;
            push 0;
            push 0x770747;
            ret;
        }
    }

    NAKEDDEF(MainMenuDrawOverride)
    {
        __asm
        {
            mov esi, [ecx + 4];
            mov edi, 0FFFFFEFCh;

            push 0x6A36E5;
            ret;
        }
    }

    NAKEDDEF(BadgeAwardBackgroundDrawOverride)
    {
        __asm
        {
            //original line
            mov     ecx, [esp + 94h];
            add ecx, theXDiff;

            push 0x689149;
            ret;
        }
    }
} // namespace widescreen_fix

void initwidescreenFix(CodeInjection::FuncInterceptor* hook)
{
    inject_jmp(0x75EF3E, reinterpret_cast<void*>(widescreen_fix::Bej3AppOverride));
    inject_jmp(0x76243F, reinterpret_cast<void*>(widescreen_fix::HandleCmdLineParamOverride));
    
    if (cfgvalues::wantExtendedWidescreen)
    {
        if (!cfgvalues::isWide)
            return;

        theY = -68 * cfgvalues::artRes / 1200;
        theXDiff = 260 * cfgvalues::artRes / 1200;

        theWidth = 220 * cfgvalues::artRes / 1200;
        theHeight = 138 * cfgvalues::artRes / 1200;

        inject_jmp(0x76F37A, reinterpret_cast<void*>(widescreen_fix::BackgroundConstructorOverride));
        inject_byte(0x73A3D1, 0xF5); //Height of Background widget to 1269
        inject_bytes(0x73A3EB, { 0xF5, 0xFE }); //X of background widget to -267
        inject_bytes(0x73A3F1, { 0x2A, 0x09 }); //width of Background widget to 2346
        inject_jmp(0x73A420, reinterpret_cast<void*>(widescreen_fix::BackgroundDrawOverride)); //Y of widget to -68
        inject_byte(0x7623FA, 0x38); //extend the game to 16:9
        inject_bytes(0x67F388, { 0xE9, 0x08, 0x0A, 0x00 }); //disable the under_dialog effect for 16:9, it crashes the game

        //fully dim the screen on special gem detonation
        inject_bytes(0x748A5B, { 0x92, 0x0A, 0x00, 0x00 });
        inject_bytes(0x748A8B, { 0x95, 0xFF, 0xFF, 0xFF });

        //w 2137.5 h 1336
        //16:9 flat bg
        inject_jmp(0x770742, reinterpret_cast<void*>(widescreen_fix::FlatBackgroundDrawOverride));

        inject_bytes(0x6A36EF, { 0x2A, 0x09, 0x00, 0x00 });
        inject_jmp(0x6A36E0, reinterpret_cast<void*>(widescreen_fix::MainMenuDrawOverride));

        //badge award dim the whole screen
        inject_bytes(0x689114, { 0x2A, 0x09, 0x00, 0x00 });
        inject_jmp(0x689142, reinterpret_cast<void*>(widescreen_fix::BadgeAwardBackgroundDrawOverride));

        //instant replay flash back to present extend to 16:9
        inject_bytes(0x74956D, { 0xF5, 0xFE, 0xFF, 0xFF }); //X to -267
        inject_bytes(0x749573, { 0x92, 0x0A, 0x00, 0x00 });

    }
    printf_s("Widescreen Fix initialized successfully!%s\n", cfgvalues::wantExtendedWidescreen ? " Using 16:9 resolution." : "");
}
