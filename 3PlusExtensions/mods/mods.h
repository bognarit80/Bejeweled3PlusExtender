#ifndef __MODS_H_
#define __MODS_H_

#include "Extender/FuncInterceptor.h"
#include <string>
#include <map>

//Contains global variables whose value is taken from the config file. Not using a struct/class because inline asm wouldn't be able to read them.
namespace cfgvalues
{
	extern int artRes;
	extern bool wantDirectX8;
	extern bool isWide;
	extern bool hasExtraModes;
	extern bool isFixless;
	extern std::string questPack;

	//advanced: not visible in launcher

	extern bool hasMaxScore;
	extern bool hasZenAddons;
	extern bool wantExtendedWidescreen;
	extern std::string lang;
	extern bool wantDiscordRichPresence;
	extern int loadingOptimisation;

	//sandbox:

	extern bool allowSpeedBonus;
	extern bool wantCalmEffects;
	extern bool forceSwaps;
	extern bool hasCheats;
	extern bool wantEliteTechnique;
	extern bool hasLargeExplosions;
	extern bool isTimedMode;
	extern std::string music;
	extern bool supportsReplays;
	extern bool isBackgroundOnly;
	extern float speedFactor;
	extern float gameSpeed;
}

bool openKeyBindsFile();
bool initMods(CodeInjection::FuncInterceptor* hook);

#endif /* __MODS_H_ */
