#pragma once

namespace RegistryHook
{
	bool __cdecl SetInteger(const char* p_Key, int p_Value)
	{
		return s_CoreSettings.SetInteger("Scarface", p_Key, p_Value);
	}

	int __cdecl GetInteger(const char* p_Key)
	{
		return s_CoreSettings.GetInteger("Scarface", p_Key);
	}
}