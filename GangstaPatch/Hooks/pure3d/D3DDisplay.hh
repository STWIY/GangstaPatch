#pragma once

namespace pure3dHook
{
	namespace D3DDisplay
	{
		typedef bool(__thiscall* Fn_InitDisplay)(pure3d::d3dDisplay*, pure3d::pddiDisplayInit*);
		Fn_InitDisplay g_InitDisplay;

		bool __fastcall InitDisplay(pure3d::d3dDisplay* p_Display, void* edx, pure3d::pddiDisplayInit* p_Init)
		{
			p_Init->displayMode = (s_CoreSettings.m_Windowed.m_Mode == CoreSettings::Windowed_t::eMode_None ? pure3d::PDDI_DISPLAY_FULLSCREEN : pure3d::PDDI_DISPLAY_WINDOW);
			if (g_InitDisplay(p_Display, p_Init))
			{
				if (s_CoreSettings.m_Windowed.m_Mode == CoreSettings::Windowed_t::eMode_Borderless) {
					SetWindowLongA(p_Display->hWnd, GWL_STYLE, WS_POPUP);
				}
				return true;
			}

			return false;
		}
	}
}