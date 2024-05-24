#include "../../../Includes.hh"
#include <set>

namespace ScriptHook
{
	void __stdcall ListScreenResolutionEntries()
	{
		std::set<std::pair<DWORD, DWORD>> sDisplayList;
		{
			DEVMODEA dvMode;
			{
				ZeroMemory(&dvMode, sizeof(dvMode));
				dvMode.dmSize = sizeof(dvMode);
			}

			for (int iModeEnum = 0; EnumDisplaySettingsA(0, iModeEnum, &dvMode) != 0; ++iModeEnum) {
				sDisplayList.emplace(dvMode.dmPelsWidth, dvMode.dmPelsHeight);
			}
		}

		for (auto& pPair : sDisplayList) {
			reinterpret_cast<void(*)(char*, ...)>(0x491A70)(reinterpret_cast<char*>(0x7375F0), pPair.second, pPair.first, 32);
		}
	}
}