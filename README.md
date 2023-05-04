![BEJEWELED 3 PLUS](https://user-images.githubusercontent.com/101661823/236090102-9b2ecd99-727b-44c7-8b07-50133c43684d.png)
# Bejeweled 3 Plus Extender
Plugin system for Bejeweled 3 Plus based on the <a href="https://github.com/PlatinumTeam/MBExtender-Legacy">MBExtender</a> that allows developers to add additional functionality to the game by writing plugins in native C++ code.

Includes several plugins:
- Three new, custom game modes, including a highly-configurable Sandbox mode 
- Support for loading various configs and quest packs (most notably Quest Mode Plus from the Bejeweled Tieba community)
- Support for 16:9 resolution
- 3D acceleration fix
- Instant Replays in Zen Mode
- Many more small patches - more info at the <a href="https://docs.google.com/document/d/1gwM1E4D5LGKj8cAvVVXWs1LiW6mJyCuxACQKGeGriqw">project doc</a>.

All plugins that change gameplay can be toggled in a simple config file that is auto-created in the game folder.

## Build
The project builds to a .dll file that gets placed in the `Game` directory.
Pressing `Run` in Visual Studio starts debugging the `Bejeweled3Plus.exe` or `Bejeweled3Plus_Debug.exe` starting in `Game/` depending on the current configuration.

You need to put the game files manually in the `Game` folder. You also need to import the `load3PlusExtender` function from the .dll into the game executable if you're using an unmodified build of Bejeweled 3 Plus.

( logo by @B1Gaming )
