#ifndef _EXTRAMODES__
#define _EXTRAMODES__

#include "Extender/FuncInterceptor.h"

//Contains extra modes data read from config file. Not using a struct/class because inline asm wouldn't be able to read them.
namespace extra_modes_cfg
{
	extern int avalancheStartingDrop;
	extern std::vector<int> avalancheLevelRequirements;

	extern int balanceStartingValue;
	extern int balanceDecPerLevel;
	extern int balanceDecPerMaxLevel;
	extern std::vector<int> balanceLevelRequirements;
}

void initExtraModes(CodeInjection::FuncInterceptor* hook);

#endif // !_EXTRAMODES__

