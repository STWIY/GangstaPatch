#pragma once

//==========================================================================
// Declarations

namespace RegistryHook
{
    bool __cdecl SetInteger(const char*, int);

    int __cdecl GetInteger(const char*);
}

namespace GameObjectHook
{
    namespace ControllerInput
    {
        void __fastcall ReadControllerInput(::ControllerInput*, void*, ActionMap*);
    }
}

namespace P3DHook
{
    namespace OceanRender
    {
        extern void* g_UnknownUpdate;
        void __fastcall UnknownUpdate(uintptr_t, uintptr_t, void*, void*, int);
    }

    namespace VehicleShader
    {
        extern void* g_UnknownRender;
        void __fastcall UnknownRender(uintptr_t, uintptr_t);
    }
}

namespace ScriptHook
{
    void __stdcall ListScreenResolutionEntries();
}

//==========================================================================
// Hooks

namespace Hooks
{
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

	void Initialize()
	{
        MH_Initialize();

        // Game
        AddHook(0x456FD0, RegistryHook::SetInteger);
        AddHook(0x457090, RegistryHook::GetInteger);

        // GameObject
        CorePatcher::MakeCallRel32(0x57AB9F, GameObjectHook::ControllerInput::ReadControllerInput);

        // P3D
        AddVFuncHook(0x76B790, P3DHook::OceanRender::UnknownUpdate, &P3DHook::OceanRender::g_UnknownUpdate);
        AddVFuncHook(0x774D3C, P3DHook::VehicleShader::UnknownRender, &P3DHook::VehicleShader::g_UnknownRender);

        // Script
        AddHook(0x456B10, ScriptHook::ListScreenResolutionEntries);
	}
}