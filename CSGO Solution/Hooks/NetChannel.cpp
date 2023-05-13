#include "../Hooks.hpp"
#include "../Features/Networking/Networking.hpp"
#include "../Features/Animations/LagCompensation.hpp"
#include "../Features/Networking/NetChannelRebuild.hpp"

bool __fastcall C_Hooks::hkSendNetMessage( LPVOID pEcx, uint32_t, C_NetMessage& Message, bool bReliable, bool bVoice )
{
	return g_Globals.m_Hooks.m_Originals.m_SendNetMessage( pEcx, Message, bReliable, bVoice || Message.GetType( ) == 9 );
}
