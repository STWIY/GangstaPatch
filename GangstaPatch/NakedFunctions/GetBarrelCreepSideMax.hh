#pragma once

__declspec(naked) void WeaponStateProp_GetBarrelCreepSideMax()
{
    __asm
    {
        mov eax, [ecx + 0x150] // mTemplate
        fld DWORD PTR[eax + 0x64]  // m_BarrelCreepSideMax
        ret
    }
}