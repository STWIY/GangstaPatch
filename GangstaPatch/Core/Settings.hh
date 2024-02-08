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
	struct Windowed_t
	{
		enum eMode : uint8_t
		{
			eMode_None = 0,
			eMode_Normal,
			eMode_Borderless
		};
		eMode m_Mode;
	};
	Windowed_t m_Windowed;

	//=======================================================
	// Wrappers

	__forceinline bool SetInteger(const char* p_Section, const char* p_Key, int p_Value)
	{
		char* _ValueStr = reinterpret_cast<char* (__cdecl*)(int)>(0x4C0CE0)(p_Value); // Itoa
		return (WritePrivateProfileStringA(p_Section, p_Key, _ValueStr, CORE_SETTINGS_FILE_NAME) != 0);
	}

	__forceinline int GetInteger(const char* p_Section, const char* p_Key, int p_Default = 0)
	{
		return static_cast<int>(GetPrivateProfileIntA(p_Section, p_Key, p_Default, CORE_SETTINGS_FILE_NAME));
	}

	//=======================================================

	void Initialize()
	{
		this->m_Windowed.m_Mode = static_cast<Windowed_t::eMode>(GetInteger("Windowed", "Mode"));
	}
};
static CoreSettings s_CoreSettings;