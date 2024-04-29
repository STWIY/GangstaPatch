#pragma once

namespace pure3dHook::VehicleShader
{
	typedef void(__thiscall* Fn_UnknownRender)(uintptr_t);
	Fn_UnknownRender g_UnknownRender;

	void __fastcall UnknownRender(uintptr_t shader, uintptr_t edx)
	{
		float transparency = *reinterpret_cast<float*>(shader + 0x98);
		if (1.f > transparency)
		{
			float damagePercentage = *reinterpret_cast<float*>(shader + 0x94);
			float* transparencyMultiplier = reinterpret_cast<float*>(0x7FC5F0); // This is literally retarded, no other way to make the window vanish...
			if (damagePercentage >= 1.f) {
				*transparencyMultiplier = -100.f;
			}
			else {
				*transparencyMultiplier = 1.f;
			}
		}

		g_UnknownRender(shader);
	}
}