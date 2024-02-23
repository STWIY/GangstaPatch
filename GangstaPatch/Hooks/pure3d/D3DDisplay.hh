#pragma once

namespace pure3dHook
{
	namespace D3DDisplay
	{
		typedef bool(__thiscall* Fn_InitDisplay)(pure3d::d3dDisplay*, pure3d::pddiDisplayInit*);
		Fn_InitDisplay g_InitDisplay;

		bool __fastcall InitDisplay(pure3d::d3dDisplay* p_Display, void* edx, pure3d::pddiDisplayInit* p_Init)
		{
			auto _WindowedMode = static_cast<CoreSettings::eWindowedMode>(CoreSettings::GetInteger("Windowed", "Mode"));

			p_Init->displayMode = (_WindowedMode == CoreSettings::eWindowedMode_None ? pure3d::PDDI_DISPLAY_FULLSCREEN : pure3d::PDDI_DISPLAY_WINDOW);
			if (g_InitDisplay(p_Display, p_Init))
			{
				if (_WindowedMode == CoreSettings::eWindowedMode_Borderless) {
					SetWindowLongA(p_Display->hWnd, GWL_STYLE, WS_POPUP);
				}
				return true;
			}

			return false;
		}
	}
}