//==========================================================================
// Includes

#include <cstdint>
#include <Windows.h>
#include <d3d9.h>
#include <sysinfoapi.h>

#include <SDK/.SDK.hh>

//==========================================================================

#include "Shader/VehicleGlass.hh"

//==========================================================================
// Libs

#include "3rdParty/MinHook.h"
#pragma comment(lib, "MinHook.lib")

//==========================================================================
// Core

static decltype(&VirtualProtect) g_VirtualProtect;
#include "Core/Patcher.hh"
#include "Core/Settings.hh"

//==========================================================================
// Naked Functions

#include "NakedFunctions/GetBarrelCreepSideMax.hh"
#include "NakedFunctions/WindowedBorderless.hh"

//==========================================================================
// Patches

void InitializePatches()
{
    // Affinity Mode
    int _AffinityMode = CoreSettings::GetInteger("Patch", "AffinityMode");
    if (_AffinityMode != 2)
    {
        // Prevent game setting affinity to 1 core.
        CorePatcher::NopBytes(0x6DF1F6, 5);

        // Remove affinity on core 0 which might improve performance on Hyper-threaded CPU.
        if (_AffinityMode == 1)
        {
            SYSTEM_INFO _SystemInfo;
            GetSystemInfo(&_SystemInfo);

            if (_SystemInfo.dwNumberOfProcessors > 2)
            {
                DWORD_PTR _AffinityMask = 0;
                for (DWORD i = 1; _SystemInfo.dwNumberOfProcessors > i; ++i) {
                    _AffinityMask |= (1 << i);
                }
                SetProcessAffinityMask(GetCurrentProcess(), _AffinityMask);
            }
        }
    }

    // Fix vertexes been broken when using D3DLOCK_DISCARD, is better to force both flags to prevent cpu spin-lock...
    CorePatcher::ApplyType<DWORD>(0x703A20, (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE));

    // Remove `D3DCREATE_MULTITHREADED` since D3D runs in single threaded anyway and it might degrade performance...
    CorePatcher::ApplyBytes(0x654965, { uint8_t(D3DCREATE_HARDWARE_VERTEXPROCESSING) });

    // Character Camera - Disable blending.
    CorePatcher::ApplyBytes(0x5636F6, { 0xE9, 0x4E, 0x01, 0x00, 0x00 });

    // Character Camera - Rattle Fix for BarrelCreep Side (Max value)
    CorePatcher::JmpRel32(0x563CB2, WeaponStateProp_GetBarrelCreepSideMax);

    // renderer::Display_List (Fix culling issues)
    CorePatcher::ApplyByte(0x45E6E9, 0x0); // None cull mode (PS2 uses PDDI_CULL_SHADOW_BACKFACE, but this doesn't exist in PC version)

    //=============================================================
    // FPS Patches
    
    // Ocean (Animation)
    CorePatcher::ApplyBytes(0x6A0B85, { 0xEB, 0x27 });

    // Animation (Test)
    // - Seems to pass (16ms) as min limit which would be fine for 60 FPS, but running game at higher FPS will cause animations to play at faster speed.
    CorePatcher::ApplyByte(0x40D723, 0x30); // Patches float address to other float address which is small enough for 1000+ FPS

    //=============================================================
    // ActiveMARK (DRM) Patches

    // Prevent initializing internet stuff & wont load rasapi32.dll (More info: https://learn.microsoft.com/en-us/windows/win32/api/_rras/)
    CorePatcher::ApplyBytes(0x974395, { 0xB0, 0x01, 0xC3 });

    //=============================================================
    // Configurable Patches

    int nVibrance = CoreSettings::GetInteger("Scarface", "Vibrance");
    if (nVibrance)
    {
        static float s_VibranceFloat = fmaxf(0.f, fminf((static_cast<float>(nVibrance) * 0.01f), 1.f));
        CorePatcher::ApplyType(0x65163E, &s_VibranceFloat);
    }

    if (CoreSettings::GetInteger("Scarface", "ShowFPS")) {
        CorePatcher::NopBytes(0x658E6A, 2);
    }

    if (CoreSettings::GetInteger("Scarface", "SkipLicenseScreen")) {
        *reinterpret_cast<float*>(0x8234CC) = 3000.f;
    }

    if (CoreSettings::GetInteger("Scarface", "SkipMovies")) {
        CorePatcher::NopBytes(0x4F61F0, 5);
    }
}

//==========================================================================
// Hooks

// pure3d
#include "Hooks/pure3d/OceanRenderer.hh"
#include "Hooks/pure3d/VehicleShader.hh"

// script
#include "Hooks/script/ListScreenResolutionEntries.hh"

// game
#include "Hooks/Registry.hh"

template <typename H, typename O = void*>
__forceinline void AddHook(uintptr_t p_Address, H p_Hook, O* p_Original = nullptr)
{
    MH_CreateHook(reinterpret_cast<void*>(p_Address), reinterpret_cast<void*>(p_Hook), reinterpret_cast<void**>(p_Original));
}

void AddVFuncHook(uintptr_t p_Address, void* p_Hook, void** p_Original = nullptr)
{
    if (p_Original) {
        *reinterpret_cast<void**>(p_Original) = *reinterpret_cast<void**>(p_Address);
    }

    CorePatcher::ApplyData(reinterpret_cast<void*>(p_Address), &p_Hook, sizeof(void*));
}

template <typename O = void*>
__forceinline void AddVFuncHook(uintptr_t p_Address, void* p_Hook, O* p_Original = nullptr)
{
    AddVFuncHook(p_Address, p_Hook, (void**)p_Original);
}

void InitializeHooks()
{
    MH_Initialize();

    // script
    AddHook(0x456B10, ScriptHook::ListScreenResolutionEntries);

    // pure3d
    AddVFuncHook(0x76B790, pure3dHook::OceanRender::UnknownUpdate, &pure3dHook::OceanRender::g_UnknownUpdate);
    AddVFuncHook(0x774D3C, pure3dHook::VehicleShader::UnknownRender, &pure3dHook::VehicleShader::g_UnknownRender);

    // Registry
    AddHook(0x456FD0, RegistryHook::SetInteger);
    AddHook(0x457090, RegistryHook::GetInteger);
}

//==========================================================================
// Globals

void InitializeGlobals()
{
    // Disable hide cursor (Let DInput handle cursor)
    *reinterpret_cast<bool*>(0x7BF728) = false;

    //=============================================================
    // Configurable Globals

    auto nWindowedMode = static_cast<CoreSettings::eWindowedMode>(CoreSettings::GetInteger("Windowed", "Mode"));
    if (nWindowedMode != CoreSettings::eWindowedMode_None)
    {
        CorePatcher::ApplyType(0x4586F2, pure3d::PDDI_DISPLAY_WINDOW);

        if (nWindowedMode == CoreSettings::eWindowedMode_Borderless) {
            CorePatcher::MakeJmpRel32(0x45728F, Core_WindowedBorderless); // We don't have enough bytes to patch...
        }
    }

    if (CoreSettings::GetInteger("PostProcessFX", "Enable")) 
    {
        *reinterpret_cast<bool*>(0x830A0C) = true;

        float flBloomValue = static_cast<float>(CoreSettings::GetInteger("PostProcessFX", "Bloom"));
        if (flBloomValue > 0.f)
        {
            flBloomValue = fminf(flBloomValue, 1.f);
            CorePatcher::ApplyBytes(0x6523E0, { 0xBA, 0x00, 0x00, 0x00, 0x00, 0x90 });
            CorePatcher::ApplyType(0x6523E1, flBloomValue);
        }
    }

    if (CoreSettings::GetInteger("Patch", "DebugMenu"))
    {
        *reinterpret_cast<bool*>(0x7C1C54) = false; // gReleaseMode
        *reinterpret_cast<bool*>(0x7C1C55) = false; // gFinalMode 
    }
}

//==========================================================================
// Fixes

void InitializeFixes()
{
    // Modify NtGlobalFlag
    {
        DWORD _Peb = __readfsdword(0x30);

        //==========================
        // Remove Heap Flags:
        // - FLG_HEAP_VALIDATE_PARAMETERS | FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_ENABLE_TAIL_CHECK

        *reinterpret_cast<ULONG*>(_Peb + 0x68) &= ~(0x00000040 | 0x00000020 | 0x00000010);
    }

    // Modify Heap Flags
    {
        DWORD _NumHeaps = GetProcessHeaps(0, 0);
        if (_NumHeaps)
        {
            HANDLE* _Heaps = new HANDLE[_NumHeaps];

            if (GetProcessHeaps(_NumHeaps, _Heaps))
            {
                for (DWORD i = 0; _NumHeaps > i; ++i)
                {
                    uintptr_t _Heap = reinterpret_cast<uintptr_t>(_Heaps[i]);

                    //==========================
                    // Remove Flags:
                    // - HEAP_VALIDATE_PARAMETERS_ENABLED | HEAP_FREE_CHECKING_ENABLED | HEAP_TAIL_CHECKING_ENABLED

                    *reinterpret_cast<ULONG*>(_Heap + 0x40) &= ~(0x40000000 | 0x00000040 | 0x00000020);

                    //==========================
                    // Remove (Force Flags)

                    *reinterpret_cast<ULONG*>(_Heap + 0x44) = 0;
                }
            }

            delete[] _Heaps;
        }
    }

    //=============================================================
    // Vehicle windows (Damage)

    CorePatcher::NopBytes(0x7080C6, 2); // Removes float sign change
    CorePatcher::ApplyType<void*>(0x708899, g_VehicleGlassShader); // Replace shader address
}

//==========================================================================

int __stdcall DllMain(HMODULE p_Module, DWORD p_Reason, void* p_Reserved)
{
    if (p_Reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(p_Module);

        if (!SDK::IsGameVersionValid())
        {
            MessageBoxA(0, "You're using wrong game version, only v1.00.2 is usable!\nVisit abandonware site for download or find alternative way to obtain this version.", "GangstaPatch", MB_OK | MB_ICONERROR);
            exit(1);
            return 0;
        }

        // Setup working directory just in-case...
        {
            char szCurrentExecutablePath[MAX_PATH];
            if (GetModuleFileNameA(0, szCurrentExecutablePath, sizeof(szCurrentExecutablePath)))
            {
                char* pLastDelimer = strrchr(szCurrentExecutablePath, '\\');
                if (pLastDelimer)
                {
                    *pLastDelimer = '\0';
                    SetCurrentDirectoryA(szCurrentExecutablePath);
                }
            }
        }

        g_VirtualProtect = reinterpret_cast<decltype(&VirtualProtect)>(MH_GetVirtualProtect());

        InitializePatches();
        InitializeHooks();
        InitializeGlobals();
        InitializeFixes();
    }

    return 1;
}
