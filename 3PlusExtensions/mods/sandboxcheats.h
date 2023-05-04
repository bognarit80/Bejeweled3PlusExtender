#pragma once

#include <map>
#include "Extender/FuncInterceptor.h"

namespace sandbox_cheats
{
	extern std::map<std::string, int> keyBindsDict;
}

void initSandboxCheats(CodeInjection::FuncInterceptor* hook);
