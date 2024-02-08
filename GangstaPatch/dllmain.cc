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

void InitializePatches()
{
    // Prevent game setting affinity to 1 core.
    //CorePatcher::NopBytes(0x6DF1F6, 5);
    // FIX-ME: Causes profile dialog have missing text and button options...

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
