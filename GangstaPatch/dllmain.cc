//==========================================================================
// Includes

#include "Includes.hh"
#include "3rdParty/CRT-STL/CRT-STL.hh"
#include "Core/Hooks.hh"

//==========================================================================

#include "Shader/VehicleGlass.hh"

//==========================================================================
// Naked Functions

#include "NakedFunctions/GetBarrelCreepSideMax.hh"
#include "NakedFunctions/WindowedBorderless.hh"

//==========================================================================
// Patches

void InitializePatches()
{
    using namespace CorePatcher;

    // Affinity Mode
    int _AffinityMode = CoreSettings::GetInteger("Patch", "AffinityMode");
    if (_AffinityMode != 2)
    {
        // Prevent game setting affinity to 1 core.
        NopBytes(0x6DF1F6, 5);

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
    ApplyType<DWORD>(0x703A20, (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE));

    // Remove `D3DCREATE_MULTITHREADED` since D3D runs in single threaded anyway and it might degrade performance...
    ApplyBytes(0x654965, { uint8_t(D3DCREATE_HARDWARE_VERTEXPROCESSING) });

    // Change camera nearPlane to fix flickering/z fighting...
    const float fNearPlane = 1.f;
    ApplyType(0x69EB16, fNearPlane);
    ApplyType(0x459D57, fNearPlane);

    // Character Camera - Disable blending.
    ApplyBytes(0x5636F6, { 0xE9, 0x4E, 0x01, 0x00, 0x00 });

    // Character Camera - Rattle Fix for BarrelCreep Side (Max value)
    JmpRel32(0x563CB2, WeaponStateProp_GetBarrelCreepSideMax);

    //--------------------------------------------------------------
    // Rendering Patches

    // renderer::Display_List (Fix culling issues)
    ApplyByte(0x45E6E9, 0x0); // None cull mode (PS2 uses PDDI_CULL_SHADOW_BACKFACE, but this doesn't exist in PC version)

    // Fix shadow rendering issue in certain areas
    // - This might not be proper way to fix this, but there is first list that is rendering always single model which isn't even visible on screen.
    {
        NopBytes(0x45E26A, 2);
        ApplyBytes(0x45E270, { 0xE9, 0xAE, 0x00 });
    }

    //--------------------------------------------------------------
    // FPS Patches
    
    // Ocean (Animation)
    ApplyBytes(0x6A0B85, { 0xEB, 0x27 });

    // Animation (Test)
    // - Seems to pass (16ms) as min limit which would be fine for 60 FPS, but running game at higher FPS will cause animations to play at faster speed.
    ApplyByte(0x40D723, 0x30); // Patches float address to other float address which is small enough for 1000+ FPS

    //--------------------------------------------------------------
    // ActiveMARK (DRM) Patches

    // Prevent initializing internet stuff & wont load rasapi32.dll (More info: https://learn.microsoft.com/en-us/windows/win32/api/_rras/)
    ApplyBytes(0x974395, { 0xB0, 0x01, 0xC3 });

    //--------------------------------------------------------------
    // Configurable Patches

    int nVibrance = CoreSettings::GetInteger("Scarface", "Vibrance");
    if (nVibrance)
    {
        static float s_VibranceFloat = fmaxf(0.f, fminf((static_cast<float>(nVibrance) * 0.01f), 1.f));
        ApplyType(0x65163E, &s_VibranceFloat);
    }

    if (CoreSettings::GetInteger("Scarface", "ShowFPS")) {
        NopBytes(0x658E6A, 2);
    }

    if (CoreSettings::GetInteger("Scarface", "SkipLicenseScreen")) {
        *reinterpret_cast<float*>(0x8234CC) = 3000.f;
    }

    if (CoreSettings::GetInteger("Scarface", "SkipMovies")) {
        NopBytes(0x4F61F0, 5);
    }
}

//==========================================================================
// Hooks

#include "Core/Hooks.hh"

//==========================================================================
// Globals

void InitializeGlobals()
{
    using namespace CorePatcher;

    // Disable hide cursor (Let DInput handle cursor)
    *reinterpret_cast<bool*>(0x7BF728) = false;

    //--------------------------------------------------------------
    // Configurable Globals

    auto nWindowedMode = static_cast<CoreSettings::eWindowedMode>(CoreSettings::GetInteger("Windowed", "Mode"));
    if (nWindowedMode != CoreSettings::eWindowedMode_None)
    {
        ApplyType(0x4586F2, pure3d::PDDI_DISPLAY_WINDOW);

        if (nWindowedMode == CoreSettings::eWindowedMode_Borderless) {
            MakeJmpRel32(0x45728F, Core_WindowedBorderless); // We don't have enough bytes to patch...
        }
    }

    if (CoreSettings::GetInteger("PostProcessFX", "Enable")) 
    {
        *reinterpret_cast<bool*>(0x830A0C) = true;

        float flBloomValue = static_cast<float>(CoreSettings::GetInteger("PostProcessFX", "Bloom"));
        if (flBloomValue > 0.f)
        {
            flBloomValue = fminf(flBloomValue, 1.f);
            ApplyBytes(0x6523E0, { 0xBA, 0x00, 0x00, 0x00, 0x00, 0x90 });
            ApplyType(0x6523E1, flBloomValue);
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
    using namespace CorePatcher;

    // Modify NtGlobalFlag
    {
        DWORD dwPeb = __readfsdword(0x30);

        //==========================
        // Remove Heap Flags:
        // - FLG_HEAP_VALIDATE_PARAMETERS | FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_ENABLE_TAIL_CHECK

        *reinterpret_cast<ULONG*>(dwPeb + 0x68) &= ~(0x00000040 | 0x00000020 | 0x00000010);
    }

    // Modify Heap Flags
    {
        DWORD dwNumHeaps = GetProcessHeaps(0, 0);
        if (dwNumHeaps)
        {
            HANDLE* pHeaps = new HANDLE[dwNumHeaps];

            if (GetProcessHeaps(dwNumHeaps, pHeaps))
            {
                for (DWORD i = 0; dwNumHeaps > i; ++i)
                {
                    uintptr_t uHeap = reinterpret_cast<uintptr_t>(pHeaps[i]);

                    //==========================
                    // Remove Flags:
                    // - HEAP_VALIDATE_PARAMETERS_ENABLED | HEAP_FREE_CHECKING_ENABLED | HEAP_TAIL_CHECKING_ENABLED

                    *reinterpret_cast<ULONG*>(uHeap + 0x40) &= ~(0x40000000 | 0x00000040 | 0x00000020);

                    //==========================
                    // Remove (Force Flags)

                    *reinterpret_cast<ULONG*>(uHeap + 0x44) = 0;
                }
            }

            delete[] pHeaps;
        }
    }

    //--------------------------------------------------------------
    // Vehicle windows (Damage)

    NopBytes(0x7080C6, 2); // Removes float sign change
    ApplyType<void*>(0x708899, g_VehicleGlassShader); // Replace shader address
}

//==========================================================================

int __stdcall DllMain(HMODULE p_Module, DWORD p_Reason, void* p_Reserved)
{
    if (p_Reason == DLL_PROCESS_ATTACH)
    {
        CRT::Initialize(); // CRT
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

        CorePatcher::g_VirtualProtect = reinterpret_cast<decltype(&VirtualProtect)>(MH_GetVirtualProtect());

        InitializePatches();
        InitializeGlobals();
        InitializeFixes();

        Hooks::Initialize();
    }

    return 1;
}
