#include <Engine.h>
#include <Extender/util.h>
#include "sandboxcheats.h"
#include "sandboxfunctions.h"
#include "gamefunctions.h"
#include "mods.h"
#include "Windows.h"
#include <process.h>
#include <thread>

namespace
{
	//The key that was pressed in Sexy::Board::KeyDown
	int theKey;
	//the pointer to the current board object
	uintptr_t theBoard;
	//The current hovered gem
	uintptr_t gemPtr = NULL;
	//Hovered gem's X coordinate
	int theGemX;
	//Hovered gem's Y coordinate
	int theGemY;
	//Hovered gem's color
	int theGemColor;
	//Hovered gem's flags
	int theGemProperties;
	//Enabling hoverOutput will output the X,Y and the flags of the hovered gem to the console
	bool hoverOutput = false;

	const std::wstring musics[] = { L"Classic", L"Zen", L"Speed", L"Speed_lose", L"Poker", L"Butterflies", L"Icestorm", L"BuriedTreasure", L"QuestTimeBased", L"QuestTurnBased", L"QuestBomb", L"LoadingScreen", L"MainMenu", L"QuestMenu", L"QuestMenu_fanfare" };
	constexpr int musicsCount = sizeof(musics) / sizeof(musics[0]);
	//The current music index to be played
	int musicIdx = 0;
	std::wstring music = L"";
	std::wstring play = L"Play";
} // namespace

std::map<std::string, int> sandbox_cheats::keyBindsDict = std::map<std::string, int>();

static void nextSong()
{
	music = musics[musicIdx];
	musicIdx++;
	if (musicIdx >= musicsCount)
		musicIdx = 0;
}

static std::string getPressedFunction(int theKey)
{
    std::map<std::string, int>::iterator found = std::find_if(
		sandbox_cheats::keyBindsDict.begin(), 
		sandbox_cheats::keyBindsDict.end(), 
		[&](std::pair<std::string, int> kvp) {return kvp.second == theKey;}
	);
    if (found == sandbox_cheats::keyBindsDict.end()) return ""; //not found

    return (*found).first;
}

static void handleHotKey()
{
    std::string thePressedFunc = getPressedFunction(theKey);

	if (thePressedFunc == "") return;

	if (gemPtr == NULL) return;
	
	theGemX = GetGemX(gemPtr);
	theGemY = GetGemY(gemPtr);
	theGemColor = GetGemColor(gemPtr);

	bool isSandbox = GetBoardType(theBoard) == SandboxBoard;

	//big else if block, just look for the function we pressed

	//only executes under sandbox
	if (isSandbox)
	{
		if (thePressedFunc == "Red Gem")
		{
			SetGemColor(gemPtr, PieceColor::RED);
			printf_s("Changing the color of %d,%d to Red\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "White Gem")
		{
			SetGemColor(gemPtr, PieceColor::WHITE);
			printf_s("Changing the color of %d,%d to White\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Green Gem")
		{
			SetGemColor(gemPtr, PieceColor::GREEN);
			printf_s("Changing the color of %d,%d to Green\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Yellow Gem")
		{
			SetGemColor(gemPtr, PieceColor::YELLOW);
			printf_s("Changing the color of %d,%d to Yellow\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Purple Gem")
		{
			SetGemColor(gemPtr, PieceColor::PURPLE);
			printf_s("Changing the color of %d,%d to Purple\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Orange Gem")
		{
			SetGemColor(gemPtr, PieceColor::ORANGE);
			printf_s("Changing the color of %d,%d to Orange\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Blue Gem")
		{
			SetGemColor(gemPtr, PieceColor::BLUE);
			printf_s("Changing the color of %d,%d to Blue\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Uncolored Gem")
		{
			if ((GetGemFlags(gemPtr) & PieceFlag::BUTTERFLY) != 0)
			{
				puts("Attempted to uncolor a Butterfly gem");
				return;
			}
			SetGemColor(gemPtr, PieceColor::UNCOLORED);
			printf_s("Changing the color of %d,%d to Uncolored\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Flamify")
		{
			Flamify(theBoard, gemPtr);
			printf_s("Creating a Flame Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Laserify")
		{
			Laserify(theBoard, gemPtr);
			printf_s("Creating a Star Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Hypercubeify")
		{
			Hypercubeify(theBoard, gemPtr, true);
			printf_s("Creating a Hypercube at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Supernovify")
		{
			Supernovify(theBoard, gemPtr);
			printf_s("Creating a Supernova Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Match Bomb")
		{
			SetGemFlag(gemPtr, PieceFlag::MBOMB + PieceFlag::COUNTER);
			SetGemCounter(gemPtr, 15);
			printf_s("Creating a Match Bomb Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Time Bomb")
		{
			SetGemFlag(gemPtr, PieceFlag::TBOMB + PieceFlag::COUNTER);
			SetGemCounter(gemPtr, 45);
			printf_s("Creating a Time Bomb Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Doom Gem")
		{
			SetGemColor(gemPtr, PieceColor::UNCOLORED);
			SetGemFlag(gemPtr, PieceFlag::DOOM + PieceFlag::COUNTER);
			SetGemCounter(gemPtr, 15);
			printf_s("Creating a Doom Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Coinify")
		{
			SetGemFlag(gemPtr, PieceFlag::COIN);
			printf_s("Creating a Coin at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Scrambler")
		{
			SetGemFlag(gemPtr, PieceFlag::SCRAMBLER);
			printf_s("Creating a Scrambler at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Detonator")
		{
			SetGemFlag(gemPtr, PieceFlag::DETONATOR);
			printf_s("Creating a Detonator at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Mystery Gem")
		{
			SetGemFlag(gemPtr, PieceFlag::MYSTERY);
			printf_s("Creating a Mystery Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Resolve Mystery")
		{
			ResolveMysteryGem(theBoard);
			printf_s("Resolving Mystery Gems\n");
		}
		else if (thePressedFunc == "Hypernovify")
		{
			Hypernovify(theBoard, gemPtr);
			printf_s("Creating a Hypernova at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Add Counter")
		{
			AddGemCounter(gemPtr, 5); //the spinning animation substracts 1
			printf_s("Adding 5 seconds/moves to the Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Substract Counter")
		{
			AddGemCounter(gemPtr, -5);
			printf_s("Substracting 5 seconds/moves from the Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Destroy Gem")
		{
			DestroyGem(gemPtr);
			printf_s("Destroying the Gem at %d,%d\n", theGemX, theGemY);
		}
		else if (thePressedFunc == "Increase game speed")
		{
			cfgvalues::speedFactor += (cfgvalues::speedFactor / 10);
			puts("Increased gamespeed");
		}
		else if (thePressedFunc == "Decrease game speed")
		{
			cfgvalues::speedFactor -= (cfgvalues::speedFactor / 10);
			puts("Decreased gamespeed");
		}
		else if (thePressedFunc == "Reset game speed")
		{
			cfgvalues::speedFactor = 1.0;
			puts("Reset gamespeed to normal");
		}
		else if (thePressedFunc == "Next background")
		{
			SetBackground(theBoard, 1);
			puts("Changing to next backdrop");
		}
		else if (thePressedFunc == "Previous background")
		{
			SetBackground(theBoard, -1);
			puts("Changing to previous backdrop");
		}
		else if (thePressedFunc == "Blazing Speed")
		{
			DoSpeedText(theBoard);
			puts("Activating Blazing Speed");
		}
	}

	//only apply piece effect when not sandbox
	if (thePressedFunc == "Multiplierify")
	{
		if (isSandbox)
		{
			SetGemFlag(gemPtr, PieceFlag::MULTIPLIER);
		}
		StartMultiplierGemEffect(theBoard, gemPtr);
		printf_s("Creating a Multiplier Gem at %d,%d\n", theGemX, theGemY);
	}
	else if (thePressedFunc == "Butterflyify")
	{
		if (theGemColor == -1 || theGemColor > 6)
		{
			printf_s("Attempted to Butterflyify a null-colored gem at %d,%d\n", theGemX, theGemY);
			return;
		}
		if (isSandbox)
		{
			Butterflyify(gemPtr, theBoard);
		}
		else
		{
			StartButterflyGemEffect(theBoard, gemPtr);
		}
		printf_s("Creating a Butterfly Gem at %d,%d\n", theGemX, theGemY);
	}
	else if (thePressedFunc == "Time Gem")
	{
		if (isSandbox)
		{
			SetGemFlag(gemPtr, PieceFlag::TIMEBONUS + PieceFlag::DECAYING);
			SetGemCounter(gemPtr, 15);
		}
		StartTimeBonusGemEffect(theBoard, gemPtr);
		printf_s("Creating a Time Gem at %d,%d\n", theGemX, theGemY);
	}
	else if (thePressedFunc == "Blastify")
	{
		if (isSandbox)
		{
			Blastify(theBoard, gemPtr);
		}
		else
		{
			StartBlastGemEffect(theBoard, gemPtr);
		}
		printf_s("Creating a Blast Gem at %d,%d\n", theGemX, theGemY);
	}
	else if (thePressedFunc == "Shake")
	{
		AddGemShake(gemPtr, 0.15f);
		printf_s("Shaking up the gem at %d,%d\n", theGemX, theGemY);
	}
	else if (thePressedFunc == "Normal")
	{
		if (isSandbox)
		{
			ClearGemFlags(gemPtr);
		}
		SetGemShake(gemPtr, 0.0f);
		FreeGemEffect(theBoard, gemPtr);
		printf_s("Clearing all flags from the Gem at %d,%d\n", theGemX, theGemY);
	}

	//same on all modes
	else if (thePressedFunc == "Toggle Hover Output")
	{
		hoverOutput = !hoverOutput;
		printf_s("%s Hover Output\n", hoverOutput ? "Enabled" : "Disabled");
	}
	else if (thePressedFunc == "Next music")
	{
		nextSong();
		PlayMusic(GetCustomBassInterface(), &play, &music, 0);
		puts("Changing to next song");
	}
	else if (thePressedFunc == "Hint button")
	{
		ButtonDepress(theBoard + 0xB8, 0);
		puts("Pressing Hint");
	}
	else if (thePressedFunc == "Reset button")
	{
		ButtonDepress(theBoard + 0xB8, 2);
		puts("Pressing Reset");
	}
	else if (thePressedFunc == "Show replay")
	{
		DoReplaySetup(theBoard);
		puts("Saving replay of current move");
	}
	else if (thePressedFunc == "Speed up Balance scales")
	{
		//only in Balance mode, not allowed in Quest
		if (GetBoardType(theBoard) == BalanceBoard && !IsInQuest(theBoard))
		{
			if (IsBoardStill(theBoard) && *(int*)(theBoard + 12988) > 10) //setting RollSpeedDivisor to 0 deletes the weights
			{
				*(int*)(theBoard + 12988) /= 10; //decrease the RollSpeedDivisor
				PlaySample(*(int*)(0x89DE50), 0, 1.0, 0.0);
				puts("Speeding up Balance scales");
			}
			else
			{
				PlaySample(*(int*)(0x89DAE8), 0, 1.0, 0.0); //badmove
			}
		}
	}
	else if (thePressedFunc == "Open Key Mapping App")
	{
		std::thread([] { if (_spawnlp(_P_WAIT, "KeyMapping.exe", "KeyMapping.exe", 0) == 1) openKeyBindsFile(); }).detach();
	}
}


namespace sandbox_cheats
{
    NAKEDDEF(KeyDownOverride)
    {
        __asm
        {
            //original code
            push eax;
            lea eax, [esp + 78h];

            //store the pressed key
            mov theKey, edi;
            //store the pointer to the Board
            mov theBoard, ecx;
            push eax;
            push ecx;
            call handleHotKey;
            pop ecx;
            pop eax;

            push 0x737556;
            ret;
        }
    }

    NAKEDDEF(MouseOverOverride)
    {
        __asm
        {
            //store the gem that we are hovering right now
            mov gemPtr, esi;
            cmp hoverOutput, 1; //check if hoverOutput is enabled
            jnz goback;  

            push edx; //temporarily save edx and ecx
            push ecx;
            mov eax, [esi + 8];
            cmp eax, theGemX; //check if the hovered gem's X coord changed
            jnz good;
            mov ecx, [esi + 0Ch];
            cmp ecx, theGemY; //if not, check if Y changed
            jz pregoback; //if Y is also unchanged, don't print the hovered gem's coords

        good: //no need to check if Y changed if X already changed
            mov theGemX, eax;
            mov ecx, [esi + 0Ch];
            mov theGemY, ecx;
            mov ecx, [esi + 220h];
            mov theGemColor, ecx;
            mov eax, [esi + 228h];
            mov theGemProperties, eax;
            push esi;
            call outputDetails;
            pop ecx;

        pregoback: //restore edx and ecx
            pop ecx;
            pop edx;

        goback:
            mov     eax, [edx + 2ACh];
            push 0x732C07;
            ret;
        }
    }
} // namespace sandbox_cheats

void initSandboxCheats(CodeInjection::FuncInterceptor* hook)
{
    if (cfgvalues::hasCheats)
    {
        inject_jmp(0x737551, reinterpret_cast<void*>(sandbox_cheats::KeyDownOverride));
        inject_jmp(0x732C01, reinterpret_cast<void*>(sandbox_cheats::MouseOverOverride));
        inject_nops(0x732B35, 6); //skip a jmp to allow us to modify gems during cascades
        puts("Sandbox Cheats initialized successfully!");
    }
    else 
    {
        puts("Sandbox Cheats initialized, but are disabled in config!");
    }
}
