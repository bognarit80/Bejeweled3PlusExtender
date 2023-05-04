#include "changepaths.h"

#include <Engine.h>
#include <Extender/util.h>
#include "mods.h"
#include "gamefunctions.h"
#include <windows.h>

namespace
{
    //use std::string for concatenation
    std::string finalArg = "";

    //the final args in C string form
    const char* finalerArg;

    const char* realArgs;

    //change the title name :)
    const wchar_t* bej3plus = L"Bejeweled 3 Plus";

    void appendCmdLine()
    {
        std::string addedArgs = finalerArg; //std::string because changing finalArg invalidates the pointer
        finalArg = realArgs;
        finalArg += addedArgs;
        finalerArg = finalArg.c_str();
    }
} // namespace

namespace change_paths 
{
    NAKEDDEF(DoParseCmdLineOverride)
    {
        __asm 
        {
            mov realArgs, eax;
            call appendCmdLine;
            mov eax, dword ptr[finalerArg];

            cmp     byte ptr[eax], 22h;
            jnz noquotes;

            inc eax;
            push 0x5C7CC2;
            ret;

        noquotes:
            push 0x5C7CD2;
            ret;
        }
    }

    NAKEDDEF(WindowTitleOverride)
    {
        __asm
        {
            push 11h;
            push [bej3plus];
            push 0x75F013;
            ret;
        }
    }
} // namespace change_paths

void initchangePaths(CodeInjection::FuncInterceptor* hook)
{
    //paths default to 3+ at any time
    //secrep.cfg
    inject_byte(0x8266AC, 'p');

    //badgep.cfg
    inject_byte(0x8267B4, 'p');

    //Failed to open badgep.cfg
    inject_byte(0x8267DB, 'p');

    //resources.xml
    inject_byte(0x836F63, 's');

    //PopCap/BejeweledP in the registry
    inject_byte(0x825CC8, 'P');

    std::string cmdline = GetCommandLine();

    switch (cfgvalues::artRes)
    {
    case ArtRes::NORMAL:
        finalArg += " -600";
        inject_byte(0x7C0FE8, 'p'); //bomb_up
        break;
    case ArtRes::HIGH:
        finalArg += " -768";
        inject_byte(0x7C0FE8, 'p'); //bomb_up
        break;
    case ArtRes::ULTRA: 
        finalArg += " -1200";
        inject_byte(0x7C0FE8, 'h'); //bomb_uh
        break;
    default:
        inject_byte(0x7C0FE8, 'p'); //bomb_up
        break;
    }

    if (cfgvalues::wantDirectX8)
    {        
        if (cmdline.find("-nod3d9") == std::string::npos)
        {
            finalArg += " -nod3d9";
        }
    }

    if (cfgvalues::isWide)
    {
        if (cmdline.find("-wide") == std::string::npos)
        {
            finalArg += " -wide";
        }
    }
    
    if (finalArg.length() != 0)
    {
        //convert the final string into a C string to be read by the ingame parser
        finalerArg = finalArg.c_str();
        printf_s("Applying args:%s\n", finalerArg);
    }

    //whether the quest pack name was input correctly
    bool correct = true;

    //redirect the quest.cfg path to the according quest pack cfg, cfgvalue case insensitive
    if (cfgvalues::questPack == "QMP2")
    {
        //qmp2n.cfg
        inject_bytes(0x826674, { 'm', 'p', '2', 'n'} ); //m p 2 n
    } 

    else if (cfgvalues::questPack == "QMP3")
    {
        //qmp3h.cfg
        inject_bytes(0x826674, { 'm', 'p', '3', 'h'} ); //m p 3 h  
    }

    else if (cfgvalues::questPack == "QMP4E")
    {
        //qmp4e.cfg
        inject_bytes(0x826674, { 'm', 'p', '4', 'e'} ); //m p 4 e
    }

    else if (cfgvalues::questPack == "QMP4N")
    {
        //qmp4n.cfg
        inject_bytes(0x826674, { 'm', 'p', '4', 'n'} ); //m p 4 n
    }

    else if (cfgvalues::questPack == "QMP4H")
    {
        //qmp4h.cfg
        inject_bytes(0x826674, { 'm', 'p', '4', 'h'} ); //m p 4 h
    }

    else if (cfgvalues::questPack == "QMP4L")
    {
        //qmp4l.cfg
        inject_bytes(0x826674, { 'm', 'p', '4', 'l'} ); //m p 4 l
    }

    else if (cfgvalues::questPack != "DEFAULT")
    {
        cfgvalues::questPack = "DEFAULT"; //set back to default for incorrect input, for easy poker fix check
        correct = false;
    }

    if (cfgvalues::isFixless) 
    {
        //secref.cfg
        inject_byte(0x8266AC, 'f');
        if (cfgvalues::hasExtraModes)
            puts("WARNING: If both Fixless and Extra Modes are enabled, Extra Modes take prority!");
    }

    //redirect secret.cfg to include the extra modes, default.xml for their strings, and the rest for the savefiles
    if (cfgvalues::hasExtraModes)
    {
        //secrea.cfg
        inject_byte(0x8266AC, 'a');

        //defaula.xml
        inject_byte(0x836F49, 'a');

        //avala.sav
        inject_bytes(0x7C9F34, { 'a', 'v', 'a', 'l', 'a' } ); //avala

        //balance_e.sav
        inject_bytes(0x7C9F41, { 'a', 'l', 'a', 'n', 'c', 'e', '_', 'e' } ); //alance_e

        //stratamax.sav
        inject_bytes(0x7C9F50, { 's', 't', 'r', 'a', 't', 'a', 'm', 'a', 'x'} ); //stratamax

        //sandbox_mode.sav
        inject_bytes(0x7C9F60, { 's', 'a', 'n', 'd', 'b', 'o', 'x', '_', 'm', 'o', 'd', 'e' } ); //sandbox_mode

        puts("Extra Modes loaded successfully!");
    }

    if (cfgvalues::loadingOptimisation == 0)
    {
        //Gameid021WC.xml, the slower profile
        inject_byte(0x83C28D, 'C');
    }
    else
    {
        //Gameid021WD.xml, the faster profile
        inject_byte(0x83C28D, 'D');
    }

    //print out the quest pack we're using (including default), notify if unknown input
    correct ? printf("Loading Quest Pack: %s\n", cfgvalues::questPack.c_str()) : 
        puts("Unknown Quest Pack, loading default pack...");

    inject_jmp(0x5C7CBC, reinterpret_cast<void*>(change_paths::DoParseCmdLineOverride));

    inject_jmp(0x75F00C, reinterpret_cast<void*>(change_paths::WindowTitleOverride));

    puts("Configs applied successfully!");
}