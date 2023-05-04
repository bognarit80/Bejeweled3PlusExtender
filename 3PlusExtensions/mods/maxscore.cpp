#include "maxscore.h"

#include <Engine.h>
#include <Extender/util.h>
#include "gamefunctions.h"

namespace max_score 
{
    NAKEDDEF(zenDrawOverride) 
    {
        __asm 
        {
            mov eax, ecx; //gApp is in ecx
            mov eax, [eax + 0BE0h]; //pointer to board object
            mov eax, [eax]; //check the type of the board

            //original line
            mov     ecx, [ecx + 9E0h];

            cmp eax, ZenBoard; //check if we are zenmode
            je zen;

            imul    ecx, 0xCA; //align the score with the classic's pod
            jmp goback;

        zen:
            imul    ecx, 0x7D; //align the score with the zen's pod

        goback:
            push 0x6FDCAE;
            ret;
        }
    }
} // namespace max_score

void initclassicMaxScore(CodeInjection::FuncInterceptor* hook)
{
    inject_bytes(0x82A3B0, { 0xD0, 0xDA, 0x6F, 0x00 }); //replace classic mode's score draw with zen's
    inject_bytes(0x82C680, { 0xD0, 0xDA, 0x6F, 0x00 }); //replace sandbox mode's score draw with zen's

    inject_jmp(0x6FDCA5, reinterpret_cast<void*>(max_score::zenDrawOverride));
    puts("Classic/Sandbox MAX SCORE draw initialized successfully!");
}