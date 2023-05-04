#pragma once
#ifndef __SANDBOXFUNCS
#define __SANDBOXFUNCS

#include "gamefunctions.h"
#include "Extender/FuncInterceptor.h"

//All possible functions

inline void(__thiscall* Flamify) (uintptr_t theBoard, uintptr_t gemPtr) = reinterpret_cast<void(__thiscall*) (uintptr_t theBoard, uintptr_t gemPtr)>(0x735F00);
inline void(__thiscall* Laserify) (uintptr_t theBoard, uintptr_t gemPtr) = reinterpret_cast<void(__thiscall*) (uintptr_t theBoard, uintptr_t gemPtr)>(0x739750);
inline void(__thiscall* Hypercubeify) (uintptr_t theBoard, uintptr_t gemPtr, bool theStartEffect) = reinterpret_cast<void(__thiscall*) (uintptr_t theBoard, uintptr_t gemPtr, bool theStartEffect)>(0x7397F0);
void Supernovify(uintptr_t theBoard, uintptr_t gemPtr);
void Blastify(uintptr_t theBoard, uintptr_t gemPtr);
void Hypernovify(uintptr_t theBoard, uintptr_t gemPtr);
inline void(__thiscall* Butterflyify) (uintptr_t gemPtr, uintptr_t theBoard) = reinterpret_cast<void(__thiscall*) (uintptr_t gemPtr, uintptr_t theBoard)>(0x739540);
inline void(__stdcall* DoSpeedText) (uintptr_t theBoard) = reinterpret_cast<void(__stdcall*) (uintptr_t theBoard)>(0x73A820);
inline void(__stdcall* StartMultiplierGemEffect) (uintptr_t theBoard, uintptr_t gemPtr) = reinterpret_cast<void(__stdcall*) (uintptr_t theBoard, uintptr_t gemPtr)>(0x738ED0);
inline void(__stdcall* StartButterflyGemEffect) (uintptr_t theBoard, uintptr_t gemPtr) = reinterpret_cast<void(__stdcall*) (uintptr_t theBoard, uintptr_t gemPtr)>(0x738D50);
inline void(__stdcall* StartBlastGemEffect) (uintptr_t theBoard, uintptr_t gemPtr) = reinterpret_cast<void(__stdcall*) (uintptr_t theBoard, uintptr_t gemPtr)>(0x738BB0);
inline void(__stdcall* StartTimeBonusGemEffect) (uintptr_t theBoard, uintptr_t gemPtr) = reinterpret_cast<void(__stdcall*) (uintptr_t theBoard, uintptr_t gemPtr)>(0x731370);
inline void(__thiscall* DeletePiece) (uintptr_t theBoard, uintptr_t gemPtr) = reinterpret_cast<void(__thiscall*) (uintptr_t theBoard, uintptr_t gemPtr)>(0x70E9B0);
inline void(__thiscall* SetBackground) (uintptr_t theBoard, int deltaIdx) = reinterpret_cast<void(__thiscall*) (uintptr_t theBoard, int deltaIdx)>(0x6D0A80);
inline void(__thiscall* ButtonDepress) (uintptr_t theBoard, int theId) = reinterpret_cast<void(__thiscall*) (uintptr_t theBoard, int theId)>(0x73BF20);
inline void(__thiscall* PlayMusic) (uintptr_t theCustomBassInterface, std::wstring* play, std::wstring* theSong, int thePriority) = reinterpret_cast<void(__thiscall*) (uintptr_t theCustomBassInterface, std::wstring * play, std::wstring * theSong, int thePriority)>(0x727F60);
void ResolveMysteryGem(uintptr_t theBoard);
void FreeGemEffect(uintptr_t theBoard, uintptr_t gemPtr);
void DoReplaySetup(uintptr_t theBoard);
int GetGemFlags(uintptr_t gemPtr);
void SetGemFlag(uintptr_t gemPtr, int theFlag);
void ClearGemFlags(uintptr_t gemPtr);
int GetGemColor(uintptr_t gemPtr);
void SetGemColor(uintptr_t gemPtr, PieceColor theColor);
int GetGemX(uintptr_t gemPtr);
int GetGemY(uintptr_t gemPtr);
void AddGemCounter(uintptr_t gemPtr, int amount);
void SetGemCounter(uintptr_t gemPtr, int amount);
void AddGemShake(uintptr_t gemPtr, float amount);
void SetGemShake(uintptr_t gemPtr, float amount);
void DestroyGem(uintptr_t gemPtr);
uintptr_t GetPieceAtRowCol(int row, int col, uintptr_t theBoard);
std::string parseGemColor(uintptr_t gemPtr);
std::string parseGemFlags(uintptr_t gemPtr);
void nextSong();
void outputDetails(uintptr_t gemPtr);

void initSandboxFunctions(CodeInjection::FuncInterceptor* hook);


#endif // !__SANDBOXFUNCS