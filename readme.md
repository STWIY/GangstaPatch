## Gangsta Patch

- Unofficial patch for v1.00.2 to make the game playable on modern systems.
- Work in progress currently...

## Building
1. Clone repository
2. Clone [SDK](https://github.com/STWIY/SDK "SDK")
3. Edit project properties and change include directory for SDK
4. Build and you should get `.asi` file that can be placed in game directory with asi loader.

## Ini Settings
```ini
[Scarface]
ShowFPS=0	              ; When set to 1 it will show FPS at left corner.
SkipLicenseScreen=0	    ; When set to 1 it will skip license screen while starting up game.
SkipMovies=0	          ; When set to 1 it will skip intro movies while starting up game.
DebugMenu=0             ; When set to 1 it will show debug menu item in the pause menu.

[Windowed]
Mode=0	                ; 0: None, 1: Windowed, 2: Windowed Borderless

[Patch]
AffinityMode=0          ; 0: None, 1: All besides core 0 (Might improve performance on Hyper-threaded CPU), 2: Game Handled (Default)
```
