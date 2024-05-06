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
Vibrance=50             ; Adjustable vibrance (1 - 100) (50 -> Default)
PostProcessFX=0         ; Enables blur & bloom.
ShowFPS=0	            ; Shows FPS at left corner.
SkipLicenseScreen=0	    ; Skips license screen while starting up game.
SkipMovies=0	        ; Skips intro movies while starting up game.
DebugMenu=0             ; Shows debug option in the pause menu.

[Windowed]
Mode=0	                ; 0: None, 1: Windowed, 2: Windowed Borderless

[Patch]
AffinityMode=0          ; 0: None, 1: All besides core 0 (Might improve performance on Hyper-threaded CPU), 2: Game Handled (Default)
```
