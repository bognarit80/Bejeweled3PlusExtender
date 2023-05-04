#include "sandboxfunctions.h"
#include <Engine.h>
#include <Extender/util.h>

namespace
{
    //Strings used in printing the color
    const char* colors[] = { "Null", "Red", "White", "Green", "Yellow", "Purple", "Orange", "Blue", "Other" };
} // namespace

void Supernovify(uintptr_t theBoard, uintptr_t gemPtr)
{
    Flamify(theBoard, gemPtr);
    Laserify(theBoard, gemPtr);
}

void Blastify(uintptr_t theBoard, uintptr_t gemPtr)
{
    //we love parameters in eax
    __asm
    {
        mov eax, gemPtr;
        mov ecx, 0x739510;
        push theBoard;
        call ecx; //Board::Blastify
    }
}

void Hypernovify(uintptr_t theBoard, uintptr_t gemPtr)
{
    Flamify(theBoard, gemPtr);
    Laserify(theBoard, gemPtr);
    SetGemFlag(gemPtr, PieceFlag::FLAME + PieceFlag::HYPERCUBE);
}

void ResolveMysteryGem(uintptr_t theBoard)
{
    __asm
    {
        mov edi, theBoard;
        mov ecx, 0x734A90;
        call ecx; //Board::ResolveMysteryGem
    }
}

void FreeGemEffect(uintptr_t theBoard, uintptr_t gemPtr)
{
    __asm
    {
        mov eax, gemPtr;
        mov ecx, [eax + 4];
        mov edx, theBoard;
        mov eax, [edx + 12C8h];
        push ecx;
        mov ecx, 0x7197C0;
        call ecx; //Piece::FreePieceEffect
    }
}

void DoReplaySetup(uintptr_t theBoard)
{
    __asm
    {
        mov eax, theBoard;
        mov ecx, 0x746A50;
        call ecx; //Board::DoReplaySetup
    }
}

int GetGemFlags(uintptr_t gemPtr)
{
    return *(int*)(gemPtr + 552);
}

//set multiple flags at once by adding them all, eg. (1+4) applies both flame and star at once
void SetGemFlag(uintptr_t gemPtr, int theFlag)
{
    *(int*)(gemPtr + 552) |= theFlag;
}

void ClearGemFlags(uintptr_t gemPtr)
{
    *(int*)(gemPtr + 552) = 0;
}

int GetGemColor(uintptr_t gemPtr)
{
    return *(int*)(gemPtr + 544);
}

void SetGemColor(uintptr_t gemPtr, PieceColor theColor)
{
    *(PieceColor*)(gemPtr + 544) = theColor;
}

int GetGemX(uintptr_t gemPtr)
{
    return *(int*)(gemPtr + 8);
}

int GetGemY(uintptr_t gemPtr)
{
    return *(int*)(gemPtr + 12);
}

//Also applies spinning animation to Counter gems
void AddGemCounter(uintptr_t gemPtr, int amount)
{
    if ((*(int*)(gemPtr + 552) & PieceFlag::COUNTER) != 0)
    {
        ++amount; //the spinning animation automatically deducts 1
        *(float*)(gemPtr + 620) = 1.0f; //mSpinFrame
        *(float*)(gemPtr + 624) = 0.33f; //mSpinSpeed
    }
    *(int*)(gemPtr + 580) += amount;
}

//Works on both Time and Counter gems
void SetGemCounter(uintptr_t gemPtr, int amount)
{
    *(int*)(gemPtr + 580) = amount;
}

void AddGemShake(uintptr_t gemPtr, float amount)
{
    *(float*)(gemPtr + 612) += amount; //mShakeScale
}

void SetGemShake(uintptr_t gemPtr, float amount)
{
    *(float*)(gemPtr + 612) = amount; //mShakeScale
}

void DestroyGem(uintptr_t gemPtr)
{
    *(int*)(gemPtr + 560) = 1;
}

//Handy for automation purposes. theBoard is a pointer to the current Board object, can be retrieved with GetBoard() too
uintptr_t GetPieceAtRowCol(int row, int col, uintptr_t theBoard)
{
    uintptr_t thePiece = 0;
    __asm
    {
        mov eax, row;
        mov ecx, col;
        push theBoard;
        mov edx, 0x72B1A0;
        call edx; //call Board::GetPieceAtRowCol, lovely call convention :)
        mov thePiece, eax; //returns a pointer to the piece and stores it in eax, move that value to thePiece
    }
    return thePiece;
}

std::string parseGemColor(uintptr_t gemPtr)
{
    int theGemColor = GetGemColor(gemPtr);
    //-1 to 6, print the according color from the colors array, otherwise "Other" at idx 8
    return (std::string)"Color: " + colors[(theGemColor < 7 && theGemColor >= -1) ? theGemColor + 1 : 8];
}

std::string parseGemFlags(uintptr_t gemPtr)
{
    //the final string to be printed by the outputCoords func
    std::string theOutput = "";
    int flags = GetGemFlags(gemPtr);

    if (flags)
    {
        theOutput += ", Flags: ";
        if ((flags & PieceFlag::FLAME) != 0)
            theOutput += "Flame, ";
        if ((flags & PieceFlag::HYPERCUBE) != 0)
            theOutput += "Hypercube, ";
        if ((flags & PieceFlag::STAR) != 0)
            theOutput += "Star, ";
        if ((flags & PieceFlag::RESERVED) != 0)
            theOutput += "Reserved, ";
        if ((flags & PieceFlag::MULTIPLIER) != 0)
            theOutput += "Multiplier, ";
        if ((flags & PieceFlag::MBOMB) != 0)
            theOutput += "Match Bomb, ";
        if ((flags & PieceFlag::TBOMB) != 0)
            theOutput += "Time Bomb, ";
        if ((flags & PieceFlag::BUTTERFLY) != 0)
            theOutput += "Butterfly, ";
        if ((flags & PieceFlag::DOOM) != 0)
            theOutput += "Doom, ";
        if ((flags & PieceFlag::COUNTER) != 0)
            theOutput += "with Counter, ";
        if ((flags & PieceFlag::COIN) != 0)
            theOutput += "Coin, ";
        if ((flags & PieceFlag::DETONATOR) != 0)
            theOutput += "Detonator, ";
        if ((flags & PieceFlag::SCRAMBLER) != 0)
            theOutput += "Scrambler, ";
        if ((flags & PieceFlag::MYSTERY) != 0)
            theOutput += "Mystery, ";
        if ((flags & PieceFlag::SKULL) != 0)
            theOutput += "Skull, ";
        if ((flags & PieceFlag::INFERNOSWAP) != 0)
            theOutput += "Blazing Swap, ";
        if ((flags & PieceFlag::GROUND) != 0)
            theOutput += "Ground, ";
        if ((flags & PieceFlag::TIMEBONUS) != 0)
            theOutput += "Time, ";
        if ((flags & PieceFlag::DECAYING) != 0)
            theOutput += "Decaying, ";
        if ((flags & PieceFlag::BLAST) != 0)
            theOutput += "Blast, ";

        //delete the space and comma at the end
        theOutput.erase(theOutput.length() - 2, 2);
    }
    return theOutput;
}

void outputDetails(uintptr_t gemPtr)
{
    printf_s("Hovered Gem: %d,%d, %s%s\n", GetGemX(gemPtr), GetGemY(gemPtr), 
        parseGemColor(gemPtr).c_str(), parseGemFlags(gemPtr).c_str());
}

void initSandboxFunctions(CodeInjection::FuncInterceptor* hook)
{
    puts("Sandbox Functions Initialized Successfully!");
}