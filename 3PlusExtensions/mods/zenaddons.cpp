#include "zenaddons.h"
#include "mods.h"

#include <Engine.h>
#include <Extender/util.h>

//default value
bool cfgvalues::hasZenAddons = true;

namespace
{
    //the modifiers applied to the sound when calm effects are enabled
    double calmVolumeModifier = 0.5;
    double calmPitchModifier = -2.0;
} // namespace

namespace zen_addons
{
    NAKEDDEF(ZenInitUIOverride)
    {
        __asm
        {
            cmp edx, 0x83A164; //check if we returned from Zen's InitUI function
            jnz goback;

            //call Board::InitUI after ZenBoard::InitUI, to init the replayBtn
            mov ecx, ebp;
            mov eax, 0x7354A0;
            call eax;

        goback:
            mov edi, [ebp + 3070h]; //original code
            push 0x73AF1F;
            ret;
        }
    }

    NAKEDDEF(ReplaySoundOverride)
    {
        __asm
        {
            mov     eax, [ebx];
            mov     edx, [eax + 1C4h];
            call    edx; //call WantCalmEffects() to firstly check if they are enabled

            test    al, al;
            jz      notCalm;

            sub     esp, 10h;
            fld     ds : calmPitchModifier;
            fstp    qword ptr[esp + 8];
            fld     calmVolumeModifier;
            fstp    qword ptr[esp + 0];
            mov     edx, ds:0x89C764; //move the SOUND_REPLAY_POPUP value to edx
            mov     eax, ds:0x8A67AC; //move the gApp value to eax
            push    0x74B651;
            ret;

        notCalm: //disabled, so return to original code
            fldz;
            mov     edx, ds:89C764h;
            push    0x74B63C;
            ret;
        }
    }
} // namespace zen_addons

void initZenAddons(CodeInjection::FuncInterceptor* hook)
{
    if (cfgvalues::hasZenAddons)
    {
        inject_jmp(0x73AF19, reinterpret_cast<void*>(zen_addons::ZenInitUIOverride));
        inject_jmp(0x74B634, reinterpret_cast<void*>(zen_addons::ReplaySoundOverride));

        inject_bytes(0x82BB88, { 0xA0 , 0xB8, 0x72 }); //ZenBoard::SupportsReplays = 1;

        puts("Zen Mode Addons initialized successfully!");
    }
    else
    {
        puts("Zen Mode Addons initialized, but are disabled in the config!");
    }
}