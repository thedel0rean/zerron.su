#include "Animations.hpp"
#include "BoneManager.hpp"
#include "../SDK/Math/Math.hpp"
#include "../RageBot/Autowall.hpp"
#include "../Tools/Tools.hpp"

float MaxDesyncDelta(C_BasePlayer* player)
{
	auto animstate = uintptr_t(player->m_PlayerAnimStateCSGO());
	float duckammount = *(float*)(animstate + 0xA4);
	float speedfraction = max(0, min(*reinterpret_cast<float*>(animstate + 0xF8), 1));
	float speedfactor = max(0, min(1, *reinterpret_cast<float*> (animstate + 0xFC)));
	float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
	float unk2 = unk1 + 1.f;
	float unk3;
	if (duckammount > 0) {
		unk2 += ((duckammount * speedfactor) * (0.5f - unk2));
	}
	unk3 = *(float*)(animstate + 0x334) * unk2;
	return unk3;
}


int C_AnimationSync::GetDesyncSideByPlaybackrate(C_BasePlayer* pPlayer, C_LagRecord& LatestRecord, C_LagRecord PreviousRecord, bool bHasPreviousRecord)
{
	float_t flLeftDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_LEFT).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate);
	float_t flRightDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_RIGHT).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate);
	float_t flCenterDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_CENTER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate);

	//onetap
	{
		if (flLeftDelta >= flRightDelta)
			return ROTATE_RIGHT;
		else if (flRightDelta >= flLeftDelta)
			return ROTATE_LEFT;
		//else
		//	return ROTATE_CENTER;
	}

	//pPlayer->m_PlayerAnimStateCSGO()->m_flPrimaryCycle = ;
	//pPlayer->GetFirstSequenceAnimationTag();
}

void C_AnimationSync::RebuildSixLayer(C_BasePlayer* player, C_LagRecord& LatestRecord, int rotation, float delta)
{
	auto m_pState = player->m_PlayerAnimStateCSGO();
	static const float CS_PLAYER_SPEED_RUN = 260.0f;

	// TODO: Find these members in the actual animstate struct
	auto m_flLastUpdateIncrement = *(float*)((DWORD)m_pState + 0x74);
	auto m_flFootYaw = m_pState->m_flFootYaw;
	auto m_flMoveYaw = m_pState->m_flMoveYaw;
	auto m_vecVelocityNormalizedNonZero = *(Vector*)((DWORD)m_pState + 0xE0);
	auto m_flInAirSmoothValue = *(float*)((DWORD)m_pState + 0x124);

	char m_szDestination[64];
	sprintf_s(m_szDestination, _S("move_%s"), m_pState->GetWeaponPrefix());

	int m_nMoveSequence = player->LookupSequence(m_szDestination);
	if (m_nMoveSequence == -1)
	{
		m_nMoveSequence = player->LookupSequence(_S("move"));
	}

	// NOTE:
	// player->get<int>( 0x3984 ) is m_iMoveState
	if (player->m_iMoveState() != g_Globals.m_RebuildData[rotation].m_iMoveState)
		g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate += 10.0f;

	g_Globals.m_RebuildData[rotation].m_iMoveState = player->m_iMoveState();

	float m_flMovementTimeDelta = *(float*)((DWORD)m_pState + 0x74) * 40.0f;

	if (-g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate <= m_flMovementTimeDelta)
	{
		if (-m_flMovementTimeDelta <= -g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate)
			g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate = 0.0f;
		else
			g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate = g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate - m_flMovementTimeDelta;
	}
	else
	{
		g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate = g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate + m_flMovementTimeDelta;
	}

	g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate = std::clamp(g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate, 0.0f, 100.0f);

	float m_flDuckSpeedClamped = std::clamp(*(float*)((DWORD)m_pState + 0xFC), 0.0f, 1.0f);
	float m_flRunSpeedClamped = std::clamp(*(float*)((DWORD)m_pState + 0xF8), 0.0f, 1.0f);

	float m_flSpeedWeight = ((m_flDuckSpeedClamped - m_flRunSpeedClamped) * *(float*)((DWORD)m_pState + 0xA4)) + m_flRunSpeedClamped;

	if (m_flSpeedWeight < g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight)
	{
		float v34 = std::clamp(g_Globals.m_RebuildData[rotation].m_flMovePlaybackRate * 0.01f, 0.0f, 1.0f);
		float m_flFeetWeightElapsed = ((v34 * 18.0f) + 2.0f) * *(float*)((DWORD)m_pState + 0x74);
		if (m_flSpeedWeight - g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight <= m_flFeetWeightElapsed)
			g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight = -m_flFeetWeightElapsed <= (m_flSpeedWeight - g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight) ? m_flSpeedWeight : g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight - m_flFeetWeightElapsed;
		else
			g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight = m_flFeetWeightElapsed + g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight;
	}
	else
	{
		g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight = m_flSpeedWeight;
	}

	float modifer = rotation != ROTATE_CENTER ? rotation == ROTATE_RIGHT || rotation == ROTATE_LOW_RIGHT ? delta : -delta : 0.f;
	float footyaw = Math::NormalizeAngle(m_pState->m_flMoveYaw + m_pState->m_flFootYaw + modifer);
	float m_flYaw = Math::AngleNormalize(footyaw + 180.0f);
	QAngle m_angAngle = { 0.0f, m_flYaw, 0.0f };
	Vector m_vecDirection;
	Math::AngleVectors(m_angAngle, m_vecDirection);

	float m_flMovementSide = Math::DotProduct(m_vecVelocityNormalizedNonZero, m_vecDirection);
	if (m_flMovementSide < 0.0f)
		m_flMovementSide = -m_flMovementSide;

	float m_flNewFeetWeight = Math::Bias(m_flMovementSide, 0.2f) * g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight;

	float m_flNewFeetWeightWithAirSmooth = m_flNewFeetWeight * m_flInAirSmoothValue;

	// m_flLayer5Weight looks a bit weird so i decided to name it m_flLayer5_Weight instead.
	float m_flLayer5_Weight = LatestRecord.m_AnimationLayers.at(rotation).at(5).m_flWeight;

	float m_flNewWeight = 0.55f;
	if (1.0f - m_flLayer5_Weight > 0.55f)
		m_flNewWeight = 1.0f - m_flLayer5_Weight;

	float m_flNewFeetWeightLayerWeight = m_flNewWeight * m_flNewFeetWeightWithAirSmooth;
	float m_flFeetCycleRate = 0.0f;

	float m_flSpeed = std::fmin(player->m_vecVelocity().Length(), 260.f);
	if (m_flSpeed > 0.00f)
	{
		std::array < float_t, MAXSTUDIOPOSEPARAM > PoseParameters;
		std::memcpy(PoseParameters.data(), player->m_aPoseParameters().data(), sizeof(float_t) * MAXSTUDIOPOSEPARAM);

		std::memcpy(player->m_aPoseParameters().data(), LatestRecord.m_PoseParameters.at(rotation).data(), sizeof(float_t) * MAXSTUDIOPOSEPARAM);
		float m_flSequenceCycleRate = player->GetSequenceCycleRate(player->GetStudioHdr(), m_nMoveSequence);
		float m_flSequenceMoveDist = player->GetSequenceMoveDist(player->GetStudioHdr(), m_nMoveSequence, player->m_aPoseParameters().data());
		std::memcpy(player->m_aPoseParameters().data(), PoseParameters.data(), sizeof(float_t) * MAXSTUDIOPOSEPARAM);


		m_flSequenceMoveDist *= 1.0f / (1.0f / m_flSequenceCycleRate);
		if (m_flSequenceMoveDist <= 0.001f)
			m_flSequenceMoveDist = 0.001f;

		float m_flSpeedMultiplier = m_flSpeed / m_flSequenceMoveDist;
		m_flFeetCycleRate = (1.0f - (m_pState->m_flWalkToRunTransition * 0.15f)) * (m_flSpeedMultiplier * m_flSequenceCycleRate);
	}

	float m_flFeetCyclePlaybackRate = (*(float*)((DWORD)m_pState + 0x74) * m_flFeetCycleRate);
	g_Globals.m_RebuildData[rotation].m_flPrimaryCycle = m_flFeetCyclePlaybackRate + g_Globals.m_RebuildData[rotation].m_flPrimaryCycle;

	// store possible information for resolving.
	g_Globals.m_RebuildData[rotation].m_ReadyData.m_flMovementSide = m_flMovementSide;
	g_Globals.m_RebuildData[rotation].m_ReadyData.m_angMoveYaw = m_angAngle;
	g_Globals.m_RebuildData[rotation].m_ReadyData.m_vecDirection = m_vecDirection;
	g_Globals.m_RebuildData[rotation].m_ReadyData.m_flFeetWeight = m_flNewFeetWeight;

	g_Globals.m_RebuildData[rotation].m_ReadyData.m_nSequence = m_nMoveSequence;
	g_Globals.m_RebuildData[rotation].m_ReadyData.m_flPlaybackRate = m_flFeetCyclePlaybackRate;
	g_Globals.m_RebuildData[rotation].m_ReadyData.m_flCycle = g_Globals.m_RebuildData[rotation].m_flPrimaryCycle;
	g_Globals.m_RebuildData[rotation].m_ReadyData.m_flWeight = std::clamp(m_flNewFeetWeightLayerWeight, 0.0f, 1.0f);
}

int C_AnimationSync::GetDesyncSideByFreestand(C_BasePlayer* player, C_LagRecord& LatestRecord, C_LagRecord PreviousRecord, bool bHasPreviousRecord)
{
	static float lock_side = 0.f;
	static int FreestandRotation = ROTATE_CENTER;
	Vector src3D, dst3D, forward, right, up, src, dst;
	float back_two, right_two, left_two;
	CGameTrace tr;
	CTraceFilter filter;

	QAngle backward_yaw = QAngle(0, Math::CalcAngle(g_Globals.m_LocalPlayer->m_vecOrigin(), player->m_vecOrigin()).yaw, 0);

	Math::AngleVectors(backward_yaw, forward, right, up);

	filter.pSkip = player;
	src3D = player->GetShootPosition();
	dst3D = src3D + (forward * 384); //Might want to experiment with other numbers, incase you don't know what the number does, its how far the trace will go. Lower = shorter.

	Ray_t ray1;
	ray1.Init(src3D, dst3D);

	Ray_t ray2;
	ray2.Init(src3D + right * 35, dst3D + g_Globals.m_ResolverData.m_ResolverDelta[player->EntIndex()] * 35);

	Ray_t ray3;
	ray3.Init(src3D - right * 35, dst3D - g_Globals.m_ResolverData.m_ResolverDelta[player->EntIndex()] * 35);

	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray1, MASK_SHOT, &filter, &tr);
	back_two = (tr.endpos - tr.startpos).Length();

	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray2, MASK_SHOT, &filter, &tr);
	right_two = (tr.endpos - tr.startpos).Length();

	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray3, MASK_SHOT, &filter, &tr);
	left_two = (tr.endpos - tr.startpos).Length();

	if (left_two > right_two) {
		return ROTATE_RIGHT;
	}
	else if (right_two > left_two) {
		return ROTATE_LEFT;
	}
	else
		return ROTATE_CENTER;
}
bool IsResolveSidePositive(int side)
{
	return side == ROTATE_RIGHT || side == ROTATE_LOW_RIGHT;
}

void C_AnimationSync::Instance( ClientFrameStage_t Stage )
{
	if ( Stage != ClientFrameStage_t::FRAME_NET_UPDATE_END )
		return;

	for ( int32_t iPlayerID = 1; iPlayerID <= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients; iPlayerID++ )
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex( iPlayerID );
		if ( !pPlayer || !pPlayer->IsPlayer( ) || !pPlayer->IsAlive( ) || pPlayer->m_iTeamNum( ) == g_Globals.m_LocalPlayer->m_iTeamNum( ) )
		{
			g_Globals.m_ResolverData.m_AnimResoled[ iPlayerID ] = false;
			g_Globals.m_ResolverData.m_MissedShots[ iPlayerID ] = 0;
			g_Globals.m_ResolverData.m_LastMissedShots[ iPlayerID ] = 0;
			g_Globals.m_ResolverData.m_MissedShots[iPlayerID] = 0;
			continue;
		}

		bool bHasPreviousRecord = false;
		if ( pPlayer->m_flOldSimulationTime( ) >= pPlayer->m_flSimulationTime( ) )
		{
			if ( pPlayer->m_flOldSimulationTime( ) > pPlayer->m_flSimulationTime( ) )
				this->UnmarkAsDormant( iPlayerID );

			continue;
		}

		auto& LagRecords = g_Globals.m_CachedPlayerRecords[ iPlayerID ];
		if ( LagRecords.empty( ) )
			continue;

		C_LagRecord PreviousRecord = m_PreviousRecord[ iPlayerID ];
		if ( TIME_TO_TICKS( fabs( pPlayer->m_flSimulationTime( ) - PreviousRecord.m_SimulationTime ) ) <= 17 )
			bHasPreviousRecord = true;

		C_LagRecord& LatestRecord = LagRecords.back( );
		if ( this->HasLeftOutOfDormancy( iPlayerID ) )
			bHasPreviousRecord = false;

		if ( LatestRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_flCycle == PreviousRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_flCycle )
		{
			pPlayer->m_flSimulationTime( ) = pPlayer->m_flOldSimulationTime( );
			continue;
		}

		LatestRecord.m_UpdateDelay = TIME_TO_TICKS( pPlayer->m_flSimulationTime( ) - this->GetPreviousRecord( iPlayerID ).m_SimulationTime );
		if ( LatestRecord.m_UpdateDelay > 17 )
			LatestRecord.m_UpdateDelay = 1;

		C_PlayerInfo PlayerInfo;
		g_Globals.m_Interfaces.m_EngineClient->GetPlayerInfo( iPlayerID, &PlayerInfo );

		if ( /*PlayerInfo.m_bIsFakePlayer ||*/ LatestRecord.m_UpdateDelay < 1 )
			LatestRecord.m_UpdateDelay = 1;
		
		Vector vecVelocity = LatestRecord.m_Velocity;
		if ( bHasPreviousRecord )
		{
			float_t flCurrentCycle = LatestRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_flCycle;
			float_t flPreviousCycle = this->GetPreviousRecord( pPlayer->EntIndex( ) ).m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_flCycle;
			if ( flCurrentCycle != flPreviousCycle )
			{
				float_t flPreviousRate = this->GetPreviousRecord( pPlayer->EntIndex( ) ).m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_flPlaybackRate;
				float_t flCurrentRate = LatestRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_flPlaybackRate;

				if ( flPreviousRate == flCurrentRate )
				{
					if ( flPreviousCycle > flCurrentCycle )
						flCurrentCycle += 1.0f;

					int32_t iCycleTiming = TIME_TO_TICKS( flCurrentCycle - flPreviousCycle );
					if ( iCycleTiming <= 17 )
						if ( iCycleTiming > LatestRecord.m_UpdateDelay )
							LatestRecord.m_UpdateDelay = iCycleTiming;
				}
			}

			if ( LatestRecord.m_UpdateDelay > 17 )
				LatestRecord.m_UpdateDelay = 1;

			if ( PlayerInfo.m_bIsFakePlayer || LatestRecord.m_UpdateDelay < 1 )
				LatestRecord.m_UpdateDelay = 1;

			if ( !( LatestRecord.m_Flags & FL_ONGROUND ) )
			{
				vecVelocity = ( LatestRecord.m_Origin - this->GetPreviousRecord( pPlayer->EntIndex( ) ).m_Origin ) / LatestRecord.m_UpdateDelay;

				float_t flWeight = 1.0f - LatestRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_flWeight;
				if ( flWeight > 0.0f )
				{
					float_t flPreviousRate = this->GetPreviousRecord( pPlayer->EntIndex( ) ).m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_flPlaybackRate;
					float_t flCurrentRate = LatestRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_flPlaybackRate;

					if ( flPreviousRate == flCurrentRate )
					{
						int32_t iPreviousSequence = this->GetPreviousRecord( pPlayer->EntIndex( ) ).m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_nSequence;
						int32_t iCurrentSequence = LatestRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_ALIVELOOP ).m_nSequence;

						if ( iPreviousSequence == iCurrentSequence )
						{
							float_t flSpeedNormalized = ( flWeight / 2.8571432f ) + 0.55f;
							if ( flSpeedNormalized > 0.0f )
							{
								float_t flSpeed = flSpeedNormalized * pPlayer->GetMaxPlayerSpeed( );
								if ( flSpeed > 0.0f )
								{
									if ( vecVelocity.Length2D( ) > 0.0f )
									{
										vecVelocity.x /= vecVelocity.Length2D( ) / flSpeed;
										vecVelocity.y /= vecVelocity.Length2D( ) / flSpeed;
									}
								}
							}
						}
					} 
				}
				
				vecVelocity.z -= g_Globals.m_ConVars.m_SvGravity->GetFloat( ) * 0.5f * TICKS_TO_TIME( LatestRecord.m_UpdateDelay );
			}
			else
				vecVelocity.z = 0.0f;
		}
			
		LatestRecord.m_Velocity = vecVelocity;
			
		std::array < C_AnimationLayer, ANIMATION_LAYER_COUNT > AnimationLayers;
		std::array < float_t, MAXSTUDIOPOSEPARAM > PoseParameters;
		C_CSGOPlayerAnimationState AnimationState;

		// áýêàïèì äàííûå
		std::memcpy( AnimationLayers.data( ), pPlayer->m_AnimationLayers( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
		std::memcpy( PoseParameters.data( ), pPlayer->m_aPoseParameters( ).data( ), sizeof( float_t ) * MAXSTUDIOPOSEPARAM );
		std::memcpy( &AnimationState, pPlayer->m_PlayerAnimStateCSGO( ), sizeof( AnimationState ) );

		// ðîòåéòèì èãðîêà äëÿ ñýéôïîèíòîâ
		for ( int32_t i = ROTATE_LEFT; i <= ROTATE_LOW_RIGHT; i++ )
		{
			// ðîòåéòèì èãðîêà òâîþ ìàòü 
			this->UpdatePlayerAnimations( pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, i );

			// ñîõðàíÿåì íåêîòîðûå äàííûå
			std::memcpy( LatestRecord.m_AnimationLayers.at( i ).data( ), pPlayer->m_AnimationLayers( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );

			// ñåòàïèì ëååðû ñåðâåðà
			std::memcpy( pPlayer->m_AnimationLayers( ), AnimationLayers.data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );

			// ñåòàïèì êîñòè
			if ( i < ROTATE_LOW_LEFT )
				g_BoneManager->BuildMatrix( pPlayer, LatestRecord.m_Matricies[ i ].data( ), true );

			std::memcpy(LatestRecord.m_PoseParameters.at(i).data(), pPlayer->m_aPoseParameters().data(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);

			// ðåñòîðèì äåôîëòíûå äàííûå
			std::memcpy( pPlayer->m_aPoseParameters( ).data( ), PoseParameters.data( ), sizeof( float_t ) * MAXSTUDIOPOSEPARAM );
			std::memcpy( pPlayer->m_PlayerAnimStateCSGO( ), &AnimationState, sizeof( AnimationState ) );
		}

		//delta calculate
		{
			static float m_flLastLbyUpdate = 0.f;

			if (LatestRecord.m_Velocity.Length2D() > 1.0f)
			{
				g_Globals.m_ResolverData.m_ResolverDelta[iPlayerID] = std::clamp(fabs(Math::AngleDiff(pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw, LatestRecord.m_LowerBodyYaw)), 0.f, 58.f);

				m_flLastLbyUpdate = g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime + 0.22f;
			}
			else
			{
				if (g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime > m_flLastLbyUpdate)
				{
					g_Globals.m_ResolverData.m_ResolverDelta[iPlayerID] = std::clamp(fabs(Math::AngleDiff(pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw, LatestRecord.m_LowerBodyYaw)), 0.f, 58.f);

					m_flLastLbyUpdate = g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime + 1.1f;
				}
			}
			if (g_Globals.m_ResolverData.m_ResolverDelta[iPlayerID] < 7.f)
				g_Globals.m_ResolverData.m_ResolverDelta[iPlayerID] = 60.f;
		}

		bool HaveLowDelta = g_Globals.m_ResolverData.m_ResolverDelta[iPlayerID] <= 25.f;

		if ( !LatestRecord.m_bIsShooting && g_Globals.m_LocalPlayer->IsAlive() && g_Globals.m_LocalPlayer->m_hActiveWeapon())
		{
			if ( LatestRecord.m_UpdateDelay > 1 && bHasPreviousRecord )
			{
				int m_iFreestandSide = GetDesyncSideByFreestand(pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord);
				int m_iPlaybackrateSide = GetDesyncSideByPlaybackrate(pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord);
				bool found = false;
				if (LatestRecord.m_Velocity.Length2D() > 1.0f)
				{
					const auto float_matches = [](const float float1, const float float2, float tolerance = 0.0002f) { return fabsf(float1 - float2) < tolerance; };
					const auto return_delta = [](const float float1, const float float2) { return fabsf(float1 - float2); };

					for (int i = 0; i < ROTATE_RIGHT; ++i)
					{
						float delta = MaxDesyncDelta(pPlayer);
						RebuildSixLayer(pPlayer, LatestRecord, i, delta);

						float NearestDelta = FLT_MAX;
						if (return_delta(g_Globals.m_RebuildData[i].m_ReadyData.m_flPlaybackRate, LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate) < NearestDelta)
						{
							NearestDelta = return_delta(g_Globals.m_RebuildData[i].m_ReadyData.m_flPlaybackRate, LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate);
							LatestRecord.m_RotationMode = i;
							LatestRecord.m_bAnimResolved = true;
							g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 30;
							found = true;
						}
					}
				}
				else
				{
					float_t flFeetDelta = Math::AngleNormalize(Math::AngleDiff(Math::AngleNormalize(pPlayer->m_flLowerBodyYaw()), Math::AngleNormalize(pPlayer->m_angEyeAngles().yaw)));
					
					const auto float_matches = [](const float float1, const float float2, float tolerance = 0.002f) { return fabsf(float1 - float2) < tolerance; };
					
					auto Layer3 = LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST);
					auto LastLayer3 = PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST);
					auto _currentRecord = LatestRecord;
					auto _previousRecord = PreviousRecord;
					int iPlayerID = pPlayer->EntIndex();

					if (float_matches(Layer3.m_flWeightDeltaRate, 10.04290f)
						&& float_matches(LastLayer3.m_flWeightDeltaRate, 2.6813f)
						&& fabsf(
							Math::AngleNormalize(_currentRecord.m_LowerBodyYaw - _currentRecord.m_EyeAngles.yaw)
						) > 55.f)
					{
						g_Globals.m_ResolverData.m_ResolverDelta[iPlayerID] = 90.f;
						g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 2;
						found = true;
					}
					if (float_matches(Layer3.m_flWeightDeltaRate, 2.6813f)
						&& float_matches(LastLayer3.m_flWeightDeltaRate, 2.6813f)
						&& fabsf(
							Math::AngleNormalize(_currentRecord.m_LowerBodyYaw - _currentRecord.m_EyeAngles.yaw)
							-
							Math::AngleNormalize(_previousRecord.m_LowerBodyYaw - _previousRecord.m_EyeAngles.yaw)
						) < 50.f)
					{
						g_Globals.m_ResolverData.m_ResolverDelta[iPlayerID] = 90.f;
						g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 3;
						found = true;
						LatestRecord.m_RotationMode = ROTATE_LEFT;
					}


					if (!found)
					{
						if (Layer3.m_flWeightDeltaRate == 0.f && LastLayer3.m_flWeightDeltaRate == 0.f
							&& Layer3.m_flWeight == 1.f && LastLayer3.m_flWeight == 1.f
							&& pPlayer->GetSequenceActivity(Layer3.m_nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST
							&& pPlayer->GetSequenceActivity(LastLayer3.m_nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST
							)
						{
							float dt = Math::AngleNormalize(Math::AngleDiff(pPlayer->m_flLowerBodyYaw(), pPlayer->m_angEyeAngles().yaw));
							if (fabsf(dt) > 35)
							{
								g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 4;

								g_Globals.m_ResolverData.m_ResolverDelta[iPlayerID] = 90.f;
								found = true;
								LatestRecord.m_RotationMode = ROTATE_LEFT;
							}
						}
					}

					if (!found)
					{
						if ((float_matches(Layer3.m_flWeightDeltaRate, 2.6813f) && float_matches(LastLayer3.m_flWeightDeltaRate, 2.6813f))
							|| (float_matches(Layer3.m_flWeightDeltaRate, 10.2735481f, 0.05f) && float_matches(LastLayer3.m_flWeightDeltaRate, 10.2735481f, 0.05f))
							|| ((Layer3.m_flWeight > 7.f && LastLayer3.m_flWeight < 8.f)
								&&
								(float_matches(Layer3.m_flWeightDeltaRate, 7.46731186f, 0.9f) && float_matches(LastLayer3.m_flWeightDeltaRate, 7.46731186f, 0.9f)))
							)
						{
							float diff = Math::AngleDiff(_currentRecord.m_LowerBodyYaw, _currentRecord.m_EyeAngles.yaw);
							if (diff < -35.f)
								found = true;
							g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 5;

							LatestRecord.m_RotationMode = ROTATE_LEFT;
						}
					}

					if (!found)
					{
						//choking 2 ticks with 180 yaw and no jitter:   weight 0.152941 weightdeltarate 7.08289
						if (float_matches(Layer3.m_flWeight, LastLayer3.m_flWeight)
							&& float_matches(Layer3.m_flWeightDeltaRate, LastLayer3.m_flWeightDeltaRate)
							&& Layer3.m_flWeight < 0.2f
							&& Layer3.m_flWeightDeltaRate > 7.f)
						{
							float diff = Math::AngleDiff(_currentRecord.m_LowerBodyYaw, _currentRecord.m_EyeAngles.yaw);
							if (fabsf(diff) > 35.f) //delta was positive 66
								found = true;
							g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 6;

							LatestRecord.m_RotationMode = ROTATE_RIGHT;
						}
					}
					if (!found)
					{


						if (flFeetDelta > 0.0f)
						{
							if (m_iFreestandSide == ROTATE_LEFT)
							{
								LatestRecord.m_bAnimResolved = true;
								LatestRecord.m_RotationMode = ROTATE_LEFT;
								g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 41;
							}
							else
							{
								LatestRecord.m_bAnimResolved = false;
								LatestRecord.m_RotationMode = m_iFreestandSide;
								g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 42;
							}
						}
						else
						{
							if (m_iFreestandSide == ROTATE_RIGHT)
							{
								LatestRecord.m_bAnimResolved = true;
								LatestRecord.m_RotationMode = ROTATE_RIGHT;
								g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 43;
							}
							else
							{
								LatestRecord.m_bAnimResolved = false;
								LatestRecord.m_RotationMode = m_iFreestandSide;
								g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 44;
							}
						}
					}
				}

				g_Globals.m_ResolverData.m_AnimResoled[iPlayerID] = LatestRecord.m_bAnimResolved;

				if (g_Globals.m_ResolverData.m_MissedShots[iPlayerID] > 0)
				{
					switch (g_Globals.m_ResolverData.m_MissedShots[iPlayerID] % 3)
					{
					case 1:
						if (HaveLowDelta)
						{
							if (g_Globals.m_ResolverData.m_BruteSide[iPlayerID] == ROTATE_LEFT)
								LatestRecord.m_RotationMode = ROTATE_RIGHT;
							else if (g_Globals.m_ResolverData.m_BruteSide[iPlayerID] == ROTATE_RIGHT)
								LatestRecord.m_RotationMode = ROTATE_LEFT;
							else
								LatestRecord.m_RotationMode = ROTATE_RIGHT;
						}
						else
						{
							if (g_Globals.m_ResolverData.m_BruteSide[iPlayerID] == ROTATE_LEFT)
								LatestRecord.m_RotationMode = ROTATE_RIGHT;
							else if (g_Globals.m_ResolverData.m_BruteSide[iPlayerID] == ROTATE_RIGHT)
								LatestRecord.m_RotationMode = ROTATE_LEFT;
							else
								LatestRecord.m_RotationMode = ROTATE_RIGHT;
						}
						g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 8;
						break;

					case 2:
						LatestRecord.m_RotationMode = ROTATE_CENTER;
						g_Globals.m_ResolverData.m_ResolverType[iPlayerID] = 9;
						break;
					}
				}

				g_Globals.m_ResolverData.m_ResolverSide[iPlayerID] = LatestRecord.m_RotationMode;

				this->UpdatePlayerAnimations( pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, LatestRecord.m_RotationMode );
			}
			else
				this->UpdatePlayerAnimations( pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, ROTATE_SERVER );
		}
		else
			this->UpdatePlayerAnimations( pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, ROTATE_SERVER );

		// ôîðñèì ïðàâèëüíûå ëååðû
		std::memcpy( pPlayer->m_AnimationLayers( ), AnimationLayers.data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );

		// ñýéâèì ïîçû
		std::memcpy( LatestRecord.m_PoseParameters.data( ), pPlayer->m_aPoseParameters( ).data( ), sizeof( float_t ) * MAXSTUDIOPOSEPARAM );

		// ñåòàïèì êîñòè
		g_BoneManager->BuildMatrix( pPlayer, LatestRecord.m_Matricies[ ROTATE_SERVER ].data( ), false );

		// ñýéâèì êîñòè
		for ( int i = 0; i < MAXSTUDIOBONES; i++ )
			m_BoneOrigins[ iPlayerID ][ i ] = pPlayer->GetAbsOrigin( ) - LatestRecord.m_Matricies[ ROTATE_SERVER ][ i ].GetOrigin( );

		// êýøèðóåì êîñòè
		std::memcpy( m_CachedMatrix[ iPlayerID ].data( ), LatestRecord.m_Matricies[ ROTATE_SERVER ].data( ), sizeof( matrix3x4_t ) * MAXSTUDIOBONES );

		// ïëååð âûøåë ñ äîðìàíòà
		this->UnmarkAsDormant( iPlayerID );
	}
}

void C_AnimationSync::UpdatePlayerAnimations( C_BasePlayer* pPlayer, C_LagRecord& LagRecord, C_LagRecord PreviousRecord, bool bHasPreviousRecord, int32_t iRotationMode )
{
	float_t flCurTime = g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime;
	float_t flRealTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
	float_t flAbsFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime;
	float_t flFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime;
	float_t iFrameCount = g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount;
	float_t iTickCount = g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount;
	float_t flInterpolationAmount = g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount;

	float_t flLowerBodyYaw = LagRecord.m_LowerBodyYaw;
	float_t flDuckAmount = LagRecord.m_DuckAmount;
	int32_t iFlags = LagRecord.m_Flags;
	int32_t iEFlags = pPlayer->m_iEFlags( );
	
	if ( this->HasLeftOutOfDormancy( pPlayer->EntIndex( ) ) )
	{
		float_t flLastUpdateTime = LagRecord.m_SimulationTime - g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
		if ( pPlayer->m_fFlags( ) & FL_ONGROUND )
		{
			pPlayer->m_PlayerAnimStateCSGO( )->m_bLanding = false;
			pPlayer->m_PlayerAnimStateCSGO( )->m_bOnGround = true;

			float_t flLandTime = 0.0f;
			if ( LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flCycle > 0.0f && 
				 LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flPlaybackRate > 0.0f )
			{ 
				int32_t iLandActivity = pPlayer->GetSequenceActivity( LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_nSequence );
				if ( iLandActivity == ACT_CSGO_LAND_LIGHT || iLandActivity == ACT_CSGO_LAND_HEAVY )
				{
					flLandTime = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flCycle / LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flPlaybackRate;
					if ( flLandTime > 0.0f )
						flLastUpdateTime = LagRecord.m_SimulationTime - flLandTime;
				}
			}

			LagRecord.m_Velocity.z = 0.0f;
		}
		else
		{
			float_t flJumpTime = 0.0f;
			if ( LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle > 0.0f && 
				 LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate > 0.0f )
			{ 
				int32_t iJumpActivity = pPlayer->GetSequenceActivity( LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_nSequence );
				if ( iJumpActivity == ACT_CSGO_JUMP )
				{
					flJumpTime = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle / LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate;
					if ( flJumpTime > 0.0f )
						flLastUpdateTime = LagRecord.m_SimulationTime - flJumpTime;
				}
			}
			
			pPlayer->m_PlayerAnimStateCSGO( )->m_bOnGround = false;
			pPlayer->m_PlayerAnimStateCSGO( )->m_flDurationInAir = flJumpTime - g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
		}

		float_t flWeight = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flWeight;
		if ( LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flPlaybackRate < 0.00001f )
			LagRecord.m_Velocity.Zero( );
		else
		{
			float_t flPostVelocityLength = pPlayer->m_vecVelocity( ).Length( );
			if ( flWeight > 0.0f && flWeight < 0.95f )
		{
				float_t flMaxSpeed = pPlayer->GetMaxPlayerSpeed( );
				if ( flPostVelocityLength > 0.0f )
				{
					float_t flMaxSpeedMultiply = 1.0f;
					if ( pPlayer->m_fFlags( ) & 6 )
						flMaxSpeedMultiply = 0.34f;
					else if ( pPlayer->m_bIsWalking( ) )
						flMaxSpeedMultiply = 0.52f;

					LagRecord.m_Velocity.x = ( LagRecord.m_Velocity.x / flPostVelocityLength ) * ( flWeight * ( flMaxSpeed * flMaxSpeedMultiply ) );
					LagRecord.m_Velocity.y = ( LagRecord.m_Velocity.y / flPostVelocityLength ) * ( flWeight * ( flMaxSpeed * flMaxSpeedMultiply ) );
				}
			}
		}

		pPlayer->m_PlayerAnimStateCSGO( )->m_flLastUpdateTime = flLastUpdateTime;
	}

	if ( bHasPreviousRecord )
	{
		pPlayer->m_PlayerAnimStateCSGO( )->m_flStrafeChangeCycle = PreviousRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flStrafeChangeWeight = PreviousRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO( )->m_nStrafeSequence = PreviousRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_nSequence;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flPrimaryCycle = PreviousRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flMoveWeight = PreviousRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flAccelerationWeight = PreviousRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_LEAN ).m_flWeight;
		std::memcpy( pPlayer->m_AnimationLayers( ), PreviousRecord.m_AnimationLayers.at( ROTATE_SERVER ).data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
	}
	else
	{
		pPlayer->m_PlayerAnimStateCSGO( )->m_flStrafeChangeCycle = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flStrafeChangeWeight = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO( )->m_nStrafeSequence = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_nSequence;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flPrimaryCycle = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flMoveWeight = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flAccelerationWeight = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_LEAN ).m_flWeight;
		std::memcpy( pPlayer->m_AnimationLayers( ), LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
	}

	if ( LagRecord.m_UpdateDelay > 1 )
	{
		int32_t iActivityTick = 0;
		int32_t iActivityType = 0;

		if ( LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flWeight > 0.0f && PreviousRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flWeight <= 0.0f )
		{
			int32_t iLandSequence = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_nSequence;
			if ( iLandSequence > 2 )
			{
				int32_t iLandActivity = pPlayer->GetSequenceActivity( iLandSequence );
				if ( iLandActivity == ACT_CSGO_LAND_LIGHT || iLandActivity == ACT_CSGO_LAND_HEAVY )
				{
					float_t flCurrentCycle = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flCycle;
					float_t flCurrentRate = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flPlaybackRate;
	
					if ( flCurrentCycle > 0.0f && flCurrentRate > 0.0f )
					{	
						float_t flLandTime = ( flCurrentCycle / flCurrentRate );
						if ( flLandTime > 0.0f )
						{
							iActivityTick = TIME_TO_TICKS( LagRecord.m_SimulationTime - flLandTime ) + 1;
							iActivityType = ACTIVITY_LAND;
						}
					}
				}
			}
		}

		if ( LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle > 0.0f && LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate > 0.0f )
		{
			int32_t iJumpSequence = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_nSequence;
			if ( iJumpSequence > 2 )
			{
				int32_t iJumpActivity = pPlayer->GetSequenceActivity( iJumpSequence );
				if ( iJumpActivity == ACT_CSGO_JUMP )
				{
					float_t flCurrentCycle = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle;
					float_t flCurrentRate = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate;
	
					if ( flCurrentCycle > 0.0f && flCurrentRate > 0.0f )
					{	
						float_t flJumpTime = ( flCurrentCycle / flCurrentRate );
						if ( flJumpTime > 0.0f )
						{
							iActivityTick = TIME_TO_TICKS( LagRecord.m_SimulationTime - flJumpTime ) + 1;
							iActivityType = ACTIVITY_JUMP;
						}
					}
				}
			}
		}

		if ( LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle > 0.0f )
			LagRecord.m_bJumped = true;

		for ( int32_t iSimulationTick = 1; iSimulationTick <= LagRecord.m_UpdateDelay; iSimulationTick++ )
		{
			float_t flSimulationTime = PreviousRecord.m_SimulationTime + TICKS_TO_TIME( iSimulationTick );
			g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = flSimulationTime;
			g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime = flSimulationTime;
			g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
			g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
			g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount = TIME_TO_TICKS( flSimulationTime );
			g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount = TIME_TO_TICKS( flSimulationTime );
			g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount = 0.0f;
			
			pPlayer->m_flDuckAmount( ) = Interpolate( PreviousRecord.m_DuckAmount, LagRecord.m_DuckAmount, iSimulationTick, LagRecord.m_UpdateDelay );
			pPlayer->m_vecVelocity( ) = Interpolate( PreviousRecord.m_Velocity, LagRecord.m_Velocity, iSimulationTick, LagRecord.m_UpdateDelay );
			pPlayer->m_vecAbsVelocity( ) = Interpolate( PreviousRecord.m_Velocity, LagRecord.m_Velocity, iSimulationTick, LagRecord.m_UpdateDelay );

			if ( iSimulationTick < LagRecord.m_UpdateDelay )
			{
				int32_t iCurrentSimulationTick = TIME_TO_TICKS( flSimulationTime );
				if ( iActivityType > ACTIVITY_NONE )
				{
					bool bIsOnGround = pPlayer->m_fFlags( ) & FL_ONGROUND;
					if ( iActivityType == ACTIVITY_JUMP )
					{
						if ( iCurrentSimulationTick == iActivityTick - 1 )
							bIsOnGround = true;
						else if ( iCurrentSimulationTick == iActivityTick )
						{
							// reset animation layer
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_flCycle = 0.0f;
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_nSequence = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_nSequence;
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_flPlaybackRate = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate;

							// reset player ground state
							bIsOnGround = false;
						}
						
					}
					else if ( iActivityType == ACTIVITY_LAND )
					{
						if ( iCurrentSimulationTick == iActivityTick - 1 )
							bIsOnGround = false;
						else if ( iCurrentSimulationTick == iActivityTick )
						{
							// reset animation layer
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_flCycle = 0.0f;
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_nSequence = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_nSequence;
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_flPlaybackRate = LagRecord.m_AnimationLayers.at( ROTATE_SERVER ).at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flPlaybackRate;

							// reset player ground state
							bIsOnGround = true;
						}
					}

					if ( bIsOnGround )
						pPlayer->m_fFlags( ) |= FL_ONGROUND;
					else
						pPlayer->m_fFlags( ) &= ~FL_ONGROUND;
				}

				if ( iRotationMode )
				{
					bool HaveLowDelta = g_Globals.m_ResolverData.m_ResolverDelta[pPlayer->EntIndex()] < 25.f;
					bool HaveOpposite = g_Globals.m_ResolverData.m_ResolverDelta[pPlayer->EntIndex()] > 60.f;
					switch (iRotationMode)
					{
					case ROTATE_LEFT: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw - HaveLowDelta ? 25.0f : HaveOpposite ? 90.f : 60.0f); break;
					case ROTATE_RIGHT: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw + HaveLowDelta ? 25.0f : HaveOpposite ? 90.f : 60.0f); break;
					case ROTATE_LOW_LEFT: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw - 25.0f); break;
					case ROTATE_LOW_RIGHT: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw + 25.0f); break;
					case ROTATE_CENTER: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw); break;
					}

					pPlayer->m_PlayerAnimStateCSGO( )->m_flFootYaw = LagRecord.m_BruteYaw;
				}
			}
			else
			{
				pPlayer->m_vecVelocity( ) = LagRecord.m_Velocity;
				pPlayer->m_vecAbsVelocity( ) = LagRecord.m_Velocity;
				pPlayer->m_flDuckAmount( ) = LagRecord.m_DuckAmount;
				pPlayer->m_fFlags( ) = LagRecord.m_Flags;
			}

			if ( pPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame > g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount - 1 )
				pPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame = g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount - 1;

			bool bClientSideAnimation = pPlayer->m_bClientSideAnimation( );
			pPlayer->m_bClientSideAnimation( ) = true;
		
			for ( int32_t iLayer = NULL; iLayer < ANIMATION_LAYER_COUNT; iLayer++ )
				pPlayer->m_AnimationLayers( )[ iLayer ].m_pOwner = pPlayer;

			g_Globals.m_AnimationData.m_bAnimationUpdate = true;
			pPlayer->UpdateClientSideAnimation( );
			g_Globals.m_AnimationData.m_bAnimationUpdate = false;
		
			pPlayer->m_bClientSideAnimation( ) = bClientSideAnimation;
		}
	}
	else
	{
		g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = LagRecord.m_SimulationTime;
		g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime = LagRecord.m_SimulationTime;
		g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
		g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
		g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount = TIME_TO_TICKS( LagRecord.m_SimulationTime );
		g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount = TIME_TO_TICKS( LagRecord.m_SimulationTime );
		g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount = 0.0f;

		pPlayer->m_vecVelocity( ) = LagRecord.m_Velocity;
		pPlayer->m_vecAbsVelocity( ) = LagRecord.m_Velocity;

		if (iRotationMode)
		{
			bool HaveLowDelta = g_Globals.m_ResolverData.m_ResolverDelta[pPlayer->EntIndex()] < 25.f;
			bool HaveOpposite = g_Globals.m_ResolverData.m_ResolverDelta[pPlayer->EntIndex()] > 60.f;

			switch (iRotationMode)
			{
			case ROTATE_LEFT: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw - HaveLowDelta ? 25.0f : HaveOpposite ? 90.f : 60.0f); break;
			case ROTATE_RIGHT: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw + HaveLowDelta ? 25.0f : HaveOpposite ? 90.f : 60.0f); break;
			case ROTATE_LOW_LEFT: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw - 25.0f); break;
			case ROTATE_LOW_RIGHT: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw + 25.0f); break;
			case ROTATE_CENTER: LagRecord.m_BruteYaw = Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw); break;
			}

			pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = LagRecord.m_BruteYaw;
		}

		if ( pPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame > g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount - 1 )
			pPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame = g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount - 1;

		bool bClientSideAnimation = pPlayer->m_bClientSideAnimation( );
		pPlayer->m_bClientSideAnimation( ) = true;
		
		for ( int32_t iLayer = NULL; iLayer < ANIMATION_LAYER_COUNT; iLayer++ )
			pPlayer->m_AnimationLayers( )[ iLayer ].m_pOwner = pPlayer;

		g_Globals.m_AnimationData.m_bAnimationUpdate = true;
		pPlayer->UpdateClientSideAnimation( );
		g_Globals.m_AnimationData.m_bAnimationUpdate = false;
		
		pPlayer->m_bClientSideAnimation( ) = bClientSideAnimation;
	}

	pPlayer->m_flLowerBodyYaw( ) = flLowerBodyYaw;
	pPlayer->m_flDuckAmount( ) = flDuckAmount;
	pPlayer->m_iEFlags( ) = iEFlags;
	pPlayer->m_fFlags( ) = iFlags;

	g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = flCurTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime = flRealTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime = flAbsFrameTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = flFrameTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount = iFrameCount;
	g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount = iTickCount;
	g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount = flInterpolationAmount;

	return pPlayer->InvalidatePhysicsRecursive( ANIMATION_CHANGED );
}

bool C_AnimationSync::GetCachedMatrix( C_BasePlayer* pPlayer, matrix3x4_t* aMatrix )
{
	std::memcpy( aMatrix, m_CachedMatrix[ pPlayer->EntIndex( ) ].data( ), sizeof( matrix3x4_t ) * pPlayer->m_CachedBoneData( ).Count( ) );
	return true;
}

void C_AnimationSync::OnUpdateClientSideAnimation( C_BasePlayer* pPlayer )
{
	for ( int i = 0; i < MAXSTUDIOBONES; i++ )
		m_CachedMatrix[ pPlayer->EntIndex( ) ][ i ].SetOrigin( pPlayer->GetAbsOrigin( ) - m_BoneOrigins[ pPlayer->EntIndex( ) ][ i ] );

	std::memcpy( pPlayer->m_CachedBoneData( ).Base( ), m_CachedMatrix[ pPlayer->EntIndex( ) ].data( ), sizeof( matrix3x4_t ) * pPlayer->m_CachedBoneData( ).Count( ) );
	std::memcpy( pPlayer->GetBoneAccessor( ).GetBoneArrayForWrite( ), m_CachedMatrix[ pPlayer->EntIndex( ) ].data( ), sizeof( matrix3x4_t ) * pPlayer->m_CachedBoneData( ).Count( ) );
	
	return pPlayer->SetupBones_AttachmentHelper( );
}