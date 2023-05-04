#include "extramodes.h"
#include "mods.h"

#include <Engine.h>
#include <Extender/util.h>

//Logic for the extra modes' progression. Sandbox mode has its own file.

//default values

int extra_modes_cfg::avalancheStartingDrop = 5;
std::vector<int> extra_modes_cfg::avalancheLevelRequirements = { 12000, 84000, 266000, 866000, 2360000 };

int extra_modes_cfg::balanceStartingValue = 1200;
int extra_modes_cfg::balanceDecPerLevel = 50;
int extra_modes_cfg::balanceDecPerMaxLevel = 30;
std::vector<int> extra_modes_cfg::balanceLevelRequirements = { 50, 75, 100, 150, 200, 300, 400, 500, 650, 800, 1000, 1250, 1500, 1750, 2000, 2500 };

static int checkAvalancheLevel(int score)
{
    int numOfLevels = extra_modes_cfg::avalancheLevelRequirements.size();
    for (int i = 0; i < numOfLevels; i++)
    {
        if ( !(score / extra_modes_cfg::avalancheLevelRequirements[i]) )
        {
            return i + extra_modes_cfg::avalancheStartingDrop;
        }
    }
    return numOfLevels + extra_modes_cfg::avalancheStartingDrop;

    //uncomment to add infinite progression
    //any further level is calculated: (score that exceeds the last level / (last level - second last level)) + startingDrop
    //int theDiff = extra_modes_cfg::avalancheLevelRequirements.at(numOfLevels - 1) - extra_modes_cfg::avalancheLevelRequirements.at(numOfLevels - 2);
    //return ((score - extra_modes_cfg::avalancheLevelRequirements.back()) / (theDiff)) + (numOfLevels + extra_modes_cfg::avalancheStartingDrop);
}

static int checkBalanceLevel(int gems)
{
    int numOfLevels = extra_modes_cfg::balanceLevelRequirements.size();
    for (int i = 0; i < numOfLevels; i++)
    {
        if (!(gems / extra_modes_cfg::balanceLevelRequirements[i]))
        {
            return extra_modes_cfg::balanceStartingValue - (i * extra_modes_cfg::balanceDecPerLevel);
        }
    }

    //any further level is calculated: startingValue - (DecPerLevel * numOfLevels) - DecPerMaxLevel * (score that exceeds the last level / (last level - second last level))
    int theDiff = extra_modes_cfg::balanceLevelRequirements.at(numOfLevels - 1) - extra_modes_cfg::balanceLevelRequirements.at(numOfLevels - 2);
    int beforeLastLevel = extra_modes_cfg::balanceDecPerLevel * numOfLevels;
    int afterLastLevel = (((gems - extra_modes_cfg::balanceLevelRequirements[numOfLevels - 1]) / theDiff) * extra_modes_cfg::balanceDecPerMaxLevel);
    return extra_modes_cfg::balanceStartingValue - beforeLastLevel - afterLastLevel;
}

namespace extra_modes
{
    NAKEDDEF(QuestModeInitOverride)
    {
        __asm
        {
            cmp[esi + 31F4h], 0; //check if isPerpetual
            jz notPerp;
            mov     byte ptr[esi + 26A4h], 0; //change showLevelPoints
            jmp goback;

        notPerp:
            mov     byte ptr[esi + 26A4h], 1;

        goback:
            push 0x6D0BF3;
            ret;
        }
    }

    NAKEDDEF(QuestModeConstructorOverride)
    {
        __asm
        {
            cmp al, 0; //check if isPerpetual
            jz goback;
            mov byte ptr[ecx + 0C18h], 2; //change uiConfig

        goback:
            mov[ecx + 31F4h], al; //original code
            push 0x7659A1;
            ret;
        }
    }

    NAKEDDEF(GetDropPerMoveOverride)
    {
        __asm
        {
            cmp[ecx + 31F4h], 0; //check if isPerpetual
            jz notPerp;

            mov eax, [ecx + 0D28h];
            push ecx; //preserve ecx
            push eax;
            call checkAvalancheLevel; //pass score as argument
            pop ecx;
            pop ecx;
            mov [ecx + 32B0h], eax; //set DropPerMove to our return value which is in eax
            sub eax, extra_modes_cfg::avalancheStartingDrop
            inc eax;
            mov[ecx + 0E14h], eax; //set multiplier to i + 1

        notPerp:
            mov eax, [ecx + 32B0h]; //original code
            push 0x6C7F1C;
            ret;
        }
    }

    NAKEDDEF(BalanceScoreOverride)
    {
        __asm
        {
            cmp[esi + 31F4h], 0; //check if isPerpetual
            jz notPerp;

            push edx; //preserve eax and edx
            push eax;
            call checkBalanceLevel;
            mov [esi + 32BCh], eax; //set RollSpeedDivisor to our return value which is in eax
            pop eax;
            pop edx;

        notPerp:
            mov ecx, [esi + 31F0h]; //original code
            push 0x6D3982;
            ret;
        }
    }
} // namespace extra_modes

void initExtraModes(CodeInjection::FuncInterceptor* hook)
{
    inject_jmp(0x6D0BEC, reinterpret_cast<void*>(extra_modes::QuestModeInitOverride));

    inject_jmp(0x76599B, reinterpret_cast<void*>(extra_modes::QuestModeConstructorOverride)); //set UIConfig to WithResetAndReplay
    inject_nops(0x7316F4, 6); //for whatever reason they forgot to show the widget for that uiconfig

    inject_jmp(0x6C7F16, reinterpret_cast<void*>(extra_modes::GetDropPerMoveOverride));

    inject_jmp(0x6D397C, reinterpret_cast<void*>(extra_modes::BalanceScoreOverride));
    //32C4 blue  //32C8 red  //32BC rollspeeddivisor  //32C0 CascadeThreshold
    inject_bytes(0x82D3C0, { 0xC0, 0x39, 0x6D, 0x00 }); //Avalanche show TimebombEndLevelDialog
    inject_bytes(0x82CAF0, { 0xC0, 0x39, 0x6D, 0x00 }); //Balance show TimebombEndLevelDialog
    inject_bytes(0x82C688, { 0xC0, 0x39, 0x6D, 0x00 }); //Sandbox show TimebombEndLevelDialog
    inject_bytes(0x82DCC0, { 0xC0, 0x39, 0x6D, 0x00 }); //Stratamax (MoveLimit) show TimebombEndLevelDialog
    puts("Extra Mode configs and functionality initialized successfully!");
}