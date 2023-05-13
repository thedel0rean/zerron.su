#include "Misc.hpp"
#include "../../Tools/Tools.hpp"
#include "../Networking/Networking.hpp"

void C_Misc::VelocityModiferFix()
{
	static int m_iLastCmdAck = 0;
	static float m_flNextCmdTime = 0.f;
	if (g_Globals.m_Interfaces.m_EngineClient->IsInGame() && g_Globals.m_LocalPlayer && g_Globals.m_LocalPlayer->IsAlive())
	{
		if (g_Globals.m_Interfaces.m_ClientState && (m_iLastCmdAck != g_Globals.m_Interfaces.m_ClientState->m_nLastCommandAck() || m_flNextCmdTime != g_Globals.m_Interfaces.m_ClientState->m_flNextCmdTime()))
		{
			if (g_Globals.m_AccuracyData.m_flVelocityModifier != g_Globals.m_LocalPlayer->m_flVelocityModifier())
			{
				*(bool*)((uintptr_t)g_Globals.m_Interfaces.m_Prediction + 0x24) = true;
				g_Globals.m_AccuracyData.m_flVelocityModifier = g_Globals.m_LocalPlayer->m_flVelocityModifier();
			}

			m_iLastCmdAck = g_Globals.m_Interfaces.m_ClientState->m_nLastCommandAck();
			m_flNextCmdTime = g_Globals.m_Interfaces.m_ClientState->m_flNextCmdTime();
		}
	}
}