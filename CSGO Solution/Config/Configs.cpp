#include "Configs.hpp"
#include "../Tools/Obfuscation/XorStr.hpp"
#include <Windows.h>
#include <iomanip>
#include "../Settings.hpp"
#include "../Features/Lua/Clua.h"
#include "../Features/Inventory/inventorychanger.h"
#include "../Tools/Tools.hpp"
#include "../Features/Inventory/protobuf/Protobuffs.h"

std::unordered_map <std::string, float[4]> colors;


void C_ConfigManager::Setup()
{
	for (int i = 0; i < 7; i++)
	{
		setup_item(&g_Settings->m_aRageSettings[i].m_bAutoStop, false, _S("m_bAutoStop") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_bAutoScope, false, _S("m_bAutoScope") + std::to_string(i));

		setup_item(&g_Settings->m_aRageSettings[i].m_iFOV, 0, _S("aimbot_m_iFOV") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_iHitChance, 0, _S("aimbot_m_iHitChance") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_iDoubleTapHitChance, 0, _S("aimbot_m_iDoubleTapHitChance") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_iMinDamage, 0, _S("aimbot_m_iMinDamage") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_iMinPenDamage, 0, _S("m_iMinPenDamage") + std::to_string(i));

		setup_item(&g_Settings->m_aRageSettings[i].m_iMinDamageOverride, 0, _S("aimbot_m_iMinDamageOverride") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_iAccuracyBoost, 0, _S("aimbot_m_iAccuracyBoost") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_iHeadScale, 0, _S("aimbot_m_iHeadScale") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_iBodyScale, 0, _S("aimbot_m_iBodyScale") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_bEnabled, false, _S("aimbot_m_bEnabled") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_bLethalSafety, false, _S("aimbot_m_bLethalSafety") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_bPreferSafe, false, _S("aimbot_m_bPreferSafe") + std::to_string(i));
		setup_item(&g_Settings->m_aRageSettings[i].m_bPreferBody, false, _S("aimbot_m_bPreferBody") + std::to_string(i));

		for (int j = 0; j < 3; j++)
		{
			setup_item(&g_Settings->m_aRageSettings[i].m_AutoStopOptions[j], false, _S("aimbot_m_aAutoStopConditions") + std::to_string(i) + std::to_string(j));
		}

		for (int z = 0; z < 6; z++)
		{
			setup_item(&g_Settings->m_aRageSettings[i].m_Hitboxes[z], false, _S("aimbot_m_aHitboxes") + std::to_string(i) + std::to_string(z));
			setup_item(&g_Settings->m_aRageSettings[i].m_SafeHitboxes[z], false, _S("aimbot_m_aSafeHitboxes") + std::to_string(i) + std::to_string(z));
			setup_item(&g_Settings->m_aRageSettings[i].m_Multipoints[z], false, _S("aimbot_m_aMultipoints") + std::to_string(i) + std::to_string(z));
		}
	}

	for (int z = 0; z < 6; z++)
	{
		setup_item(&g_Settings->m_aLogs[z], false, _S("m_aLogs") + std::to_string(z));
	}

	setup_item(g_Settings->m_flRechargeTime, 0.f, _S("m_flRechargeTime"));

	setup_item(g_Settings->m_bBunnyHop, false, _S("m_bBunnyHop"));
	setup_item(g_Settings->m_bAntiUntrusted, true, _S("m_bAntiUntrusted"));
	setup_item(g_Settings->m_bAutoStrafe, false, _S("m_bAutoStrafe"));
	setup_item(g_Settings->m_bWASDStrafe, false, _S("m_bWASDStrafe"));
	setup_item(g_Settings->m_bSpeedBoost, false, _S("m_bSpeedBoost"));
	setup_item(g_Settings->m_bEdgeJump, false, _S("m_bEdgeJump"));
	setup_item(g_Settings->m_bFilterConsole, false, _S("m_bFilterConsole"));
	setup_item(g_Settings->m_bUnhideConvars, false, _S("m_bUnhideConvars"));
	setup_item(g_Settings->m_bRevealRanks, false, _S("m_bRevealRanks"));
	setup_item(g_Settings->m_bAdBlock, false, _S("m_bAdBlock"));
	setup_item(g_Settings->m_bInfinityDuck, false, _S("m_bInfinityDuck"));
	setup_item(g_Settings->m_bFastStop, false, _S("m_bFastStop"));
	setup_item(g_Settings->m_bUnlockInventoryAccess, false, _S("m_bUnlockInventoryAccess"));
	setup_item(g_Settings->m_bHoldFireAnimation, false, _S("m_bHoldFireAnimation"));
	setup_item(g_Settings->m_bPreserveKillfeed, false, _S("m_bPreserveKillfeed"));
	setup_item(g_Settings->m_bFakeLagEnabled, false, _S("m_bFakeLagEnabled"));
	setup_item(g_Settings->m_iLagLimit, 0, _S("m_iLagLimit"));
	setup_item(g_Settings->m_iTriggerLimit, 0, _S("m_iTriggerLimit"));
	setup_item(g_Settings->m_bAntiAim, false, _S("m_bAntiAim"));
	setup_item(g_Settings->m_bAntiAimAtTargets, false, _S("m_bAntiAimAtTargets"));
	setup_item(g_Settings->m_iLeftFakeLimit, 0, _S("m_iLeftFakeLimit"));
	setup_item(g_Settings->m_iRightFakeLimit, 0, _S("m_iRightFakeLimit"));
	setup_item(g_Settings->m_iJitterAmount, 0, _S("m_iJitterAmount"));
	setup_item(g_Settings->m_iYawAddOffset, 0, _S("m_iYawAddOffset"));
	setup_item(g_Settings->m_iPitchMode, 0, _S("m_iPitchMode"));
	setup_item(g_Settings->m_iLegMovement, 0, _S("m_iLegMovement"));
	setup_item(g_Settings->m_bAutoDirection, false, _S("m_bAutoDirection"));
	setup_item(g_Settings->m_bJitterMove, false, _S("m_bJitterMove"));
	setup_item(g_Settings->m_bDrawServerImpacts, false, _S("m_bDrawServerImpacts"));
	setup_item(g_Settings->m_bDrawClientImpacts, false, _S("m_bDrawClientImpacts"));
	setup_item(g_Settings->m_bDrawLocalTracers, false, _S("m_bDrawLocalTracers"));
	setup_item(g_Settings->m_bDrawEnemyTracers, false, _S("m_bDrawEnemyTracers"));
	setup_item(g_Settings->m_bDrawRagdolls, false, _S("m_bDrawRagdolls"));
	setup_item(g_Settings->m_bOverrideFOVWhileScoped, false, _S("m_bOverrideFOVWhileScoped"));
	setup_item(g_Settings->m_iThirdPersonDistance, 90, _S("m_iThirdPersonDistance"));
	setup_item(g_Settings->m_iCameraDistance, 90, _S("m_iCameraDistance"));
	setup_item(g_Settings->m_iViewmodelDistance, 60, _S("m_iViewmodelDistance"));
	setup_item(g_Settings->m_iViewmodelX, 1, _S("m_iViewmodelX"));
	setup_item(g_Settings->m_iViewmodelY, 1, _S("m_iViewmodelY"));
	setup_item(g_Settings->m_iViewmodelZ, -1, _S("m_iViewmodelZ"));
	setup_item(g_Settings->m_iViewmodelRoll, 0, _S("m_iViewmodelRoll"));
	setup_item(g_Settings->m_iSkybox, 0, _S("m_iSkybox"));
	setup_item(g_Settings->m_VisualInterpolation, 14, _S("m_VisualInterpolation"));
	setup_item(g_Settings->m_szCustomSkybox, "", _S("m_szCustomSkybox"));
	setup_item(g_Settings->m_bBuyBotEnabled, false, _S("m_bBuyBotEnabled"));
	setup_item(g_Settings->m_bBuyBotKeepAWP, false, _S("m_bBuyBotKeepAWP"));
	setup_item(g_Settings->m_BuyBotPrimaryWeapon, 0, _S("m_BuyBotPrimaryWeapon"));
	setup_item(g_Settings->m_BuyBotSecondaryWeapon, 0, _S("m_BuyBotSecondaryWeapon"));
	setup_item(g_Settings->m_bRenderC4Glow, false, _S("m_bRenderC4Glow"));
	setup_item(g_Settings->m_iC4GlowStyle, false, _S("m_iC4GlowStyle"));
	setup_item(g_Settings->m_bRenderDroppedWeaponGlow, false, _S("m_bRenderDroppedWeaponGlow"));
	setup_item(g_Settings->m_iDroppedWeaponGlowStyle, false, _S("m_iDroppedWeaponGlowStyle"));
	setup_item(g_Settings->m_bRenderProjectileGlow, false, _S("m_bRenderProjectileGlow"));
	setup_item(g_Settings->m_GrenadeTimers, false, _S("m_GrenadeTimers"));
	setup_item(g_Settings->m_bPredictGrenades, false, _S("m_bPredictGrenades"));
	setup_item(g_Settings->m_bForceCrosshair, false, _S("m_bForceCrosshair"));
	setup_item(g_Settings->m_bHitMarker, false, _S("m_bHitMarker"));
	setup_item(g_Settings->m_bHitSound, false, _S("m_bHitSound"));
	setup_item(g_Settings->m_bDamageMarker, false, _S("m_bDamageMarker"));
	setup_item(g_Settings->m_flAspectRatio, 1.8f, _S("m_flAspectRatio"));
	setup_item(g_Settings->m_bClantag, false, _S("m_bClantag"));

	setup_item(g_Settings->m_szCustomFont, "", _S("CustomFontName"));

	setup_item(g_Settings->m_iProjectileGlowStyle, 0, _S("m_iProjectileGlowStyle"));

	setup_item(&g_Settings->m_aSlowwalk.Get().m_iKeySelected, 0, _S("m_aSlowwalk.m_iKeySelected"));
	setup_item(&g_Settings->m_aSlowwalk.Get().m_iModeSelected, 0, _S("m_aSlowwalk.m_iModeSelected"));
	setup_item(&g_Settings->m_aThirdPerson.Get().m_iKeySelected, 0, _S("m_aThirdPerson.m_iKeySelected"));
	setup_item(&g_Settings->m_aThirdPerson.Get().m_iModeSelected, 0, _S("m_aThirdPerson.m_iModeSelected"));
	setup_item(&g_Settings->m_aAutoPeek.Get().m_iKeySelected, 0, _S("m_aAutoPeek.m_iKeySelected"));
	setup_item(&g_Settings->m_aAutoPeek.Get().m_iModeSelected, 0, _S("m_aAutoPeek.m_iModeSelected"));
	setup_item(&g_Settings->m_aInverter.Get().m_iKeySelected, 0, _S("m_aInverter.m_iKeySelected"));
	setup_item(&g_Settings->m_aInverter.Get().m_iModeSelected, 0, _S("m_aInverter.m_iModeSelected"));
	setup_item(&g_Settings->m_aMinDamage.Get().m_iKeySelected, 0, _S("m_aMinDamage.m_iKeySelected"));
	setup_item(&g_Settings->m_aMinDamage.Get().m_iModeSelected, 0, _S("m_aMinDamage.m_iModeSelected"));
	setup_item(&g_Settings->m_aSafePoint.Get().m_iModeSelected, 0, _S("m_aSafePoint.m_iModeSelected"));
	setup_item(&g_Settings->m_aSafePoint.Get().m_iModeSelected, 0, _S("m_aSafePoint.m_iModeSelected"));
	setup_item(&g_Settings->m_aManualLeft.Get().m_iModeSelected, 0, _S("m_aManualLeft.m_iModeSelected"));
	setup_item(&g_Settings->m_aManualLeft.Get().m_iModeSelected, 0, _S("m_aManualLeft.m_iModeSelected"));
	setup_item(&g_Settings->m_aManualBack.Get().m_iModeSelected, 0, _S("m_aManualBack.m_iModeSelected"));
	setup_item(&g_Settings->m_aManualBack.Get().m_iModeSelected, 0, _S("m_aManualBack.m_iModeSelected"));
	setup_item(&g_Settings->m_aManualRight.Get().m_iModeSelected, 0, _S("m_aManualRight.m_iModeSelected"));
	setup_item(&g_Settings->m_aManualRight.Get().m_iModeSelected, 0, _S("m_aManualRight.m_iModeSelected"));
	setup_item(&g_Settings->m_aDoubleTap.Get().m_iKeySelected, 0, _S("m_aDoubletap.m_iKeySelected"));
	setup_item(&g_Settings->m_aDoubleTap.Get().m_iModeSelected, 0, _S("m_aDoubletap.m_iModeSelected"));
	setup_item(&g_Settings->m_aHideShots.Get().m_iKeySelected, 0, _S("m_aHideshots.m_iKeySelected"));
	setup_item(&g_Settings->m_aHideShots.Get().m_iModeSelected, 0, _S("m_aHideshots.m_iModeSelected"));
	setup_item(&g_Settings->m_aFakeDuck.Get().m_iKeySelected, 0, _S("m_aFakeDuck.m_iKeySelected"));
	setup_item(&g_Settings->m_aFakeDuck.Get().m_iModeSelected, 0, _S("m_aFakeDuck.m_iModeSelected"));

	//for (int x = 0; x < 3; x++)
	//{
	setup_item(&g_Settings->m_LocalPlayer.Get().m_BoundaryBox, false, _S("m_LocalPlayer.m_BoundaryBox"));
	setup_item(&g_Settings->m_LocalPlayer.Get().m_RenderName, false, _S("m_LocalPlayer.m_RenderName"));
	setup_item(&g_Settings->m_LocalPlayer.Get().m_RenderHealthBar, false, _S("m_LocalPlayer.m_RenderHealthBar"));
	setup_item(&g_Settings->m_LocalPlayer.Get().m_RenderHealthText, false, _S("m_LocalPlayer.m_RenderHealthText"));
	setup_item(&g_Settings->m_LocalPlayer.Get().m_RenderAmmoBar, false, _S("m_LocalPlayer.m_RenderAmmoBar"));
	setup_item(&g_Settings->m_LocalPlayer.Get().m_RenderAmmoBarText, false, _S("m_LocalPlayer.m_RenderAmmoBarText"));
	setup_item(&g_Settings->m_LocalPlayer.Get().m_RenderWeaponText, false, _S("m_LocalPlayer.m_RenderWeaponText"));
	setup_item(&g_Settings->m_LocalPlayer.Get().m_RenderWeaponIcon, false, _S("m_LocalPlayer.m_RenderWeaponIcon"));
	setup_item(&g_Settings->m_LocalPlayer.Get().m_bRenderGlow, false, _S("m_LocalPlayer.m_bRenderGlow"));
	setup_item(&g_Settings->m_LocalPlayer.Get().m_iGlowStyle, 0, _S("m_LocalPlayer.m_iGlowStyle"));
	setup_item(g_Settings->m_bOutOfViewArrows, false, _S("m_bOutOfViewArrows"));

	setup_item(&g_Settings->m_Teammates.Get().m_BoundaryBox, false, _S("m_Teammates.m_BoundaryBox"));
	setup_item(&g_Settings->m_Teammates.Get().m_RenderName, false, _S("m_Teammates.m_RenderName"));
	setup_item(&g_Settings->m_Teammates.Get().m_RenderHealthBar, false, _S("m_Teammates.m_RenderHealthBar"));
	setup_item(&g_Settings->m_Teammates.Get().m_RenderHealthText, false, _S("m_Teammates.m_RenderHealthText"));
	setup_item(&g_Settings->m_Teammates.Get().m_RenderAmmoBar, false, _S("m_Teammates.m_RenderAmmoBar"));
	setup_item(&g_Settings->m_Teammates.Get().m_RenderAmmoBarText, false, _S("m_Teammates.m_RenderAmmoBarText"));
	setup_item(&g_Settings->m_Teammates.Get().m_RenderWeaponText, false, _S("m_Teammates.m_RenderWeaponText"));
	setup_item(&g_Settings->m_Teammates.Get().m_RenderWeaponIcon, false, _S("m_Teammates.m_RenderWeaponIcon"));
	setup_item(&g_Settings->m_Teammates.Get().m_bRenderGlow, false, _S("m_Teammates.m_bRenderGlow"));
	setup_item(&g_Settings->m_Teammates.Get().m_iGlowStyle, 0, _S("m_Teammates.m_iGlowStyle"));

	setup_item(&g_Settings->m_Enemies.Get().m_BoundaryBox, false, _S("m_Enemies.m_BoundaryBox"));
	setup_item(&g_Settings->m_Enemies.Get().m_RenderName, false, _S("m_Enemies.m_RenderName"));
	setup_item(&g_Settings->m_Enemies.Get().m_RenderHealthBar, false, _S("m_Enemies.m_RenderHealthBar"));
	setup_item(&g_Settings->m_Enemies.Get().m_RenderHealthText, false, _S("m_Enemies.m_RenderHealthText"));
	setup_item(&g_Settings->m_Enemies.Get().m_RenderAmmoBar, false, _S("m_Enemies.m_RenderAmmoBar"));
	setup_item(&g_Settings->m_Enemies.Get().m_RenderAmmoBarText, false, _S("m_Enemies.m_RenderAmmoBarText"));
	setup_item(&g_Settings->m_Enemies.Get().m_RenderWeaponText, false, _S("m_Enemies.m_RenderWeaponText"));
	setup_item(&g_Settings->m_Enemies.Get().m_RenderWeaponIcon, false, _S("m_Enemies.m_RenderWeaponIcon"));
	setup_item(&g_Settings->m_Enemies.Get().m_bRenderGlow, false, _S("m_Enemies.m_bRenderGlow"));
	setup_item(&g_Settings->m_Enemies.Get().m_iGlowStyle, 0, _S("m_Enemies.m_iGlowStyle"));
	for (int i = 0; i < 6; i++)
	{
		setup_item(&g_Settings->m_Enemies.Get().m_Flags[i], false, _S("m_Enemies.m_Flags") + std::to_string(i));
		setup_item(&g_Settings->m_Teammates.Get().m_Flags[i], false, _S("m_Teammates.m_Flags") + std::to_string(i));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_Flags[i], false, _S("m_LocalPlayer.m_Flags") + std::to_string(i));
	}

	//}
	for (int c = 0; c < 4; c++)
	{
		setup_item(&g_Settings->m_Enemies.Get().m_aGlow, Color(255, 255, 255, 255), _S("m_Enemies.m_aGlow"));
		setup_item(&g_Settings->m_Enemies.Get().m_aBoundaryBox, Color(255, 255, 255, 255), _S("m_Enemies.m_aBoundaryBox"));
		setup_item(&g_Settings->m_Enemies.Get().m_aNameColor, Color(255, 255, 255, 255), _S("m_Enemies.m_aNameColor"));
		setup_item(&g_Settings->m_Enemies.Get().m_aHealthBar, Color(255, 255, 255, 255), _S("m_Enemies.m_aHealthBar"));
		setup_item(&g_Settings->m_Enemies.Get().m_aHealthText, Color(255, 255, 255, 255), _S("m_Enemies.m_aHealthText"));
		setup_item(&g_Settings->m_Enemies.Get().m_aAmmoBar, Color(255, 255, 255, 255), _S("m_Enemies.m_aAmmoBar"));
		setup_item(&g_Settings->m_Enemies.Get().m_aAmmoBarText, Color(255, 255, 255, 255), _S("m_Enemies.m_aAmmoBarText"));
		setup_item(&g_Settings->m_Enemies.Get().m_aArmorBar, Color(255, 255, 255, 255), _S("m_Enemies.m_aArmorBar"));
		setup_item(&g_Settings->m_Enemies.Get().m_aArmorBarText, Color(255, 255, 255, 255), _S("m_Enemies.m_aArmorBarText"));
		setup_item(&g_Settings->m_Enemies.Get().m_aWeaponText, Color(255, 255, 255, 255), _S("m_Enemies.m_aWeaponText"));
		setup_item(&g_Settings->m_Enemies.Get().m_aWeaponIcon, Color(255, 255, 255, 255), _S("m_Enemies.m_aWeaponIcon"));

		setup_item(&g_Settings->m_Teammates.Get().m_aGlow, Color(255, 255, 255, 255), _S("m_Teammates.m_aGlow"));
		setup_item(&g_Settings->m_Teammates.Get().m_aBoundaryBox, Color(255, 255, 255, 255), _S("m_Teammates.m_aBoundaryBox"));
		setup_item(&g_Settings->m_Teammates.Get().m_aNameColor, Color(255, 255, 255, 255), _S("m_Teammates.m_aNameColor"));
		setup_item(&g_Settings->m_Teammates.Get().m_aHealthBar, Color(255, 255, 255, 255), _S("m_Teammates.m_aHealthBar"));
		setup_item(&g_Settings->m_Teammates.Get().m_aHealthText, Color(255, 255, 255, 255), _S("m_Teammates.m_aHealthText"));
		setup_item(&g_Settings->m_Teammates.Get().m_aAmmoBar, Color(255, 255, 255, 255), _S("m_Teammates.m_aAmmoBar"));
		setup_item(&g_Settings->m_Teammates.Get().m_aAmmoBarText, Color(255, 255, 255, 255), _S("m_Teammates.m_aAmmoBarText"));
		setup_item(&g_Settings->m_Teammates.Get().m_aArmorBar, Color(255, 255, 255, 255), _S("m_Teammates.m_aArmorBar"));
		setup_item(&g_Settings->m_Teammates.Get().m_aArmorBarText, Color(255, 255, 255, 255), _S("m_Teammates.m_aArmorBarText"));
		setup_item(&g_Settings->m_Teammates.Get().m_aWeaponText, Color(255, 255, 255, 255), _S("m_Teammates.m_aWeaponText"));
		setup_item(&g_Settings->m_Teammates.Get().m_aWeaponIcon, Color(255, 255, 255, 255), _S("m_Teammates.m_aWeaponIcon"));

		setup_item(&g_Settings->m_LocalPlayer.Get().m_aGlow, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aGlow"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aBoundaryBox, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aBoundaryBox"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aNameColor, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aNameColor"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aHealthBar, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aHealthBar"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aHealthText, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aHealthText"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aAmmoBar, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aAmmoBar"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aAmmoBarText, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aAmmoBarText"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aArmorBar, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aArmorBar"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aArmorBarText, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aArmorBarText"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aWeaponText, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aWeaponText"));
		setup_item(&g_Settings->m_LocalPlayer.Get().m_aWeaponIcon, Color(255, 255, 255, 255), _S("m_LocalPlayer.m_aWeaponIcon"));


		setup_item(&g_Settings->m_aC4Glow, Color(255, 255, 255, 255), _S("m_aC4Glow"));
		setup_item(&g_Settings->m_aDroppedWeaponGlow, Color(255, 255, 255, 255), _S("m_aDroppedWeaponGlow"));
		setup_item(&g_Settings->m_aProjectileGlow, Color(255, 255, 255, 255), _S("m_aProjectileGlow"));
		setup_item(&g_Settings->m_WorldModulation, Color(255, 255, 255, 255), _S("m_WorldModulation"));
		setup_item(&g_Settings->m_PropModulation, Color(255, 255, 255, 255), _S("m_PropModulation"));
		setup_item(&g_Settings->m_SkyModulation, Color(255, 255, 255, 255), _S("m_SkyModulation"));
		setup_item(&g_Settings->m_ClientImpacts, Color(255, 255, 255, 255), _S("m_ClientImpacts"));
		setup_item(&g_Settings->m_ServerImpacts, Color(255, 255, 255, 255), _S("m_ServerImpacts"));
		setup_item(&g_Settings->m_LocalTracers, Color(255, 255, 255, 255), _S("m_LocalTracers"));
		setup_item(&g_Settings->m_EnemyTracers, Color(255, 255, 255, 255), _S("m_EnemyTracers"));
		setup_item(&g_Settings->m_aDroppedWeaponGlow, Color(255, 255, 255, 255), _S("m_aDroppedWeaponGlow"));
		setup_item(&g_Settings->m_aDroppedWeaponGlow, Color(255, 255, 255, 255), _S("m_aDroppedWeaponGlow"));
		setup_item(&g_Settings->m_aDroppedWeaponGlow, Color(255, 255, 255, 255), _S("m_aDroppedWeaponGlow"));
		setup_item(&g_Settings->m_aDroppedWeaponGlow, Color(255, 255, 255, 255), _S("m_aDroppedWeaponGlow"));
		setup_item(&g_Settings->m_aDroppedWeaponGlow, Color(255, 255, 255, 255), _S("m_aDroppedWeaponGlow"));
		setup_item(&g_Settings->m_aDroppedWeaponGlow, Color(255, 255, 255, 255), _S("m_aDroppedWeaponGlow"));
		setup_item(&g_Settings->m_aDroppedWeaponGlow, Color(255, 255, 255, 255), _S("m_aDroppedWeaponGlow"));
		setup_item(&g_Settings->m_aDroppedWeaponGlow, Color(255, 255, 255, 255), _S("m_aDroppedWeaponGlow"));
		setup_item(&g_Settings->m_GrenadeWarning, Color(255, 255, 255, 255), _S("m_GrenadeWarning"));
		setup_item(&g_Settings->m_GrenadeWarningTimer, Color(255, 255, 255, 255), _S("m_GrenadeWarningTimer"));
		setup_item(&g_Settings->m_aOutOfViewArrows, Color(255, 255, 255, 255), _S("m_aOutOfViewArrows"));
	}



	for (int i = 0; i < 10; i++)
	{
		setup_item(&g_Settings->m_aChamsSettings[i].m_iMainMaterial, 0, _S("m_iMainMaterial") + std::to_string(i));
		setup_item(&g_Settings->m_aChamsSettings[i].m_bRenderChams, false, _S("m_bRenderChams") + std::to_string(i));

		for (int x = 0; x < 5; x++)
			setup_item(&g_Settings->m_aChamsSettings[i].m_aModifiers[x], 0, _S("m_aModifiers") + std::to_string(i) + std::to_string(x));

		for (int c = 0; c < 4; c++)
		{
			for (int v = 0; v < 5; v++)
				setup_item(&g_Settings->m_aChamsSettings[i].m_aModifiersColors[v], Color(255, 255, 255, 255), _S("m_aModifiersColors") + std::to_string(i) + std::to_string(v));

			setup_item(&g_Settings->m_aChamsSettings[i].m_Color, Color(255, 255, 255, 255), _S("m_Color") + std::to_string(i));
		}
	}

	for (int i = 0; i < 10; i++)
		setup_item(&g_Settings->m_aWorldRemovals[i], false, _S("m_aWorldRemovals") + std::to_string(i));

	for (int i = 0; i < 3; i++)
		setup_item(&g_Settings->m_aInverterConditions[i], false, _S("m_aInverterConditions") + std::to_string(i));

	for (int i = 0; i < 3; i++)
		setup_item(&g_Settings->m_aFakelagTriggers[i], false, _S("m_aFakelagTriggers") + std::to_string(i));

	for (int i = 0; i < 7; i++)
		setup_item(&g_Settings->m_aEquipment[i], false, _S("m_aEquipment") + std::to_string(i));

	for (int i = 0; i < 100; i++)
	{
		setup_item(&g_Settings->m_aInventory[i].wId, 0, _S("wId") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].paintKit, 0, _S("paintKit") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].quality, 0, _S("quality") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].wear, 0.f, _S("wear") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].seed, 0, _S("seed") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].name, "", _S("name") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].m_bInUseT, false, _S("m_bInUseT") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].m_bInUseCT, false, _S("m_bInUseCT") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].stattrak, 0, _S("stattrak") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].build_patch, "", _S("build_patch") + std::to_string(i));
		setup_item(&g_Settings->m_aInventory[i].rarity, 0, _S("rarity") + std::to_string(i));
	}
}

void C_ConfigManager::add_item(void* pointer, const char* name, const std::string& type) {
	items.push_back(new C_ConfigItem(std::string(name), pointer, type));
}

void C_ConfigManager::setup_item(int* pointer, int value, const std::string& name)
{
	add_item(pointer, name.c_str(), _S("int"));
	*pointer = value;
}
void C_ConfigManager::setup_item(float* pointer, float value, const std::string& name)
{
	add_item(pointer, name.c_str(), _S("float"));
	*pointer = value;
}

void C_ConfigManager::setup_item(bool* pointer, bool value, const std::string& name)
{
	add_item(pointer, name.c_str(), _S("bool"));
	*pointer = value;
}

void C_ConfigManager::setup_item(Color* pointer, Color value, const std::string& name)
{
	colors[name][0] = (float)value.r() / 255.0f;
	colors[name][1] = (float)value.g() / 255.0f;
	colors[name][2] = (float)value.b() / 255.0f;
	colors[name][3] = (float)value.a() / 255.0f;

	add_item(pointer, name.c_str(), _S("Color"));
	*pointer = value;
}

void C_ConfigManager::setup_item(std::vector< int >* pointer, int size, const std::string& name)
{
	add_item(pointer, name.c_str(), _S("vector<int>"));
	pointer->clear();

	for (int i = 0; i < size; i++)
		pointer->push_back(false);
}
void C_ConfigManager::setup_item(std::vector< std::string >* pointer, const std::string& name)
{
	add_item(pointer, name.c_str(), _S("vector<string>"));
}

void C_ConfigManager::setup_item(std::string* pointer, const std::string& value, const std::string& name)
{
	add_item(pointer, name.c_str(), _S("string"));
	*pointer = value; //-V820
}

void C_ConfigManager::save(std::string config)
{
	std::string folder, file;

	auto get_dir = [&folder, &file, &config]() -> void
	{
		folder = _S("selling.pw\\");
		file = _S("selling.pw\\") + config;

		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	json allJson;

	for (auto it : items)
	{
		json j;

		j[_S("name")] = it->name;
		j[_S("type")] = it->type;

		if (!it->type.compare(_S("int")))
			j[_S("value")] = (int)*(int*)it->pointer; //-V206
		else if (!it->type.compare(_S("float")))
			j[_S("value")] = (float)*(float*)it->pointer;
		else if (!it->type.compare(_S("bool")))
			j[_S("value")] = (bool)*(bool*)it->pointer;
		else if (!it->type.compare(_S("Color")))
		{
			auto c = *(Color*)(it->pointer);

			std::vector<int> a = { c.r(), c.g(), c.b(), c.a() };
			json ja;

			for (auto& i : a)
				ja.push_back(i);

			j[_S("value")] = ja.dump();
		}
		else if (!it->type.compare(_S("vector<int>")))
		{
			auto& ptr = *(std::vector<int>*)(it->pointer);
			json ja;

			for (auto& i : ptr)
				ja.push_back(i);

			j[_S("value")] = ja.dump();
		}
		else if (!it->type.compare(_S("vector<string>")))
		{
			auto& ptr = *(std::vector<std::string>*)(it->pointer);
			json ja;

			for (auto& i : ptr)
				ja.push_back(i);

			j[_S("value")] = ja.dump();
		}
		else if (!it->type.compare(_S("string")))
			j[_S("value")] = (std::string) * (std::string*)it->pointer;

		allJson.push_back(j);
	}

	auto get_type = [](menu_item_type type)
	{
		switch (type) //-V719
		{
		case CHECK_BOX:
			return "bool";
		case COMBO_BOX:
		case SLIDER_INT:
			return "int";
		case SLIDER_FLOAT:
			return "float";
		case COLOR_PICKER:
			return "Color";
		}
	};

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto& script = c_lua::get().scripts.at(i);

		for (auto& item : c_lua::get().items.at(i))
		{
			if (item.second.type == NEXT_LINE)
				continue;

			json j;
			auto type = (std::string)get_type(item.second.type);

			j[_S("name")] = item.first;
			j[_S("type")] = type;

			if (!type.compare(_S("bool")))
				j[_S("value")] = item.second.check_box_value;
			else if (!type.compare(_S("int")))
				j[_S("value")] = item.second.type == COMBO_BOX ? item.second.combo_box_value : item.second.slider_int_value;
			else if (!type.compare(_S("float")))
				j[_S("value")] = item.second.slider_float_value;
			else if (!type.compare(_S("Color")))
			{
				std::vector <int> color =
				{
					item.second.color_picker_value.r(),
					item.second.color_picker_value.g(),
					item.second.color_picker_value.b(),
					item.second.color_picker_value.a()
				};

				json j_color;

				for (auto& i : color)
					j_color.push_back(i);

				j[_S("value")] = j_color.dump();
			}

			allJson.push_back(j);
		}
	}

	std::string data = allJson.dump();

	//Base64 base64;
	//base64.encode(allJson.dump(), &data);

	std::ofstream ofs;
	ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	ofs << std::setw(4) << data << std::endl;
	ofs.close();

	int i = 0;
	for (auto weapon : g_InventorySkins)
	{
		if (!weapon.second.wId)
			continue;

		g_Settings->m_aInventory[i].wId = weapon.second.wId;
		g_Settings->m_aInventory[i].paintKit = weapon.second.paintKit;
		g_Settings->m_aInventory[i].quality = weapon.second.quality;
		g_Settings->m_aInventory[i].wear = weapon.second.wear;
		g_Settings->m_aInventory[i].seed = weapon.second.seed;
		g_Settings->m_aInventory[i].name = weapon.second.name;
		g_Settings->m_aInventory[i].m_bInUseT = weapon.second.m_bInUseT;
		g_Settings->m_aInventory[i].m_bInUseCT = weapon.second.m_bInUseCT;
		g_Settings->m_aInventory[i].stattrak = weapon.second.stattrak;
		g_Settings->m_aInventory[i].build_patch = weapon.second.build_patch;
		g_Settings->m_aInventory[i].rarity = weapon.second.rarity;

		i++;
	}
}

void C_ConfigManager::load(std::string config, bool load_script_items)
{
	static auto find_item = [](std::vector< C_ConfigItem* > items, std::string name) -> C_ConfigItem*
	{
		for (int i = 0; i < (int)items.size(); i++) //-V202
			if (!items[i]->name.compare(name))
				return items[i];

		return nullptr;
	};

	std::string folder, file;

	auto get_dir = [&folder, &file, &config]() ->void
	{
		folder = _S("selling.pw\\");
		file = _S("selling.pw\\") + config;
		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	std::string data;

	std::ifstream ifs;
	ifs.open(file + '\0');

	ifs >> data;
	ifs.close();

	if (data.empty())
		return;

	//Base64 base64;

	//std::string decoded_data;
	//base64.decode(data, &decoded_data);

	std::ofstream ofs;
	ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	ofs << data/*decoded_data*/;
	ofs.close();

	json allJson;

	std::ifstream ifs_final;
	ifs_final.open(file + '\0');

	ifs_final >> allJson;
	ifs_final.close();

	//base64.encode(allJson.dump(), &data);

	std::ofstream final_ofs;
	final_ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	final_ofs << data;
	final_ofs.close();


	for (auto it = allJson.begin(); it != allJson.end(); ++it)
	{
		json j = *it;

		std::string name = j[_S("name")];
		std::string type = j[_S("type")];

		auto script_item = std::count_if(name.begin(), name.end(),
			[](char& c)
			{
				return c == '.';
			}
		) >= 2;

		if (load_script_items && script_item)
		{
			std::string script_name;
			auto first_point = false;

			for (auto& c : name)
			{
				if (c == '.')
				{
					if (first_point)
						break;
					else
						first_point = true;
				}

				script_name.push_back(c);
			}

			auto script_id = c_lua::get().get_script_id(script_name);

			if (script_id == -1)
				continue;

			for (auto& current_item : c_lua::get().items.at(script_id))
			{
				if (current_item.first == name)
				{
					if (!type.compare(_S("bool")))
					{
						current_item.second.type = CHECK_BOX;
						current_item.second.check_box_value = j[_S("value")].get<bool>();
					}
					else if (!type.compare(_S("int")))
					{
						if (current_item.second.type == COMBO_BOX)
							current_item.second.combo_box_value = j[_S("value")].get<int>();
						else
							current_item.second.slider_int_value = j[_S("value")].get<int>();
					}
					else if (!type.compare(_S("float")))
						current_item.second.slider_float_value = j[_S("value")].get<float>();
					else if (!type.compare(_S("Color")))
					{
						std::vector<int> a;
						json ja = json::parse(j[_S("value")].get<std::string>().c_str());

						for (json::iterator it = ja.begin(); it != ja.end(); ++it)
							a.push_back(*it);

						colors.erase(name);
						current_item.second.color_picker_value = Color(a[0], a[1], a[2], a[3]);
					}
				}
			}
		}
		else if (!load_script_items && !script_item)
		{
			auto item = find_item(items, name);

			if (item)
			{
				if (!type.compare(_S("int")))
					*(int*)item->pointer = j[_S("value")].get<int>(); //-V206
				else if (!type.compare(_S("float")))
					*(float*)item->pointer = j[_S("value")].get<float>();
				else if (!type.compare(_S("bool")))
					*(bool*)item->pointer = j[_S("value")].get<bool>();
				else if (!type.compare(_S("Color")))
				{
					std::vector<int> a;
					json ja = json::parse(j[_S("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						a.push_back(*it);

					colors.erase(item->name);
					*(Color*)item->pointer = Color(a[0], a[1], a[2], a[3]);
				}
				else if (!type.compare(_S("vector<int>")))
				{
					auto ptr = static_cast<std::vector <int>*> (item->pointer);
					ptr->clear();

					json ja = json::parse(j[_S("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						ptr->push_back(*it);
				}
				else if (!type.compare(_S("vector<string>")))
				{
					auto ptr = static_cast<std::vector <std::string>*> (item->pointer);
					ptr->clear();

					json ja = json::parse(j[_S("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						ptr->push_back(*it);
				}
				else if (!type.compare(_S("string")))
					*(std::string*)item->pointer = j[_S("value")].get<std::string>();
			}
		}
	}

	g_InventorySkins.clear();

	for (int i = 0; i < 100; i++)
	{
		if (g_Settings->m_aInventory[i].wId == 0)
			continue;

		wskin skinInfo;
		skinInfo.wId = g_Settings->m_aInventory[i].wId;
		skinInfo.paintKit = g_Settings->m_aInventory[i].paintKit;
		skinInfo.wear = g_Settings->m_aInventory[i].wear;
		skinInfo.seed = g_Settings->m_aInventory[i].seed;
		skinInfo.m_bInUseT = g_Settings->m_aInventory[i].m_bInUseT;
		skinInfo.m_bInUseCT = g_Settings->m_aInventory[i].m_bInUseCT;
		skinInfo.quality = g_Settings->m_aInventory[i].quality;
		skinInfo.name = g_Settings->m_aInventory[i].name;
		skinInfo.stattrak = g_Settings->m_aInventory[i].stattrak;
		skinInfo.build_patch = g_Settings->m_aInventory[i].build_patch;
		skinInfo.rarity = g_Settings->m_aInventory[i].rarity;

		g_InventorySkins.insert({ g_Tools->RandomInt(20000, 200000), skinInfo });
	}

	write.SendClientHello();
	write.SendMatchmakingClient2GCHello();
}

void C_ConfigManager::remove(std::string config)
{
	std::string folder, file;

	auto get_dir = [&folder, &file, &config]() -> void
	{
		folder = _S("selling.pw\\");
		file = _S("selling.pw\\") + config;

		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();

	std::string path = file + '\0';
	std::remove(path.c_str());
}

void C_ConfigManager::config_files()
{
	std::string folder;

	auto get_dir = [&folder]() -> void
	{
		folder = _S("selling.pw\\");

		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	files.clear();

	std::string path = folder + _S("/*.og");
	WIN32_FIND_DATA fd;

	HANDLE hFind = FindFirstFile(path.c_str(), &fd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				files.push_back(fd.cFileName);
		} while (FindNextFile(hFind, &fd));

		FindClose(hFind);
	}
}