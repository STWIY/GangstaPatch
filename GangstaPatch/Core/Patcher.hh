#pragma once
#include <initializer_list>
#include <intrin.h>
#include <cstdint>

namespace CorePatcher
{
    extern decltype(&VirtualProtect) g_VirtualProtect;

    //==============================================================

    bool ApplyData(void* p_Address, void* p_Data, size_t p_Size);

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

    bool NopBytes(uintptr_t p_Address, size_t p_NumBytes);

    //==============================================================
    // p_Address -> E8 (Call) / E9 (Jump)

    bool JmpRel32(uintptr_t p_Address, void* p_Target);

    __forceinline bool MakeCallRel32(uintptr_t p_Address, void* p_Target)
    {
        return (ApplyByte(p_Address, 0xE8) && JmpRel32(p_Address, p_Target));
    }

    __forceinline  bool MakeJmpRel32(uintptr_t p_Address, void* p_Target)
    {
        return (ApplyByte(p_Address, 0xE9) && JmpRel32(p_Address, p_Target));
    }
};