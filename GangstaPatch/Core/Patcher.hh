#pragma once
#include <initializer_list>

namespace CorePatcher
{
    static decltype(&VirtualProtect) s_VirtualProtect;

    //==============================================================

    __forceinline bool ApplyDWORD(uintptr_t p_Address, DWORD p_Value)
    {
        DWORD _OldProtection;
        if (!s_VirtualProtect(reinterpret_cast<void*>(p_Address), sizeof(DWORD), PAGE_EXECUTE_READWRITE, &_OldProtection)) {
            return false;
        }

        *reinterpret_cast<DWORD*>(p_Address) = p_Value;

        s_VirtualProtect(reinterpret_cast<void*>(p_Address), sizeof(DWORD), _OldProtection, &_OldProtection);
        return true;
    }

    //==============================================================

    __forceinline bool ApplyBytes(uintptr_t p_Address, std::initializer_list<uint8_t> _InitializerList)
    {
        size_t _Size = _InitializerList.size();

        DWORD _OldProtection;
        if (!s_VirtualProtect(reinterpret_cast<void*>(p_Address), _Size, PAGE_EXECUTE_READWRITE, &_OldProtection)) {
            return false;
        }

        memcpy(reinterpret_cast<void*>(p_Address), _InitializerList.begin(), _Size);
        s_VirtualProtect(reinterpret_cast<void*>(p_Address), _Size, _OldProtection, &_OldProtection);
        return true;
    }

    //==============================================================

    __forceinline bool NopBytes(uintptr_t p_Address, size_t p_NumBytes)
    {
        DWORD _OldProtection;
        if (!s_VirtualProtect(reinterpret_cast<void*>(p_Address), p_NumBytes, PAGE_EXECUTE_READWRITE, &_OldProtection)) {
            return false;
        }

        memset(reinterpret_cast<void*>(p_Address), 0x90, p_NumBytes);
        s_VirtualProtect(reinterpret_cast<void*>(p_Address), p_NumBytes, _OldProtection, &_OldProtection);
        return true;
    }
};