#include "gamefunctions.h"
#include <Engine.h>
#include <Extender/util.h>

//General convenience functions for the game that don't apply to gameboard only.
//Some of them call functions from original code.

namespace
{
    std::wstring* (__thiscall* getQuestTitle)(int* paramsPtr, std::string* key) = reinterpret_cast<std::wstring * (__thiscall*) (int*, std::string*)>(0x403540);
    std::string titleArg = "Title";
}

uintptr_t GetGApp()
{
    return *(uintptr_t*)(0x8A67AC);
}

//Returns a pointer to the current Board object
uintptr_t GetBoard()
{
    uintptr_t gApp = GetGApp();
    if (gApp == NULL)
        return NULL;

    return *(uintptr_t*)(gApp + 0xBE0); //gApp's pointer to board obj
}

uintptr_t GetBoardBackground(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return NULL;

    return *(uintptr_t*)(theBoard + 0x2870);
}

BoardType GetBoardType(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return BoardType::UnknownBoard;

    return *(BoardType*)(theBoard);
}

int GetScore(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0xD28);
}

int GetLevel(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0xE0C) + 1; //levels internally start from 0
}

int GetMultiplier(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0xE14);
}

//Returns the pointer to the QuestGoal instance. For the actual goal value, see GetQuestRequirement()
uintptr_t GetQuestGoal(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return QuestGoalType::UnknownGoal;

    return *(uintptr_t*)(theBoard + 0x31F0);
}

QuestGoalType GetQuestGoalType(uintptr_t theQuestGoal)
{
    if (theQuestGoal == NULL)
        return QuestGoalType::UnknownGoal;

    return *(QuestGoalType*)(theQuestGoal);
}

int GetQuestRequirement(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    if (GetBoardType(theBoard) == BalanceBoard)
        return *(int*)(theBoard + 0x32B0) * 2;
    else if (GetBoardType(theBoard) == PokerBoard)
        return *(int*)(theBoard + 0x39D0);

    uintptr_t theQuestGoal = GetQuestGoal(theBoard);
    if (theQuestGoal == NULL)
        return 0;

    switch (GetQuestGoalType(theQuestGoal))
    {
    case HiddenObjectGoal:
        return *(int*)(theQuestGoal + 0x1F4C);
    case DigGoal:
        return *(int*)(theQuestGoal + 0x3BC);
    }

    //default offset for all other quest types
    return *(uintptr_t*)(theQuestGoal + 0x110);
}

bool IsPerpetual(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return false;

    return *(bool*)(theBoard + 0x31F4);
}

bool IsInQuest(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return false;

    BoardType type = GetBoardType(theBoard);
    //classic, zen and lightning might not have the quest ID initialised
    if (type == ClassicBoard || type == ZenBoard || type == SpeedBoard || type == UnknownBoard)
        return false;

    return GetQuestId(theBoard) < 1000; //game modes are just quest with 1000 added to their ID
}

bool IsInQuestMenu()
{
    uintptr_t gApp = GetGApp();
    if (!gApp)
        return false;

    return *(uintptr_t*)(gApp + 0xBD0) != NULL; //check if QuestMenu is initialised
}

std::wstring* GetQuestTitle(uintptr_t theBoard)
{
    if (!IsInQuest(theBoard))
        return new std::wstring();

    return getQuestTitle((int*)(theBoard + 0xC4), &titleArg);
}

int GetQuestId(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0x32A4);
}

bool IsRealTimeBomb(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return false;

    return *(bool*)(theBoard + 0x32CA);
}

uintptr_t GetProfile()
{
    uintptr_t profilePtr = GetGApp() + 0xB78;
    return profilePtr != 0xB78 ? *(uintptr_t*)(profilePtr) : NULL;
}

std::wstring GetProfileName()
{
    uintptr_t profilePtr = GetProfile();
    return profilePtr != NULL ? *(std::wstring*)(profilePtr + 0x60) : std::wstring();
}

char* GetTotalPlayTime(char* buffer)
{
    uintptr_t profile = GetProfile();
    if (profile == NULL)
        return new char('\0');

    int totalSeconds = *(int*)(profile + 192);
    if (totalSeconds > 3600)
    {
        float totalHours = totalSeconds / 3600.0f;
        sprintf_s(buffer, 32, "%.1f hours", totalHours);
    }
    else
    {
        sprintf_s(buffer, 32, "%d minutes", totalSeconds / 60);
    }

    return buffer;
}

int GetAmountOfGemsOnBoard(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0x250);
}

int GetPokerHands(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0x39D8);
}

int GetReleasedButterflies(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0xCF0); //stat 1C
}

int GetDefusedBombs(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0xD18); //stat 26
}

int GetRedBalance(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0x32C8);
}

int GetBlueBalance(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0x32C4);
}

float GetBalance(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(float*)(theBoard + 0x32B8);
}

int GetMovesLeft(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0x32B4);
}

int GetDiamondMineMoney(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    return *(int*)(theBoard + 0xC84);
}

int GetDiamondMineDepth(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return 0;

    uintptr_t questGoal = GetQuestGoal(theBoard);
    if (questGoal == NULL)
        return 0;

    return (int)*(double*)(questGoal + 0x4F0) * 10; //depth is shown in game as 10x of the actual stored value
}

//Returns the graphics setting. 0 if called during the init process because gApp hasn't loaded yet.
int GetArtRes()
{
    int gApp = GetGApp();
    return gApp != NULL ? *(int*)(gApp + 0x9E0) : 0;
}

uintptr_t GetCustomBassInterface()
{
    int gApp = GetGApp();
    return gApp != NULL ? *(uintptr_t*)(gApp + 0xA28) : NULL;
}

//SoundID is retrieved in the resource loader function
void PlaySample(int theSoundId, int thePan, double theVolume, double theNumSteps)
{
    uintptr_t gApp = GetGApp();
    if (!gApp)
        return;

    //lovely call convention
    __asm
    {
        mov edx, theSoundId;
        mov eax, gApp;
        fld theNumSteps;
        fstp qword ptr[esp + 8];
        fld theVolume;
        fstp qword ptr[esp + 0];
        push thePan;
        mov ecx, 0x75B8D0;
        call ecx;
    }
}

bool IsBoardStill(uintptr_t theBoard)
{
    if (theBoard == NULL)
        return false;

    bool theValue = false;

    __asm
    {
        mov edi, theBoard;
        mov edx, 0x752AC0;
        call edx;
        mov theValue, al;
    }

    return theValue;
}