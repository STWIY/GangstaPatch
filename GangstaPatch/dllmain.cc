//==========================================================================
// Includes

#include <cstdint>
#include <Windows.h>
#include <d3d9.h>

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

#define HEAP_VALIDATE_ALL_ENABLED       0x20000000
#define HEAP_CAPTURE_STACK_BACKTRACES   0x08000000
#define RTLDEBUGCREATEHEAP_HEAP_FLAGS   (HEAP_TAIL_CHECKING_ENABLED | HEAP_FREE_CHECKING_ENABLED | 0x10000000)
#define NTGLOBALFLAGS_HEAP_FLAGS        (HEAP_DISABLE_COALESCE_ON_FREE | HEAP_FREE_CHECKING_ENABLED | HEAP_TAIL_CHECKING_ENABLED | HEAP_VALIDATE_ALL_ENABLED | 0x40000000 | HEAP_CAPTURE_STACK_BACKTRACES)
#define HEAP_CLEARABLE_FLAGS            (RTLDEBUGCREATEHEAP_HEAP_FLAGS | NTGLOBALFLAGS_HEAP_FLAGS)
#define HEAP_VALID_FORCE_FLAGS          (HEAP_NO_SERIALIZE | HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY | HEAP_REALLOC_IN_PLACE_ONLY | 0x40000000 | HEAP_VALIDATE_ALL_ENABLED | HEAP_TAIL_CHECKING_ENABLED | HEAP_CREATE_ALIGN_16 | HEAP_FREE_CHECKING_ENABLED)
#define HEAP_CLEARABLE_FORCE_FLAGS      (HEAP_CLEARABLE_FLAGS & HEAP_VALID_FORCE_FLAGS)

void PEBHeapFlags()
{
    char* peb = (char*)__readfsdword(0x30);

    DWORD numOfHeaps = *(int*)(peb + 0x88);

    for (DWORD i = 0; i < numOfHeaps; ++i)
    {
        char* heap = (char*)(*(DWORD***)(peb + 0x90))[i];

        auto flags = (DWORD*)(heap + 0x40);
        auto force_flags = (DWORD*)(heap + 0x44);

        *flags &= ~HEAP_CLEARABLE_FLAGS;
        *force_flags &= ~HEAP_CLEARABLE_FORCE_FLAGS;
    }
}

void InitializePatches()
{
    // Prevent game setting affinity to 1 core.
    CorePatcher::NopBytes(0x6DF1F6, 5);

    // FIX-ME: Causes profile dialog have missing text and button options...
    PEBHeapFlags();

    // Fix vertexes been broken when using D3DLOCK_DISCARD, is better to force both flags to prevent cpu spin-lock...
    CorePatcher::ApplyDWORD(0x703A20, (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE));

    // Remove `D3DCREATE_MULTITHREADED` since D3D runs in single threaded anyway and it might degrade performance...
    CorePatcher::ApplyBytes(0x654965, { uint8_t(D3DCREATE_HARDWARE_VERTEXPROCESSING) });
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

        s_CoreSettings.Initialize();

        CorePatcher::s_VirtualProtect = reinterpret_cast<decltype(&VirtualProtect)>(MH_GetVirtualProtect());

        InitializePatches();
        InitializeHooks();
    }

    return 1;
}
