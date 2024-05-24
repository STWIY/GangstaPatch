#include "../../../Includes.hh"

namespace GameObjectHook::ControllerInput
{
	struct CharacterControllerExtension_t
	{
		ActionMap* m_LastActionMap = nullptr;
		uint8_t m_WalkModeIndex;

		CharacterControllerExtension_t(ActionMap* p_ActionMap)
		{
			p_ActionMap->Pop();

			p_ActionMap->BindEvent("Keyboard0", "Alt", "WalkMode");
			m_WalkModeIndex = p_ActionMap->FindEventIndex(core::MakeKey("WalkMode"));

			p_ActionMap->Push();
		}

		void Update(::ControllerInput* p_ControllerInput, ActionMap* p_ActionMap)
		{
			if (p_ActionMap->GetStateByIndex(m_WalkModeIndex) >= Controller::Pressed) 
			{
				p_ControllerInput->mLeftStickX *= 0.25f;
				p_ControllerInput->mLeftStickY *= 0.25f;
			}
		}
	};

	typedef void(__thiscall* Fn_ReadControllerInput)(::ControllerInput*, ActionMap*);

	void __fastcall ReadControllerInput(::ControllerInput* p_ControllerInput, void* edx, ActionMap* p_ActionMap)
	{
		static core::Key s_CharacterActionMapName = core::MakeKey("CharacterActionMap");
		if (p_ActionMap->m_Key != s_CharacterActionMapName) 
		{
			reinterpret_cast<Fn_ReadControllerInput>(0x57E310)(p_ControllerInput, p_ActionMap);
			return;
		}
		
		static CharacterControllerExtension_t s_CharacterExtension = { p_ActionMap };

		reinterpret_cast<Fn_ReadControllerInput>(0x57E310)(p_ControllerInput, p_ActionMap);

		s_CharacterExtension.Update(p_ControllerInput, p_ActionMap);
	}
}