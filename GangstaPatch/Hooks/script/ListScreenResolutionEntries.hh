#pragma once
#include <set>

namespace ScriptHook
{
	void __stdcall ListScreenResolutionEntries()
	{
		std::set<std::pair<DWORD, DWORD>> _DisplayList;
		{
			DEVMODEA _DevMode;
			{
				ZeroMemory(&_DevMode, sizeof(_DevMode));
				_DevMode.dmSize = sizeof(_DevMode);
			}

			for (int _ModeEnum = 0; EnumDisplaySettingsA(0, _ModeEnum, &_DevMode) != 0; ++_ModeEnum) {
				_DisplayList.emplace(_DevMode.dmPelsWidth, _DevMode.dmPelsHeight);
			}
		}

		for (auto& _Pair : _DisplayList) {
			reinterpret_cast<void(*)(char*, ...)>(0x491A70)(reinterpret_cast<char*>(0x7375F0), _Pair.second, _Pair.first, 32);
		}
	}
}