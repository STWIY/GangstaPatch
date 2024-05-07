#pragma once

__declspec(naked) void Core_WindowedBorderless()
{
    __asm
    {
        push edx
        push eax
        push ecx
        push WS_POPUP

        mov eax, 0x457294
        jmp eax
    }
}