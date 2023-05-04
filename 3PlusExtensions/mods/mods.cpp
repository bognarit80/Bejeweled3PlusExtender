#include "mods.h"
#include "gamefunctions.h"
#include "discordrpc.h"
#include "hwaccelfix.h"
#include "miscpatches.h"
#include "widescreen.h"
#include "playmenutoggle.h"
#include "extramodes.h"
#include "maxscore.h"
#include "zenaddons.h"
#include "sandboxmode.h"
#include "sandboxfunctions.h"
#include "sandboxcheats.h"
#include "changepaths.h"
#include <Extender/FuncInterceptor.h>
#include <iostream>
#include <json.hpp>
#include <fstream>
#include <Windows.h>

using namespace nlohmann;

//default config values

int cfgvalues::artRes = ArtRes::HIGH;
bool cfgvalues::wantDirectX8 = false;
bool cfgvalues::hasExtraModes = false;
bool cfgvalues::isFixless = false;
std::string cfgvalues::questPack = "default";
bool cfgvalues::isWide = true;

//advanced: not visible in launcher

bool cfgvalues::hasMaxScore = false;
std::string cfgvalues::lang = "en";
bool cfgvalues::wantDiscordRichPresence = true;
int cfgvalues::loadingOptimisation = 1;

namespace
{
    const char* cfgFileName = "config.json";
    const char* keyBindsFileName = "keyBinds.json";
    const char* crashFileName = "crash.txt";

    const json disableOptimisation = R"([ { "op": "add", "path": "/advanced/loadingOptimisation", "value": 0 } ])"_json;
    const json dontAskAgainOptimisation = R"([ { "op": "add", "path": "/advanced/loadingOptimisation", "value": -1 } ])"_json;

    ordered_json config;
} // namespace

//returns false if "settings" or "advanced" are not found, otherwise true with missing values falling back to default
static bool openConfig() 
{
    puts("Opening config...");

    std::ifstream ifs(cfgFileName);
    if (ifs.fail()) 
    {
        //default values, create the file, I know it's horrible practice but I really can't get json to work
        std::string content = R"({
  "settings": {
    "wantDirectX8": false,
    "isWidescreen": true,
    "hasExtraModes": false,
    "questPack": "Default",
    "isFixless": false,
    "artRes": 0
  },
  "advanced": {
    "hasMaxScore": true,
    "hasZenAddons": true,
    "wantExtendedWidescreen": false,
    "lang": "en",
    "wantDiscordRichPresence": true,
    "loadingOptimisation": 1
  },
  "sandbox": {
    "allowSpeedBonus": false,
    "wantCalmEffects": false,
    "forceSwaps": true,
    "hasCheats": true,
    "wantEliteTechnique": false,
    "hasLargeExplosions": false,
    "isTimedMode": false,
    "supportsReplays": true,
    "isBackgroundOnly": false,
    "music": "Default Music",
    "speedFactor": 1.0,
    "gameSpeed": 1.0
  },
  "avalanche": {
    "startingDropPerMove": 5,
    "levels": [12000, 84000, 266000, 866000, 2360000]
  },
  "balance": {
    "startingRollSpeedDivisor": 1200,
    "decPerLevel": 50,
    "decPerMaxLevel": 30,
    "levels": [50, 75, 100, 150, 200, 300, 400, 500, 650, 800, 1000, 1250, 1500, 1750, 2000, 2500]
  }
})";
        config = ordered_json::parse(content);
        std::fstream writer;
        writer.open(cfgFileName, std::ios_base::out);
        writer.write(content.data(), content.size());
        return true; //no need to open the file again and so
    }

    try
    {
        config = ordered_json::parse(ifs);
        auto& settings = config["settings"];

        cfgvalues::artRes = settings["artRes"].get<int>();
        if (cfgvalues::artRes != ArtRes::NORMAL && cfgvalues::artRes != ArtRes::ULTRA)
            cfgvalues::artRes = ArtRes::HIGH; //the code depends on knowing what artres is chosen, don't allow just any input
        cfgvalues::wantDirectX8 = settings["wantDirectX8"].get<bool>();
        cfgvalues::hasExtraModes = settings["hasExtraModes"].get<bool>();
        cfgvalues::isFixless = settings["isFixless"].get<bool>();
        cfgvalues::questPack = settings["questPack"].get<std::string>();
        for (auto& c : cfgvalues::questPack) c = std::toupper((unsigned char)c);
        cfgvalues::isWide = settings["isWidescreen"].get<bool>();

        //advanced: not visible in launcher
        auto& advanced = config["advanced"];
        cfgvalues::hasMaxScore = advanced["hasMaxScore"].get<bool>();
        cfgvalues::lang = advanced["lang"].get<std::string>();
        for (auto& c : cfgvalues::lang) c = std::tolower((unsigned char)c);
        cfgvalues::hasZenAddons = advanced["hasZenAddons"].get<bool>();
        cfgvalues::wantExtendedWidescreen = advanced["wantExtendedWidescreen"].get<bool>();
        cfgvalues::wantDiscordRichPresence = advanced["wantDiscordRichPresence"].get<bool>();
        cfgvalues::loadingOptimisation = advanced["loadingOptimisation"].get<int>();

        //sandbox configs
        auto& sandbox = config["sandbox"];
        cfgvalues::allowSpeedBonus = sandbox["allowSpeedBonus"].get<bool>();
        cfgvalues::wantCalmEffects = sandbox["wantCalmEffects"].get<bool>();
        cfgvalues::forceSwaps = sandbox["forceSwaps"].get<bool>();
        cfgvalues::hasCheats = sandbox["hasCheats"].get<bool>();
        cfgvalues::wantEliteTechnique = sandbox["wantEliteTechnique"].get<bool>();
        cfgvalues::hasLargeExplosions = sandbox["hasLargeExplosions"].get<bool>();
        cfgvalues::isTimedMode = sandbox["isTimedMode"].get<bool>();
        cfgvalues::supportsReplays = sandbox["supportsReplays"].get<bool>();
        cfgvalues::isBackgroundOnly = sandbox["isBackgroundOnly"].get<bool>();
        cfgvalues::music = sandbox["music"].get<std::string>();
        cfgvalues::speedFactor = sandbox["speedFactor"].get<float>();
        cfgvalues::gameSpeed = sandbox["gameSpeed"].get<float>();
        
        //avalanche configs
        auto& avalanche = config["avalanche"];
        extra_modes_cfg::avalancheLevelRequirements = avalanche["levels"].get<std::vector<int>>();
        extra_modes_cfg::avalancheStartingDrop = avalanche["startingDropPerMove"].get<int>();

        //balance configs
        auto& balance = config["balance"];
        extra_modes_cfg::balanceLevelRequirements = balance["levels"].get<std::vector<int>>();
        extra_modes_cfg::balanceStartingValue = balance["startingRollSpeedDivisor"].get<int>();
        extra_modes_cfg::balanceDecPerLevel = balance["decPerLevel"].get<int>();
        extra_modes_cfg::balanceDecPerMaxLevel = balance["decPerMaxLevel"].get<int>();
    }
    catch (const json::type_error& e)
    {
        std::string error = "";
        error = error + "Error parsing the " + cfgFileName + " file.\n\n" + e.what();
        MessageBox(0, error.c_str(), "Error", MB_ICONERROR);
        return false;
    }
    catch (json::parse_error& e)
    {
        std::string error = "";
        error = error + "Error parsing the " + cfgFileName + " file.\n";
        if (e.id == 101)
            error += "Make sure to end the value definition with a comma, but not for the last element in the object! Also, make sure objects also end with a comma, except for the last one.";
        error = error + "\n\n" + e.what() + "\n\nDefault values will be used.";
        MessageBox(0, error.c_str(), "Error", MB_ICONERROR);
        return false;
    }

    return true;
}

bool openKeyBindsFile()
{
    printf_s("Opening keybinds config...\n");
    std::ifstream ifs(keyBindsFileName);
    if (ifs.fail())
    {
        std::string error = "";
        error = error + "Couldn't find the " + keyBindsFileName + " file.\n\nIf you want to use hotkeys, you can generate the file with KeyMapping.exe tool.";
        MessageBox(0, error.c_str(), "Error", MB_ICONERROR);
        return false;
    }

    try
    {
        json keysconfig = json::parse(ifs);
        sandbox_cheats::keyBindsDict = keysconfig["KeyBinds"].get<std::map<std::string, int>>();

        auto kvp = sandbox_cheats::keyBindsDict.find("Toggle Extra Modes");
        if (kvp != sandbox_cheats::keyBindsDict.end())
        {
            playmenu_toggle::toggleKey = kvp->second;
        }
    }
    catch (const json::type_error& e)
    {
        std::string error = "";
        error = error + "Error parsing the " + keyBindsFileName + " file.\n\n" + e.what();
        MessageBox(0, error.c_str(), "Error", MB_ICONERROR);
        return false;
    }
    catch (json::parse_error& e)
    {
        std::string error = "";
        error = error + "Error parsing the " + keyBindsFileName + " file.\n";
        if (e.id == 101)
            error += "Make sure to end the value definition with a comma, but not for the last element in the object! Also, make sure objects also end with a comma, except for the last one.";
        error = error + "\n\n" + e.what() + "\n\nDefault values will be used.";
        MessageBox(0, error.c_str(), "Error", MB_ICONERROR);
        return false;
    }
    
    return true;
}

//pass the first line of crash.txt to check if the exception address matches one of those related to loading
static bool isLoadingOptimisationCrash(const std::string &line)
{
    return (line.find("432F07") != std::string::npos || 
        line.find("45ADE0") != std::string::npos || 
        line.find("74A101") != std::string::npos);
}

static void openCrashFile()
{
    printf_s("Opening %s to check the last crash...\n", crashFileName);
    std::ifstream ifs(crashFileName);
    if (ifs.fail())
        return;

    std::string line;
    std::getline(ifs, line);

    if (isLoadingOptimisationCrash(line))
    {
        int result = MessageBox(0, "It appears that the last crash has been caused by the loading optimisation. Do you want to turn it off?\n\nThis is recommended if the game crashes frequently or has glitched fonts.\n\nTo turn it back on, set loadingOptimisation to 1 in the config file, or -1 to not show this message again (or just press \"No\").", "Loading Optimisation", MB_YESNO);
        if (result == IDYES)
        {
            config = config.patch(disableOptimisation);
            std::string newJson = config.dump(2);
            std::fstream(cfgFileName, std::ios_base::trunc | std::ios_base::out).write(newJson.data(), newJson.size());
        }
        else if (result == IDNO)
        {
            config = config.patch(dontAskAgainOptimisation);
            std::string newJson = config.dump(2);
            std::fstream(cfgFileName, std::ios_base::trunc | std::ios_base::out).write(newJson.data(), newJson.size());
        }
    }
}

bool initMods(CodeInjection::FuncInterceptor* hook)
{
    printf_s("> Initializing Mods\n");

    if (!openConfig())
        printf_s("Failed to parse the config file! Default values may be used!\n");

    if (cfgvalues::hasCheats)
    {
        if (!openKeyBindsFile())
            printf_s("Failed to parse the keybinds file! Default values may be used!\n");
    }

    if (cfgvalues::loadingOptimisation == 1)
        openCrashFile();
    
    initchangePaths(hook);
    initDiscordRpc(hook);
    inithwAccelFix(hook);
    initMiscPatches(hook);
    initPlayMenuToggle(hook);
    initExtraModes(hook);
    initwidescreenFix(hook);
    initclassicMaxScore(hook);
    initZenAddons(hook);
    initSandboxModeProps(hook);
    initSandboxFunctions(hook);
    initSandboxCheats(hook);

    return true;
}
