#include "hwaccelfix.h"

#include <Engine.h>
#include <Extender/util.h>

void inithwAccelFix(CodeInjection::FuncInterceptor* hook)
{
    inject_byte(0x54B227, 0xEB); //fix vram issue
    inject_byte(0x54B464, 0xEB); //fix gpu check issue
    puts("3D Acceleration Fix initialized successfully!");
}