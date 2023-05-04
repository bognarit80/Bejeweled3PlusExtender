#include <Extender/util.h>

#include <Windows.h>
#include <Extender/CodeInjectionStream.h>

#include <ctime>
#include <fstream>

bool test_string_pointer(const char *testPointer, const char *testStr)
{
	// Make sure we can actually read from the test memory location
	MEMORY_BASIC_INFORMATION memInfo;
	if (VirtualQuery(testPointer, &memInfo, sizeof(memInfo)) != sizeof(memInfo))
		return false;
	if (memInfo.Protect == 0 || (memInfo.Protect & PAGE_NOACCESS) || (memInfo.Protect & PAGE_EXECUTE))
		return false;

	// Check if the string matches
	if (memcmp(testPointer, testStr, strlen(testStr)) != 0)
		return false;
	return true;
}

/// <summary>
/// Injects a jump to a target at the desired address.
/// </summary>
/// <param name="address">The desired address.</param>
/// <param name="func">The target. Use NAKEDDEF to properly jump to own code.</param>
void inject_jmp(int address, void(*func))
{
	void* initPtr = reinterpret_cast<void*>(address);
	CodeInjection::CodeInjectionStream stream(initPtr, 6);
	stream.writeRel32Jump(func);
	stream.flush();
}

/// <summary>
/// Injects NOPs to the desired address.
/// </summary>
/// <param name="address">The desired address.</param>
/// <param name="count">The amount of NOP instructions to inject. Each NOP is 1 byte large.</param>
void inject_nops(int address, int count)
{
    void* initPtr = reinterpret_cast<void*>(address);
    CodeInjection::CodeInjectionStream stream(initPtr, count + 1); // <-- this part
    stream.writeNops(count);
    stream.flush();
}

/// <summary>
/// Injects a byte to the desired address.
/// </summary>
/// <param name="address">The desired address.</param>
/// <param name="byte">The byte to inject.</param>
void inject_byte(int address, uint8_t byte)
{
    void* initPtr = reinterpret_cast<void*>(address);
    CodeInjection::CodeInjectionStream stream(initPtr, 1); // <-- this part
    uint8_t byteArray[1] = { byte };
    stream.write(byteArray, 1);
    stream.flush();
}

/// <summary>
/// Injects a list of bytes to the desired address.
/// </summary>
/// <param name="address">The desired address.</param>
/// <param name="bytes">The list of bytes to inject, initialised with the {} syntax.</param>
void inject_bytes(int address, std::initializer_list<unsigned char> bytes)
{
    void* initPtr = reinterpret_cast<void*>(address);
    CodeInjection::CodeInjectionStream stream(initPtr, bytes.size() + 1);
    stream.write(bytes.begin(), bytes.size());
    stream.flush();
}

int random(int min, int max) //range : [min, max)
{
    static bool first = true;
    if (first)
    {
        srand(static_cast<int>(time(nullptr))); //seeding for the first time only!
        first = false;
    }
    return min + rand() % ((max + 1) - min);
}

std::string lastError = "";

void setLastError(std::string error)
{
    if (lastError.length() > 0)
        lastError += "\n";
    lastError += "> " + error;
    printf_s("%s", (error + "\n").c_str());
}


bool fileExists(std::string path)
{
    std::ifstream f(path.c_str());
    return f.good();
}
