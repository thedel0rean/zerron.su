#include "Movement.hpp"
#include "../Packet/PacketManager.hpp"
#include "../Prediction/EnginePrediction.hpp"
#include "../SDK/Math/Math.hpp"
#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)

void C_Movement::BunnyHop( )
{
	C_UserCmd* pCmd = g_PacketManager->GetModifableCommand( );
	if ( !g_Globals.m_LocalPlayer->IsAlive( ) || g_Globals.m_LocalPlayer->m_nRenderMode( ) == 9 || g_Globals.m_LocalPlayer->m_nRenderMode( ) == 8 )
		 return;

	if ( !g_Settings->m_bBunnyHop )
		return;

	if ( g_Globals.m_LocalPlayer->GetMoveType( ) == MOVETYPE_NOCLIP || 
		 g_Globals.m_LocalPlayer->GetMoveType( ) == MOVETYPE_LADDER )
		 return;

	if ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_ONGROUND )
		return;
	
	pCmd->m_nButtons &= ~IN_JUMP;
}

void C_Movement::AutoStrafe()
{
	C_UserCmd* pCmd = g_PacketManager->GetModifableCommand();
	if (!g_Globals.m_LocalPlayer->IsAlive() || g_Globals.m_LocalPlayer->m_nRenderMode() == 9 || g_Globals.m_LocalPlayer->m_nRenderMode() == 8)
		return;

	if (!g_Settings->m_bAutoStrafe)
		return;

	if (g_Globals.m_LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP ||
		g_Globals.m_LocalPlayer->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (g_Globals.m_LocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	if (g_Globals.m_LocalPlayer->m_vecVelocity().Length2D() <= 5.0f)
	{
		C_BaseCombatWeapon* pCombatWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get();
		if (pCombatWeapon)
		{
			if (pCombatWeapon->m_iItemDefinitionIndex() != WEAPON_SSG08)
			{
				if (!g_Settings->m_bSpeedBoost)
					return;
			}
			else
				return;
		}
	}

	if (!g_Settings->m_bWASDStrafe)
	{
		static auto old_yaw = 0.0f;

		auto get_velocity_degree = [](float velocity)
		{
			auto tmp = RAD2DEG(atan(30.0f / velocity));

			if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
				return 90.0f;

			else if (tmp < 0.0f)
				return 0.0f;
			else
				return tmp;
		};

		auto velocity = g_Globals.m_LocalPlayer->m_vecVelocity();
		velocity.z = 0;

		static auto flip = false;
		flip = !flip;

		auto turn_direction_modifier = flip ? 1.0f : -1.0f;
		auto viewangles = pCmd->m_angViewAngles;

		auto strafe_angle = RAD2DEG(atan(15.0f / velocity.Length2D()));

		if (strafe_angle > 90.0f)
			strafe_angle = 90.0f;
		else if (strafe_angle < 0.0f)
			strafe_angle = 0.0f;

		auto temp = Vector(0.0f, viewangles.yaw - old_yaw, 0.0f);
		temp.y = Math::NormalizeAngle(temp.y);

		auto yaw_delta = temp.y;
		old_yaw = viewangles.yaw;

		auto abs_yaw_delta = fabs(yaw_delta);
		if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 30.0f)
		{
			Vector velocity_angles;
			Math::VectorAnglesView(velocity, velocity_angles);

			temp = Vector(0.0f, viewangles.yaw - velocity_angles.y, 0.0f);
			temp.y = Math::NormalizeAngle(temp.y);

			auto velocityangle_yawdelta = temp.y;
			auto velocity_degree = get_velocity_degree(velocity.Length2D());

			if (velocityangle_yawdelta <= velocity_degree || velocity.Length2D() <= 15.0f)
			{
				if (-velocity_degree <= velocityangle_yawdelta || velocity.Length2D() <= 15.0f)
				{
					viewangles.yaw += strafe_angle * turn_direction_modifier;
					pCmd->m_flSideMove = g_Globals.m_ConVars.m_clSideSpeed->GetFloat() * turn_direction_modifier;
				}
				else
				{
					viewangles.yaw = velocity_angles.y - velocity_degree;
					pCmd->m_flSideMove = g_Globals.m_ConVars.m_clSideSpeed->GetFloat();
				}
			}
			else
			{
				viewangles.yaw = velocity_angles.y + velocity_degree;
				pCmd->m_flSideMove = -g_Globals.m_ConVars.m_clSideSpeed->GetFloat();
			}
		}
		else if (yaw_delta > 0.0f)
			pCmd->m_flSideMove = -g_Globals.m_ConVars.m_clSideSpeed->GetFloat();
		else if (yaw_delta < 0.0f)
			pCmd->m_flSideMove = g_Globals.m_ConVars.m_clSideSpeed->GetFloat();

		auto move = Vector(pCmd->m_flForwardMove, pCmd->m_flSideMove, 0.0f);
		auto speed = move.Length();

		Vector angles_move;
		Math::VectorAnglesView(move, angles_move);

		auto normalized_x = fmod(pCmd->m_angViewAngles.pitch + 180.0f, 360.0f) - 180.0f;
		auto normalized_y = fmod(pCmd->m_angViewAngles.yaw + 180.0f, 360.0f) - 180.0f;

		auto yaw = DEG2RAD(normalized_y - viewangles.yaw + angles_move.y);

		if (normalized_x >= 90.0f || normalized_x <= -90.0f || pCmd->m_angViewAngles.pitch >= 90.0f && pCmd->m_angViewAngles.pitch <= 200.0f || pCmd->m_angViewAngles.pitch <= -90.0f && pCmd->m_angViewAngles.pitch <= 200.0f) //-V648
			pCmd->m_flForwardMove = -cos(yaw) * speed;
		else
			pCmd->m_flForwardMove = cos(yaw) * speed;

		pCmd->m_flSideMove = sin(yaw) * speed;
	}
	else if (g_Settings->m_bWASDStrafe)
	{
		static auto old_yaw = 0.0f;

		auto get_velocity_degree = [](float velocity)
		{
			auto tmp = RAD2DEG(atan(30.0f / velocity));

			if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
				return 90.0f;

			else if (tmp < 0.0f)
				return 0.0f;
			else
				return tmp;
		};

		auto velocity = g_Globals.m_LocalPlayer->m_vecVelocity();
		velocity.z = 0.0f;

		auto forwardmove = pCmd->m_flForwardMove;
		auto sidemove = pCmd->m_flSideMove;

		if (velocity.Length2D() < 5.0f && !forwardmove && !sidemove)
			return;

		static auto flip = false;
		flip = !flip;

		auto turn_direction_modifier = flip ? 1.0f : -1.0f;
		auto viewangles = pCmd->m_angViewAngles;

		if (forwardmove || sidemove)
		{
			pCmd->m_flForwardMove = 0.0f;
			pCmd->m_flSideMove = 0.0f;

			auto turn_angle = atan2(-sidemove, forwardmove);
			viewangles.yaw += turn_angle * M_RADPI;
		}
		else if (forwardmove) //-V550
			pCmd->m_flForwardMove = 0.0f;

		auto strafe_angle = RAD2DEG(atan(15.0f / velocity.Length2D()));

		if (strafe_angle > 90.0f)
			strafe_angle = 90.0f;
		else if (strafe_angle < 0.0f)
			strafe_angle = 0.0f;

		auto temp = Vector(0.0f, viewangles.yaw - old_yaw, 0.0f);
		temp.y = Math::NormalizeAngle(temp.y);

		auto yaw_delta = temp.y;
		old_yaw = viewangles.yaw;

		auto abs_yaw_delta = fabs(yaw_delta);

		if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 30.0f)
		{
			Vector velocity_angles;
			Math::VectorAnglesView(velocity, velocity_angles);

			temp = Vector(0.0f, viewangles.yaw - velocity_angles.y, 0.0f);
			temp.y = Math::NormalizeAngle(temp.y);

			auto velocityangle_yawdelta = temp.y;
			auto velocity_degree = get_velocity_degree(velocity.Length2D());

			if (velocityangle_yawdelta <= velocity_degree || velocity.Length2D() <= 15.0f)
			{
				if (-velocity_degree <= velocityangle_yawdelta || velocity.Length2D() <= 15.0f)
				{
					viewangles.yaw += strafe_angle * turn_direction_modifier;
					pCmd->m_flSideMove = g_Globals.m_ConVars.m_clSideSpeed->GetFloat() * turn_direction_modifier;
				}
				else
				{
					viewangles.yaw = velocity_angles.y - velocity_degree;
					pCmd->m_flSideMove = g_Globals.m_ConVars.m_clSideSpeed->GetFloat();
				}
			}
			else
			{
				viewangles.yaw = velocity_angles.y + velocity_degree;
				pCmd->m_flSideMove = -g_Globals.m_ConVars.m_clSideSpeed->GetFloat();
			}
		}
		else if (yaw_delta > 0.0f)
			pCmd->m_flSideMove = -g_Globals.m_ConVars.m_clSideSpeed->GetFloat();
		else if (yaw_delta < 0.0f)
			pCmd->m_flSideMove = g_Globals.m_ConVars.m_clSideSpeed->GetFloat();

		auto move = Vector(pCmd->m_flForwardMove, pCmd->m_flSideMove, 0.0f);
		auto speed = move.Length();

		Vector angles_move;
		Math::VectorAnglesView(move, angles_move);

		auto normalized_x = fmod(pCmd->m_angViewAngles.pitch + 180.0f, 360.0f) - 180.0f;
		auto normalized_y = fmod(pCmd->m_angViewAngles.yaw + 180.0f, 360.0f) - 180.0f;

		auto yaw = DEG2RAD(normalized_y - viewangles.yaw + angles_move.y);

		if (normalized_x >= 90.0f || normalized_x <= -90.0f || pCmd->m_angViewAngles.pitch >= 90.0f && pCmd->m_angViewAngles.pitch <= 200.0f || pCmd->m_angViewAngles.pitch <= -90.0f && pCmd->m_angViewAngles.pitch <= 200.0f) //-V648
			pCmd->m_flForwardMove = -cos(yaw) * speed;
		else
			pCmd->m_flForwardMove = cos(yaw) * speed;

		pCmd->m_flSideMove = sin(yaw) * speed;
	}
}

void C_Movement::MouseCorrection( )
{
	QAngle angOldViewAngles;
	g_Globals.m_Interfaces.m_EngineClient->GetViewAngles( &angOldViewAngles );

	float delta_x = std::remainderf( g_PacketManager->GetModifableCommand( )->m_angViewAngles.pitch - angOldViewAngles.pitch, 360.0f );
	float delta_y = std::remainderf( g_PacketManager->GetModifableCommand( )->m_angViewAngles.yaw - angOldViewAngles.yaw, 360.0f );

	if ( delta_x != 0.0f ) 
	{
		float mouse_y = -( ( delta_x / g_Globals.m_ConVars.m_Pitch->GetFloat( ) ) / g_Globals.m_ConVars.m_Sensitivity->GetFloat( ) );
		short mousedy;
		if ( mouse_y <= 32767.0f ) {
			if ( mouse_y >= -32768.0f ) {
				if ( mouse_y >= 1.0f || mouse_y < 0.0f ) {
					if ( mouse_y <= -1.0f || mouse_y > 0.0f )
						mousedy = static_cast<short>( mouse_y );
					else
						mousedy = -1;
				}
				else {
					mousedy = 1;
				}
			}
			else {
				mousedy = 0x8000u;
			}
		}
		else {
			mousedy = 0x7FFF;
		}

		g_PacketManager->GetModifableCommand( )->m_nMouseDirectionY = mousedy;
	}

	if ( delta_y != 0.0f )
	{
		float mouse_x = -( ( delta_y / g_Globals.m_ConVars.m_Yaw->GetFloat( ) ) / g_Globals.m_ConVars.m_Sensitivity->GetFloat( ) );
		short mousedx;
		if ( mouse_x <= 32767.0f ) {
			if ( mouse_x >= -32768.0f ) {
				if ( mouse_x >= 1.0f || mouse_x < 0.0f ) {
					if ( mouse_x <= -1.0f || mouse_x > 0.0f )
						mousedx = static_cast<short>( mouse_x );
					else
						mousedx = -1;
				}
				else {
					mousedx = 1;
				}
			}
			else {
				mousedx = 0x8000u;
			}
		}
		else {
			mousedx = 0x7FFF;
		}

		g_PacketManager->GetModifableCommand( )->m_nMouseDirectionX = mousedx;
	}
}

void C_Movement::FastStop( )
{
	if ( !g_Globals.m_LocalPlayer->IsAlive( ) || g_Globals.m_LocalPlayer->m_nRenderMode( ) == 9 || g_Globals.m_LocalPlayer->m_nRenderMode( ) == 8 )
		 return;

	if ( !g_Settings->m_bFastStop )
		return;

	if ( g_Globals.m_LocalPlayer->GetMoveType( ) == MOVETYPE_NOCLIP || 
		 g_Globals.m_LocalPlayer->GetMoveType( ) == MOVETYPE_LADDER )
		 return;

	if ( g_PacketManager->GetModifableCommand( )->m_nButtons & ( IN_JUMP | IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK ) )
		return;

	if ( !( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_ONGROUND ) )
		return;

	if ( g_Globals.m_LocalPlayer->m_vecVelocity( ).Length2D( ) <= g_Globals.m_LocalPlayer->GetMaxPlayerSpeed( ) * 0.34f )
	{
		g_PacketManager->GetModifableCommand( )->m_flForwardMove = g_PacketManager->GetModifableCommand( )->m_flSideMove = 0.0f;
		return;
	}

	QAngle angResistance = QAngle( 0, 0, 0 );
    Math::VectorAngles( ( g_Globals.m_LocalPlayer->m_vecVelocity( ) * -1.f ), angResistance );

	angResistance.yaw = g_PacketManager->GetModifableCommand( )->m_angViewAngles.yaw - angResistance.yaw;
	angResistance.pitch = g_PacketManager->GetModifableCommand( )->m_angViewAngles.pitch - angResistance.pitch;

    Vector vecResistance = Vector( 0, 0, 0 );
    Math::AngleVectors( angResistance, vecResistance );

    g_PacketManager->GetModifableCommand( )->m_flForwardMove = std::clamp( vecResistance.x, -450.f, 450.0f );
	g_PacketManager->GetModifableCommand( )->m_flSideMove = std::clamp( vecResistance.y, -450.f, 450.0f );
}

void C_Movement::EdgeJump( )
{
	if ( !g_Globals.m_LocalPlayer->IsAlive( ) || g_Globals.m_LocalPlayer->m_nRenderMode( ) == 9 || g_Globals.m_LocalPlayer->m_nRenderMode( ) == 8 )
		 return;

	if ( !g_Settings->m_bEdgeJump )
		return;

	if ( g_Globals.m_LocalPlayer->GetMoveType( ) == MOVETYPE_NOCLIP || 
		 g_Globals.m_LocalPlayer->GetMoveType( ) == MOVETYPE_LADDER )
		 return;

	if ( !( g_PredictionSystem->GetNetvars( g_PacketManager->GetModifableCommand( )->m_nCommand ).m_fFlags & FL_ONGROUND ) )
		return;

	if ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_ONGROUND )
		return;

	g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_JUMP;
	
	g_PredictionSystem->RestoreNetvars( g_PacketManager->GetModifableCommand( )->m_nCommand );
	return g_PredictionSystem->Repredict( );
}