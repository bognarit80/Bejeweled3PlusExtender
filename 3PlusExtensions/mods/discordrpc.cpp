#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <Engine.h>
#include <Extender/util.h>
#include "../discord/discord.h"
#include "discordrpc.h"
#include "mods.h"
#include <memory>
#include <csignal>
#include <thread>
#include <sstream>
#include "gamefunctions.h"
#include <codecvt>

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

static std::string formatNumber(int theNumber)
{
    std::stringstream output;
    output.imbue(std::locale("en_US.UTF-8"));
    output << theNumber;

    return output.str();
}

static bool updateOngoingGameInfo(std::stringstream& detailsText, std::stringstream &stateText,
    const char** smallImage, const char** smallText, const uintptr_t theBoard)
{
    BoardType boardType = GetBoardType(theBoard);

    //board is already constructed when the savegame prompt appears, don't show details in that case
    if (!GetBoardBackground(theBoard))
        boardType = UnknownBoard;

    switch (boardType)
    {
    case ClassicBoard:
        detailsText << "Classic: Level " << GetLevel(theBoard);
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "classic_icon";
        *smallText = "Classic Mode";
        break;
    case ZenBoard:
        detailsText << "Zen: Level " << GetLevel(theBoard);
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "zen_icon";
        *smallText = "Zen Mode";
        break;
    case SpeedBoard:
        detailsText << "Lightning: x" << GetMultiplier(theBoard) << " multiplier";
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "lightning_icon";
        *smallText = "Lightning Mode";
        break;
    case ButterflyBoard:
        detailsText << "Butterflies: " << formatNumber(GetReleasedButterflies(theBoard)) << " released";
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "butterflies_icon";
        *smallText = "Butterflies Mode";
        break;
    case PokerBoard:
        detailsText << "Poker: Hand #" << formatNumber(GetPokerHands(theBoard));
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "poker_icon";
        *smallText = "Poker Mode";
        break;
    case DigBoard:
        detailsText << "Diamond Mine: " << formatNumber(GetDiamondMineDepth(theBoard)) << "m depth";
        stateText << "Money: $" << formatNumber(GetDiamondMineMoney(theBoard));
        *smallImage = "diamondmine_icon";
        *smallText = "Diamond Mine Mode";
        break;
    case InfernoBoard:
        detailsText << "Ice Storm: x" << GetMultiplier(theBoard) << " multiplier";
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "icestorm_icon";
        *smallText = "Ice Storm Mode";
        break;
    case TimeBombBoard:
        if (IsRealTimeBomb(theBoard))
        {
            detailsText << "Time Bomb: " << formatNumber(GetDefusedBombs(theBoard)) << " defused";
        }
        else
        {
            detailsText << "Match Bomb: " << formatNumber(GetDefusedBombs(theBoard)) << " defused";
        }
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "";
        *smallText = "";
        break;
    case RealTimeBombBoard: //I don't know if this one is ever even used
        detailsText << "Time Bomb: " << formatNumber(GetDefusedBombs(theBoard)) << " defused";
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "";
        *smallText = "";
        break;
        //3+ Extra Modes
    case FillerBoard:
        detailsText << "Avalanche: " << formatNumber(GetAmountOfGemsOnBoard(theBoard)) << " gems onscreen";
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "";
        *smallText = "";
        break;
    case BalanceBoard:
        detailsText << "Balance: " << formatNumber(GetRedBalance(theBoard))
            << " Red/" << formatNumber(GetBlueBalance(theBoard)) << " Blue gems";
        stateText << "Offset: " << GetBalance(theBoard);
        *smallImage = "";
        *smallText = "";
        break;
    case MoveLimitBoard:
        detailsText << "Stratamax: " << GetMovesLeft(theBoard) << " moves left";
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "";
        *smallText = "";
        break;
    case SandboxBoard:
        detailsText << "Sandbox";
        stateText << "Score: " << formatNumber(GetScore(theBoard));
        *smallImage = "";
        *smallText = "";
        break;
        //if not in game, we are in the menu
    default:
        return false;
    }

    return true;
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
    
    const char *smallImage = "";
    const char *smallText = "";

    do {
        std::stringstream detailsText;
        std::stringstream stateText;

        uintptr_t theBoard = GetBoard();

        if (!GetGApp())
        {
            detailsText << "Loading";
            stateText << "";
            smallImage = "";
            smallText = "";
        }
        else if (theBoard)
        {
            if (IsInQuest(theBoard))
            {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
                auto questTitle = GetQuestTitle(theBoard);
                detailsText << "Quest: " << convert.to_bytes(*questTitle);
                stateText << "Goal: " << GetScore(theBoard) << "/" << GetQuestRequirement(theBoard);
                smallImage = "quest_icon";
                smallText = "Quest Mode";
            }
            else
            {
                updateOngoingGameInfo(detailsText, stateText, &smallImage, &smallText, theBoard);
                /*auto title = std::wstring(L"CONFLICT");
                auto text = std::wstring(L"This name matches a profile that already exists on Standard Bejeweled 3. Are you absolutely sure you want to proceed?");
                auto whereText = std::wstring(L"Proceeding will DELETE your Standard profile PERMANENTLY.");
                int response = DoDialog(GetGApp(), 0, true, &title, &text, &whereText, 2);
                int response2 = response;*/
            }
        }
        else
        {
            //no board means the player is in the menu, either normal or quest
            if (IsInQuestMenu())
            {
                detailsText << "In Quest Menu";
                stateText << longQmpNames.find(cfgvalues::questPack)->second; //guaranteed to always have a value
                smallImage = "quest_icon";
                smallText = "Quest Mode";
            }
            else
            {
                detailsText << "In Main Menu";
                stateText << "Total Playtime: " << GetTotalPlayTime();
                smallImage = "";
                smallText = "";
            }
        }

        activity.SetDetails(detailsText.str().c_str());
        activity.SetState(stateText.str().c_str());
        activity.GetAssets().SetSmallImage(smallImage);
        activity.GetAssets().SetSmallText(smallText);
        activity.GetAssets().SetLargeImage("largeimage");
        corePtr->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
            if (result != discord::Result::Ok) printf_s("Failed to update discord activity! (error %d)", static_cast<int>(result));
        });
        corePtr->RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
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