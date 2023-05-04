#include <Engine.h>
#include <Extender/util.h>
#include "../discord/discord.h"
#include "discordrpc.h"
#include "mods.h"
#include <memory>
#include <csignal>
#include <thread>
#include "gamefunctions.h"

namespace {
    volatile bool interrupted{ false };

    //A const map linking the quest pack codenames to their full names.
    const std::map<std::string, const char*> longQmpNames =
    {
        {"DEFAULT", "Default Quest Pack"},
        {"QMP2", "Quest Mode Plus II"},
        {"QMP3", "Quest Mode Plus III"},
        {"QMP4E", "Quest Mode Plus IV - Easy"},
        {"QMP4N", "Quest Mode Plus IV - Normal"},
        {"QMP4H", "Quest Mode Plus IV - Hard"},
        {"QMP4L", "Quest Mode Plus IV - Lunatic"}
    };
} // namespace

static char* formatNumber(int theNumber, char *outBuffer)
{
    char tempBuffer[14];

    if (theNumber > 1000)
    {
        formatNumber(theNumber / 1000, outBuffer);
        sprintf_s(tempBuffer, ",%03d", theNumber % 1000);
        strcat_s(outBuffer, 14, tempBuffer);
    }
    else
    {
        sprintf_s(tempBuffer, "%d", theNumber);
        strcpy_s(outBuffer, 4, tempBuffer);
    }

    return outBuffer;
}


static void discordThread()
{
    printf_s("Initialising Discord Core... ");
    std::unique_ptr<discord::Core> corePtr;
    discord::Core* core{};
    auto result = discord::Core::Create(1091570153076039700, DiscordCreateFlags_NoRequireDiscord, &core);
    corePtr.reset(core);
    if (!corePtr) {
        printf_s("Failed! (error %d)\n", static_cast<int>(result));
        return;
    }
    printf_s("Succeeded!\n");

    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);
    activity.GetTimestamps().SetStart(time(0));

    std::signal(SIGINT, [](int) { interrupted = true; });
    std::signal(SIGABRT, [](int) { interrupted = true; });
    std::signal(SIGTERM, [](int) { interrupted = true; });
    char detailsText[64];
    char stateText[64];
    char smallImage[32];
    char smallText[32];
    char questName[64];
    char scoreBuffer[16];
    char totalTimeBuffer[32];
    size_t i;

    do {
        if (!GetGApp())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            continue; //we're still loading, don't show presence
        }
        uintptr_t theBoard = GetBoard();
        BoardType boardType = GetBoardType(theBoard);

        //board is already constructed when the savegame prompt appears, don't show details in that case
        if (!GetBoardBackground(theBoard))
            boardType = UnknownBoard; 

        switch (boardType)
        {
        case ClassicBoard:
            sprintf_s(detailsText, "Classic: Level %d", GetLevel(theBoard));
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "classic_icon");
            strcpy_s(smallText, "Classic Mode");
            break;
        case ZenBoard:
            sprintf_s(detailsText, "Zen: Level %d", GetLevel(theBoard));
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "zen_icon");
            strcpy_s(smallText, "Zen Mode");
            break;
        case SpeedBoard:
            sprintf_s(detailsText, "Lightning: x%d multiplier", GetMultiplier(theBoard));
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "lightning_icon");
            strcpy_s(smallText, "Lightning Mode");
            break;
        case ButterflyBoard:
            sprintf_s(detailsText, "Butterflies: %s released", formatNumber(GetReleasedButterflies(theBoard), scoreBuffer));
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "butterflies_icon");
            strcpy_s(smallText, "Butterflies Mode");
            break;
        case PokerBoard:
            sprintf_s(detailsText, "Poker: Hand #%d", GetPokerHands(theBoard));
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "poker_icon");
            strcpy_s(smallText, "Poker Mode");
            break;
        case DigBoard:
            sprintf_s(detailsText, "Diamond Mine: %dm depth", GetDiamondMineDepth(theBoard));
            sprintf_s(stateText, "Money: %s$", formatNumber(GetDiamondMineMoney(theBoard), scoreBuffer));
            strcpy_s(smallImage, "diamondmine_icon");
            strcpy_s(smallText, "Diamond Mine Mode");
            break;
        case InfernoBoard:
            sprintf_s(detailsText, "Ice Storm: x%d multiplier", GetMultiplier(theBoard));
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "icestorm_icon");
            strcpy_s(smallText, "Ice Storm Mode");
            break;
        case TimeBombBoard:
            if (IsRealTimeBomb(theBoard))
            {
                sprintf_s(detailsText, "Time Bomb: %d defused", GetDefusedBombs(theBoard));
            }
            else
            {
                sprintf_s(detailsText, "Match Bomb: %d defused", GetDefusedBombs(theBoard));
            }
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "");
            strcpy_s(smallText, "");
            break;
        case RealTimeBombBoard: //I don't know if this one is ever even used
            sprintf_s(detailsText, "Time Bomb: %d defused", GetDefusedBombs(theBoard));
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "");
            strcpy_s(smallText, "");
            break;
        //3+ Extra Modes
        case FillerBoard:
            sprintf_s(detailsText, "Avalanche: %d gems", GetAmountOfGemsOnBoard(theBoard));
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "");
            strcpy_s(smallText, "");
            break;
        case BalanceBoard:
            sprintf_s(detailsText, "Balance: %d Red/%d Blue gems", GetRedBalance(theBoard), GetBlueBalance(theBoard));
            sprintf_s(stateText, "Offset: %f", GetBalance(theBoard));
            strcpy_s(smallImage, "");
            strcpy_s(smallText, "");
            break;
        case MoveLimitBoard:
            sprintf_s(detailsText, "Stratamax: %d moves left", GetMovesLeft(theBoard));
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "");
            strcpy_s(smallText, "");
            break;
        case SandboxBoard:
            strcpy_s(detailsText, "Sandbox");
            sprintf_s(stateText, "Score: %s", formatNumber(GetScore(theBoard), scoreBuffer));
            strcpy_s(smallImage, "");
            strcpy_s(smallText, "");
            break;
        //if not in game, we are in the menu
        default:
            if (IsInQuestMenu())
            {
                strcpy_s(detailsText, "In Quest Menu");
                strcpy_s(stateText, longQmpNames.find(cfgvalues::questPack)->second); //guaranteed to always have a value
                strcpy_s(smallImage, "quest_icon");
                strcpy_s(smallText, "Quest Mode");
            }
            else
            {
                strcpy_s(detailsText, "In Main Menu");
                sprintf_s(stateText, "Total Playtime: %s", GetTotalPlayTime(totalTimeBuffer));
                strcpy_s(smallImage, "");
                strcpy_s(smallText, "");
            }
            break;
        }
        if (IsInQuest(theBoard))
        {
            wcstombs_s(&i, questName, GetQuestTitle(theBoard)->c_str(), 58);
            sprintf_s(detailsText, "Quest: %s", questName);
            sprintf_s(stateText, "Goal: %d/%d", GetScore(theBoard), GetQuestRequirement(theBoard));
            strcpy_s(smallImage, "quest_icon");
            strcpy_s(smallText, "Quest Mode");
        }

        activity.SetDetails(detailsText);
        activity.SetState(stateText);
        activity.GetAssets().SetSmallImage(smallImage);
        activity.GetAssets().SetSmallText(smallText);
        activity.GetAssets().SetLargeImage("largeimage");
        corePtr->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
            printf_s("%s updating Discord activity!\n", (result == discord::Result::Ok) ? "Succeeded" : "Failed");
            });
        corePtr->RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    } while (!interrupted);
}


void initDiscordRpc(CodeInjection::FuncInterceptor* hook)
{
	if (cfgvalues::wantDiscordRichPresence)
	{
        std::thread(discordThread).detach();
        puts("Discord Rich Presence initialised successfully!");
	}
    else
    {
        puts("Discord Rich Presence initialised, but is disabled in the config!");
    }
}