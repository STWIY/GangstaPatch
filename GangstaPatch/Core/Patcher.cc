#include <Windows.h>
#include "Patcher.hh"

namespace CorePatcher
{
    decltype(&VirtualProtect) g_VirtualProtect;

    //==============================================================

    bool ApplyData(void* p_Address, void* p_Data, size_t p_Size)
    {
        DWORD oldProtection;
        if (!g_VirtualProtect(p_Address, p_Size, PAGE_EXECUTE_READWRITE, &oldProtection)) {
            return false;
        }

        __movsb(reinterpret_cast<uint8_t*>(p_Address), reinterpret_cast<uint8_t*>(p_Data), p_Size);

        g_VirtualProtect(p_Address, p_Size, oldProtection, &oldProtection);
        return true;
    }


    bool NopBytes(uintptr_t p_Address, size_t p_NumBytes)
    {
        DWORD oldProtection;
        if (!g_VirtualProtect(reinterpret_cast<void*>(p_Address), p_NumBytes, PAGE_EXECUTE_READWRITE, &oldProtection)) {
            return false;
        }

        __stosb(reinterpret_cast<uint8_t*>(p_Address), 0x90, p_NumBytes);

        g_VirtualProtect(reinterpret_cast<void*>(p_Address), p_NumBytes, oldProtection, &oldProtection);
        return true;
    }

    bool JmpRel32(uintptr_t p_Address, void* p_Target)
    {
        uintptr_t jmpOffset = (reinterpret_cast<uintptr_t>(p_Target) - p_Address - 5);
        void* pPatchAddress = reinterpret_cast<void*>(p_Address + 0x1);

        return ApplyData(pPatchAddress, &jmpOffset, sizeof(jmpOffset));
    }
};