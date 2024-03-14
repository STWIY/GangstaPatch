#pragma once

namespace pure3dHook
{
	namespace OceanRender
	{
		typedef void(__thiscall* Fn_UnknownUpdate)(uintptr_t, void*, void*, int);
		Fn_UnknownUpdate g_UnknownUpdate;

		void __fastcall UnknownUpdate(uintptr_t ecx, uintptr_t edx, void* p_Camera, void* p_UnknownArg, int p_UnusedArg)
		{
			g_UnknownUpdate(ecx, p_Camera, p_UnknownArg, p_UnusedArg);

			// Update only when we're below delta...
			{
				static float s_DeltaCounter = 0.f;
				s_DeltaCounter -= pure3d::d3dContext::GetInstance()->stats[pure3d::PDDI_STAT_FRAME_TIME];
				if (s_DeltaCounter > 0.f) {
					return;
				}

				static const float s_DeltaUpdate = (1000.f / 30.f);
				s_DeltaCounter = s_DeltaUpdate;
			}

			int _KeyFrame = *reinterpret_cast<int*>(ecx + 0x208) + 1;
			*reinterpret_cast<int*>(ecx + 0x208) = *reinterpret_cast<int*>(ecx + 0x20C) = (_KeyFrame % 30);
		}
	}
}