#include <Windows.h>

#include <Addresses.h>
#include <Extender/CodeAllocator.h>
#include <Extender/CodeInjectionStream.h>
#include <Extender/FuncInterceptor.h>
#include <Extender/util.h>
#include "mods/mods.h"
#include "Engine.h"

void fixFloatingPointSupport(CodeInjection::FuncInterceptor* hook);

namespace
{
    CodeInjection::CodeAllocator* codeAlloc;
    CodeInjection::CodeInjectionStream* injectionStream;
    CodeInjection::FuncInterceptor* hook;

    bool initCore()
    {
        printf("> Initializing Core\n");
        codeAlloc = new CodeInjection::CodeAllocator();
        injectionStream = new CodeInjection::CodeInjectionStream(reinterpret_cast<void*>(BJ3_TEXT_START), BJ3_TEXT_SIZE);
        hook = new CodeInjection::FuncInterceptor(injectionStream, codeAlloc);

        fixFloatingPointSupport(hook);

        return true;
    }

    bool init3PlusExtender()
    {
        printf("== Initializing 3Plus Extender ==\n");
        if (!initCore())
        {
            setLastError("Failed to initialize core!");
            return false;
        }

        if (!initMods(hook))
        {
            setLastError("Failed to initialize one or more mods!");
            return false;
        }

        printf("Bejeweled3Plus Extender Loaded Successfully!\n");

        return true;
    }
}

extern "C" __declspec(dllexport) bool load3PlusExtender()
{
    const char* testPointer = reinterpret_cast<const char*>(BJ3_VERIFY_TEXT);
    const char* testStr = "Bejeweled3";

    if (!test_string_pointer(testPointer, testStr))
    {
        setLastError("3PlusExtender is only designed for Bejeweled 3 Plus from Popcap.com");
        return false;
    }

    return init3PlusExtender();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            if (!load3PlusExtender())
            {
                std::string msg = "3Plus Extender Initialization Failed!";
                if (lastError.length() > 0)
                    msg.append("\n\n" + lastError);
                MessageBox(nullptr, msg.c_str(), "3Plus Extender", MB_OK | MB_ICONERROR);
                TerminateProcess(GetCurrentProcess(), 0);
            }
            break;
        case DLL_PROCESS_DETACH:
            break;
        default:
            break;
    }

    return TRUE;
}
