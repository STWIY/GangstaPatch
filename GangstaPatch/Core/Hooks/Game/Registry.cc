#include "../../../Includes.hh"

namespace RegistryHook
{
	bool __cdecl SetInteger(const char* p_Key, int p_Value)
	{
		return CoreSettings::SetInteger("Scarface", p_Key, p_Value);
	}

	int __cdecl GetInteger(const char* p_Key)
	{
		return CoreSettings::GetInteger("Scarface", p_Key);
	}
}