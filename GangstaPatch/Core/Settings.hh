#pragma once

//======================================================================
// 
//	Description: Small header file to handle ini settings that's
//				 wrapped around Windows API.
// 
//======================================================================

#define CORE_SETTINGS_FILE_NAME	".\\settings.ini"

class CoreSettings
{
public:
	enum eWindowedMode : uint8_t
	{
		eWindowedMode_None = 0,
		eWindowedMode_Normal,
		eWindowedMode_Borderless
	};

	//=======================================================
	// Wrappers

	__forceinline static bool SetInteger(const char* p_Section, const char* p_Key, int p_Value)
	{
		char* valueStr = reinterpret_cast<char* (__cdecl*)(int)>(0x4C0CE0)(p_Value); // Itoa
		return (WritePrivateProfileStringA(p_Section, p_Key, valueStr, CORE_SETTINGS_FILE_NAME) != 0);
	}

	__forceinline static int GetInteger(const char* p_Section, const char* p_Key, int p_Default = 0)
	{
		return static_cast<int>(GetPrivateProfileIntA(p_Section, p_Key, p_Default, CORE_SETTINGS_FILE_NAME));
	}
};