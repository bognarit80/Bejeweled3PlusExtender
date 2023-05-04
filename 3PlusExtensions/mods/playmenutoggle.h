#pragma once

#include "Extender/FuncInterceptor.h"

namespace playmenu_toggle 
{
	extern unsigned int toggleKey;
}

void initPlayMenuToggle(CodeInjection::FuncInterceptor* hook);