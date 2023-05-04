#pragma once

#include <string>
#include "Extender/FuncInterceptor.h"

enum BoardType
{
	UnknownBoard = 0,
	Board = 0x829B4C,
	ClassicBoard = 0x829FB4,
	SpeedBoard = 0x82B42C,
	ZenBoard = 0x82B9CC,
	QuestBoard = 0x82BE1C,
	DigBoard = 0x82A644,
	BadMoveLimitBoard = 0x82C284,
	SandboxBoard = 0x82C284,
	BalanceBoard = 0x82C6EC,
	ButterflyBoard = 0x82CB54,
	FillerBoard = 0x82CFBC,
	InfernoBoard = 0x82D424,
	MoveLimitBoard = 0x82D8BC,
	NoLoseBoard = 0x82DD24,
	PokerBoard = 0x82E18C,
	RealTimeBombBoard = 0x82E774,
	TimeBombBoard = 0x82EBDC,
	TimeLimitBoard = 0x82F044
};

enum QuestGoalType
{
	UnknownGoal = 0,
	QuestGoal = 0x82F4C4,
	ArsenalGoal = 0x82F5EC,
	ButterflyGoal = 0x82F714,
	ClearBombsGoal = 0x82F83C,
	ClearGoal = 0x82F964,
	DetonatorGoal = 0x82FA8C,
	FirestormGoal = 0x82FBB4,
	GoldGoal = 0x82FCDC,
	GoldLinesGoal = 0x82FE04,
	GoldYellowGoal = 0x82FF2C,
	HiddenGoal = 0x830054,
	HiddenGridGoal = 0x83017C,
	PreserverGoal = 0x8302A4,
	ScoreGoal = 0x8303CC,
	SurvivalGoal = 0x8304F4,
	WallBlastGoal = 0x83061C,
	GoldLinesFlipGoal = 0x830744,
	HiddenObjectGoal = 0x83086C,
	DigGoal = 0x8309B4
};

enum ArtRes
{
	NORMAL = 600,
	HIGH = 768,
	ULTRA = 1200
};

enum PieceColor
{
	UNCOLORED = -1,
	RED,
	WHITE,
	GREEN,
	YELLOW,
	PURPLE,
	ORANGE,
	BLUE,
};

enum PieceFlag
{
	FLAME = 1,
	HYPERCUBE = 2,
	STAR = 4,
	RESERVED = 8,
	MULTIPLIER = 16,
	MBOMB = 32,
	TBOMB = 64,
	BUTTERFLY = 128,
	DOOM = 256,
	COUNTER = 512,
	COIN = 1024,
	DETONATOR = 2048,
	SCRAMBLER = 4096,
	MYSTERY = 8192,
	SKULL = 16384,
	INFERNOSWAP = 32768,
	GROUND = 65536,
	TIMEBONUS = 131072,
	DECAYING = 262144,
	BLAST = 524288
};

uintptr_t GetGApp();
uintptr_t GetBoard();
uintptr_t GetBoardBackground(uintptr_t theBoard);
BoardType GetBoardType(uintptr_t theBoard);
int GetScore(uintptr_t theBoard);
int GetLevel(uintptr_t theBoard);
int GetMultiplier(uintptr_t theBoard);
uintptr_t GetQuestGoal(uintptr_t theBoard);
QuestGoalType GetQuestGoalType(uintptr_t theQuestGoal);
int GetQuestRequirement(uintptr_t theQuestGoal);
bool IsPerpetual(uintptr_t theBoard);
bool IsInQuest(uintptr_t theBoard);
bool IsInQuestMenu();
std::wstring* GetQuestTitle(uintptr_t theBoard);
int GetQuestId(uintptr_t theBoard);
bool IsRealTimeBomb(uintptr_t theBoard);
uintptr_t GetProfile();
std::wstring GetProfileName();
char* GetTotalPlayTime(char* buffer);
int GetAmountOfGemsOnBoard(uintptr_t theBoard);
int GetPokerHands(uintptr_t theBoard);
int GetReleasedButterflies(uintptr_t theBoard);
int GetDefusedBombs(uintptr_t theBoard);
int GetRedBalance(uintptr_t theBoard);
int GetBlueBalance(uintptr_t theBoard);
float GetBalance(uintptr_t theBoard);
int GetMovesLeft(uintptr_t theBoard);
int GetDiamondMineMoney(uintptr_t theBoard);
int GetDiamondMineDepth(uintptr_t theBoard);
int GetArtRes();
uintptr_t GetCustomBassInterface();
void PlaySample(int theSoundId, int thePan, double theVolume, double theNumSteps);
bool IsBoardStill(uintptr_t theBoard);