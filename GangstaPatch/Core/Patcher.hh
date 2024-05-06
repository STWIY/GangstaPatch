#pragma once
#include <initializer_list>
#include <intrin.h>

namespace CorePatcher
{
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

    template <typename T>
    __forceinline bool ApplyType(uintptr_t p_Address, T p_Value)
    {
        return ApplyData(reinterpret_cast<void*>(p_Address), &p_Value, sizeof(T));
    }

    //==============================================================

    __forceinline bool ApplyBytes(uintptr_t p_Address, std::initializer_list<uint8_t> p_InitializerList)
    {
        size_t size = p_InitializerList.size();
        return ApplyData(reinterpret_cast<void*>(p_Address), (void*)(p_InitializerList.begin()), size);
    }

    __forceinline bool ApplyByte(uintptr_t p_Address, uint8_t p_Byte)
    {
        return ApplyBytes(p_Address, { p_Byte });
    }

    //==============================================================

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

    //==============================================================
    // p_Address -> E8 (Call) / E9 (Jump)

    bool JmpRel32(uintptr_t p_Address, void* p_Target)
    {
        uintptr_t jmpOffset = (reinterpret_cast<uintptr_t>(p_Target) - p_Address - 5);
        void* pPatchAddress = reinterpret_cast<void*>(p_Address + 0x1);

        return ApplyData(pPatchAddress, &jmpOffset, sizeof(jmpOffset));
    }

    bool MakeCallRel32(uintptr_t p_Address, void* p_Target)
    {
        return (ApplyByte(p_Address, 0xE8) && JmpRel32(p_Address, p_Target));
    }

    bool MakeJmpRel32(uintptr_t p_Address, void* p_Target)
    {
        return (ApplyByte(p_Address, 0xE9) && JmpRel32(p_Address, p_Target));
    }
};