## Gangsta Patch
[![Downloads](https://img.shields.io/github/downloads/STWIY/GangstaPatch/total?label=Downloads&color=ed1459)](#)
[![Download latest release](https://img.shields.io/github/v/release/STWIY/GangstaPatch?display_name=release&label=Download%20latest%20release&color=21abc7)](https://github.com/STWIY/GangstaPatch/releases/latest/download/GangstaPatch.asi)

- Unofficial patch for v1.00.2 to make the game playable on modern systems.
- Work in progress currently...

## Installation
1. Download latest release.
2. Download ASI Loader or [Simple ASI Loader](https://github.com/sneakyevil/SimpleASILoader/releases/download/vorbisfile/vorbisfile.dll).
3. Place asi file of the patch in game folder / plugins folder.

## Ini Settings
```ini
[Scarface]
Vibrance=50             ; Adjustable vibrance (1 - 100) (50 -> Default)
ShowFPS=0	        ; Shows FPS at left corner.
SkipLicenseScreen=0	; Skips license screen while starting up game.
SkipMovies=0	        ; Skips intro movies while starting up game.

[Windowed]
Mode=0	                ; 0: None, 1: Windowed, 2: Windowed Borderless

[PostProcessFX]
Enable=0                ; Enables blur & bloom.
Bloom=50                ; The game uses dynamic value, using this option will force the value to be always same

[Patch]
AffinityMode=0          ; 0: None, 1: All besides core 0 (Might improve performance on Hyper-threaded CPU), 2: Game Handled (Default)
DebugMenu=0             ; Shows debug option in the pause menu.
```

## Requirements for compiling
- Clang-CL (LLVM) for Windows (Only when using CRT-STL)
- Visual Studio 2019+ (Optional)

## Steps for compiling (Visual Studio)
1. Clone repository.
2. Clone [SDK](https://github.com/STWIY/SDK "SDK").
3. Edit project properties and change include directory for SDK in `VC++ Directories`.
4. Build as Release/Debug (x86) and you should get `.asi` file.
   
