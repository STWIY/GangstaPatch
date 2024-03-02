//==========================================================================
// Includes

#include <cstdint>
#include <Windows.h>
#include <d3d9.h>
#include <sysinfoapi.h>

#include <SDK/.SDK.hh>

//==========================================================================
// Libs

#include "3rdParty/MinHook.h"
#pragma comment(lib, "MinHook.lib")

//==========================================================================
// Core

#include "Core/Patcher.hh"
#include "Core/Settings.hh"

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
                DWORD_PTR _AffinityMask;
                for (DWORD i = 1; _SystemInfo.dwNumberOfProcessors > i; ++i) {
                    _AffinityMask |= (1 << i);
                }
                SetProcessAffinityMask(GetCurrentProcess(), _AffinityMask);
            }
        }
    }

    // Fix vertexes been broken when using D3DLOCK_DISCARD, is better to force both flags to prevent cpu spin-lock...
    CorePatcher::ApplyDWORD(0x703A20, (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE));

    // Remove `D3DCREATE_MULTITHREADED` since D3D runs in single threaded anyway and it might degrade performance...
    CorePatcher::ApplyBytes(0x654965, { uint8_t(D3DCREATE_HARDWARE_VERTEXPROCESSING) });

    if (CoreSettings::GetInteger("Scarface", "ShowFPS")) {
        CorePatcher::NopBytes(0x658E6A, 2);
    }

    if (CoreSettings::GetInteger("Scarface", "SkipLicenseScreen")) {
        *reinterpret_cast<float*>(0x8234CC) = 3000.f;
    }

    if (CoreSettings::GetInteger("Scarface", "SkipMovies")) {
        CorePatcher::NopBytes(0x4F61F0, 5);
    }

    if (CoreSettings::GetInteger("Scarface", "DebugMenu")) 
    {
        *reinterpret_cast<bool*>(0x7C1C54) = false; // gReleaseMode
        *reinterpret_cast<bool*>(0x7C1C55) = false; // gFinalMode 
    }
}

//==========================================================================
// Hooks

#include "Hooks/script/ListScreenResolutionEntries.hh"
#include "Hooks/pure3d/D3DDisplay.hh"
#include "Hooks/Registry.hh"

template <typename H, typename O = void*>
__forceinline void AddHook(uintptr_t p_Address, H p_Hook, O* p_Original = nullptr)
{
    MH_CreateHook(reinterpret_cast<void*>(p_Address), reinterpret_cast<void*>(p_Hook), reinterpret_cast<void**>(p_Original));
}

void InitializeHooks()
{
    MH_Initialize();

    // script
    AddHook(0x456B10, ScriptHook::ListScreenResolutionEntries);

    // pure3d
    AddHook(0x6545D0, pure3dHook::D3DDisplay::InitDisplay, &pure3dHook::D3DDisplay::g_InitDisplay);

    // Registry
    AddHook(0x456FD0, RegistryHook::SetInteger);
    AddHook(0x457090, RegistryHook::GetInteger);
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
            char _CurrentExecutablePath[MAX_PATH];
            if (GetModuleFileNameA(0, _CurrentExecutablePath, sizeof(_CurrentExecutablePath)))
            {
                char* _LastDelimer = strrchr(_CurrentExecutablePath, '\\');
                if (_LastDelimer) 
                {
                    *_LastDelimer = '\0';
                    SetCurrentDirectoryA(_CurrentExecutablePath);
                }
            }
        }

        CorePatcher::s_VirtualProtect = reinterpret_cast<decltype(&VirtualProtect)>(MH_GetVirtualProtect());

        InitializePatches();
        InitializeHooks();
        InitializeFixes();
    }

    return 1;
}
