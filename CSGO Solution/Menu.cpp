#include <d3d9.h>
#include <d3dx9.h>
#pragma comment (lib, "d3dx9.lib")

#include "Config/Configs.hpp"
#include "Features/Log Manager/LogManager.hpp"
#include "Features/Inventory/inventorychanger.h"
#include "Features/Inventory/protobuf/Protobuffs.h"
#include "Features/Inventory/items.h"
#include "Features/Lua/Clua.h"
#include "Tools/Tools.hpp"
#include "Tools/Obfuscation/XorStr.hpp"
#include "SDK/Globals.hpp"
#include "Data/Ava.h"
#include "../Render.hpp"
#include "../Config.hpp"
#include "../Settings.hpp"
#include "../Features/Model/Model.hpp"
static int selected_script = 0;

const char* flags[] =
{
	("Scoped"),
	("Armor"),
	("Flashed"),
	("Money"),
	("FD")
};
const char* m_aEquipment[] =
{
	("Fire grenade/Molotov"),
	("Smoke grenade"),
	("Flash grenade"),
	("Explosive grenade"),
	("Taser"),
	("Heavy armor"),
	("Helmet"),
	("Defuser")
};
const char* m_aLogs[] =
{
	("Hurt"),
	("Harm"),
	("Purchase"),
	("Bomb"),
	("Miss")
};

const char* double_tap_options[] =
{
	("Move between shots"),
	("Full stop")
};
const char* aAutoStop[] =
{
	("Full Stop"),
	("Early")
};

const char* m_aInverterConditions[] =
{
	("Stand"),
	("Move"),
	("Air")
};

const char* m_aFakelagTriggers[] =
{
	("Move"),
	("Air"),
	("Peek")
};

std::vector <const char*> aRageWeapons =
{
	("Auto"),
	("Scout"),
	("AWP"),
	("Deagle"),
	("Revolver"),
	("Pistol"),
	("Rifle")
};
const char* m_aWorldRemovals[] =
{
	("Visual punch"),
	("Visual kick"),
	("Scope"),
	("Smoke"),
	("Flash"),
	("Post process"),
	("World FOG"),
	("Shadows"),
	("Landing bob"),
	("Hand shaking")
};

const char* aHitboxes[] =
{
	("Head"),
	("Chest"),
	("Arms"),
	("Pelvis"),
	("Stomach"),
	("Legs"),
};

enum STATE_ID {
	STATE_NONE = 0,
	STATE_SELECTING_WEAPON,
	STATE_SELECTING_PAINTKIT,
	STATE_CONFIGURING_WEAPON
};

void draw_combo_lua(const char* name, int& variable, std::vector<const char*> labels)
{
	ImGui::SingleSelect(std::string(("##COMBO__") + std::string(name)).c_str(), &variable, labels);
}


void C_Menu::Instance()
{
	if (!m_bIsMenuOpened && ImGui::GetStyle().Alpha > 0.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha - fc / 255.f, 0.f, 1.f);
	}

	if (m_bIsMenuOpened && ImGui::GetStyle().Alpha < 1.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha + fc / 255.f, 0.f, 1.f);
	}

	this->DrawKeybindList();

	if (!m_bIsMenuOpened && ImGui::GetStyle().Alpha < 0.1f)
	{
		for (const auto& [id, value] : g_Globals.m_Animations.slider_size)
		{
			g_Globals.m_Animations.slider_size[id] = 5.f;
		}
		for (const auto& [id, value] : g_Globals.m_Animations.slider_active_value)
		{
			g_Globals.m_Animations.slider_active_value[id] = 0.f;
		}
		for (const auto& [id, value] : g_Globals.m_Animations.slider_old_value)
		{
			g_Globals.m_Animations.slider_old_value[id] = 0.f;
		}
		for (const auto& [id, value] : g_Globals.m_Animations.checkbox_pos_active)
		{
			g_Globals.m_Animations.checkbox_pos_active[id] = 3.f;
		}
		for (const auto& [id, value] : g_Globals.m_Animations.checkbox_size_active)
		{
			g_Globals.m_Animations.checkbox_size_active[id] = 6.f;
		}
		return;
	}

	int32_t iScreenSizeX, iScreenSizeY;
	g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(iScreenSizeX, iScreenSizeY);

	ImGui::SetNextWindowPos(ImVec2((iScreenSizeX / 2) - 325, (iScreenSizeY / 2) - 220), ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(760, 565), ImVec2(760, 565));
	ImGui::Begin(this->GetMenuName(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);

	ImGui::TabButton(_S("Rage"), _S("a"), &m_iCurrentTab, MENU_RAGE, 9);
	ImGui::TabButton(_S("Anti-aim"), _S("b"), &m_iCurrentTab, MENU_ANTIAIM, 9);
	ImGui::TabButton(_S("Legit"), _S("c"), &m_iCurrentTab, MENU_LEGIT, 9);
	ImGui::TabButton(_S("Players"), _S("d"), &m_iCurrentTab, MENU_PLAYERS, 9);
	ImGui::TabButton(_S("World"), _S("e"), &m_iCurrentTab, MENU_WORLD, 9);
	ImGui::TabButton(_S("Misc"), _S("f"), &m_iCurrentTab, MENU_MISC, 9);
	ImGui::TabButton(_S("Inventory"), _S("h"), &m_iCurrentTab, MENU_INVENTORY, 9);
	ImGui::TabButton(_S("Lua"), _S("i"), &m_iCurrentTab, MENU_LUA, 9);

	if (m_iCurrentTab == MENU_RAGE)
		this->DrawRageTab();
	else if (m_iCurrentTab == MENU_ANTIAIM)
		this->DrawAntiAimTab();
	else if (m_iCurrentTab == MENU_PLAYERS)
		this->DrawPlayersTab();
	else if (m_iCurrentTab == MENU_WORLD)
		this->DrawWorldTab();
	else if (m_iCurrentTab == MENU_MISC)
		this->DrawMiscTab();
	else if (m_iCurrentTab == MENU_INVENTORY)
		this->DrawInventoryTab();
	else if (m_iCurrentTab == MENU_LUA)
		this->DrawLuaTab();



	ImVec2 window_pos = GImGui->CurrentWindow->Pos;
	ImVec2 window_size = GImGui->CurrentWindow->Size;

	GImGui->CurrentWindow->DrawList->PushClipRectFullScreen();

	RectangleShadowSettings shadowSettings;
	shadowSettings.shadowColor = GImGui->Style.Colors[ImGuiCol_BorderShadow];
	ImGui::DrawRectangleShadowVerticesAdaptive(shadowSettings, window_pos, window_size);
	ImGui::End();
}

void C_Menu::DrawRageTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 50) / 2;
	int iChildDoubleSizeY = (565 - 45) / 2;

	int iChildSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 40 - 30);
	int iChildSizeY = (565 - 30);

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 30;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 10;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;
	ImGui::PopFont();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild("Main", ImVec2(iChildDoubleSizeX, iChildDoubleSizeY - 150));

	static int wep = 0;
	ImGui::Checkbox(_S("Enable Ragebot"), &g_Settings->m_aRageSettings[wep].m_bEnabled);
	ImGui::Checkbox(_S("Auto stop"), &g_Settings->m_aRageSettings[wep].m_bAutoStop);
	ImGui::Checkbox(_S("Auto scope"), &g_Settings->m_aRageSettings[wep].m_bAutoScope);
	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosSecondY - 150));
	ImGui::BeginChild(_S("Accuracy"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY + 150));

	ImGui::SingleSelect(_S("Weapon"), &wep, aRageWeapons);
	ImGui::SliderInt(_S("Mindamage"), &g_Settings->m_aRageSettings[wep].m_iMinDamage, 0, 100);
	ImGui::SliderInt(_S("Mindamage autowall"), &g_Settings->m_aRageSettings[wep].m_iMinPenDamage, 0, 100);

	ImGui::SliderInt(_S("Mindamage override"), &g_Settings->m_aRageSettings[wep].m_iMinDamageOverride, 0, 100);
	ImGui::SliderInt(_S("Hitchance"), &g_Settings->m_aRageSettings[wep].m_iHitChance, 0, 100);
	ImGui::SliderInt(_S("Accuracy boost"), &g_Settings->m_aRageSettings[wep].m_iAccuracyBoost, 0, 100);


	ImGui::Text(_S("Force safe"));
	ImGui::Keybind(_S("Force safe"), &g_Settings->m_aSafePoint->m_iKeySelected, &g_Settings->m_aSafePoint->m_iModeSelected);

	ImGui::Text(_S("Damage override"));
	ImGui::Keybind(_S("Damage override"), &g_Settings->m_aMinDamage->m_iKeySelected, &g_Settings->m_aMinDamage->m_iModeSelected);

	ImGui::MultiCombo(_S("Autostop options"), g_Settings->m_aRageSettings[wep].m_AutoStopOptions, aAutoStop, ARRAYSIZE(aAutoStop), preview);

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosFirstY));
	ImGui::BeginChild(_S("Exploits"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::SliderInt(_S("DT Hitchance"), &g_Settings->m_aRageSettings[wep].m_iDoubleTapHitChance, 0, 100);
	ImGui::MultiCombo(_S("DT options"), g_Settings->m_aRageSettings[wep].m_DoubleTapOptions, double_tap_options, ARRAYSIZE(double_tap_options), preview);
	ImGui::SliderFloat(_S("Recharge time"), g_Settings->m_flRechargeTime, 0.0f, 2.0f, "%.1f");

	ImGui::Text(_S("Double tap"));
	ImGui::Keybind(_S("Double tap"), &g_Settings->m_aDoubleTap->m_iKeySelected, &g_Settings->m_aDoubleTap->m_iModeSelected);

	ImGui::Text(_S("Hide shots"));
	ImGui::Keybind(_S("Hide shots"), &g_Settings->m_aHideShots->m_iKeySelected, &g_Settings->m_aHideShots->m_iModeSelected);

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosSecondY));
	ImGui::BeginChild(_S("Target"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::MultiCombo(_S("Hitboxes"), g_Settings->m_aRageSettings[wep].m_Hitboxes, aHitboxes, ARRAYSIZE(aHitboxes), preview);
	ImGui::MultiCombo(_S("Safe Hitboxes"), g_Settings->m_aRageSettings[wep].m_SafeHitboxes, aHitboxes, ARRAYSIZE(aHitboxes), preview);
	ImGui::MultiCombo(_S("Multipoints"), g_Settings->m_aRageSettings[wep].m_Multipoints, aHitboxes, ARRAYSIZE(aHitboxes), preview);

	ImGui::SliderInt(_S("Head scale"), &g_Settings->m_aRageSettings[wep].m_iHeadScale, 0, 100);
	ImGui::SliderInt(_S("Body scale"), &g_Settings->m_aRageSettings[wep].m_iBodyScale, 0, 100);

	ImGui::Checkbox(_S("Lethal safe"), &g_Settings->m_aRageSettings[wep].m_bLethalSafety);
	ImGui::Checkbox(_S("Prefer body"), &g_Settings->m_aRageSettings[wep].m_bPreferBody);
	ImGui::Checkbox(_S("Prefer safe"), &g_Settings->m_aRageSettings[wep].m_bPreferSafe);

	ImGui::EndChild();
}
void C_Menu::DrawAntiAimTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 50) / 2;
	int iChildDoubleSizeY = (565 - 45) / 2;

	int iChildSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 40 - 30);
	int iChildSizeY = (565 - 30);

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 30;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 10;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;
	ImGui::PopFont();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild(_S("Main"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::Checkbox(_S("Enable Anti-aim"), g_Settings->m_bAntiAim);
	ImGui::Checkbox(_S("At targets"), g_Settings->m_bAntiAimAtTargets);
	ImGui::Checkbox(_S("Auto direction"), g_Settings->m_bAutoDirection);

	ImGui::SliderInt(_S("Yaw add offset"), g_Settings->m_iYawAddOffset, 0, 180);
	ImGui::SliderInt(_S("Jitter amount"), g_Settings->m_iJitterAmount, 0, 180);

	ImGui::Text(_S("Manual left"));
	ImGui::Keybind(_S("123"), &g_Settings->m_aManualLeft->m_iKeySelected, &g_Settings->m_aManualLeft->m_iModeSelected);

	ImGui::Text(_S("Manual back"));
	ImGui::Keybind(_S("1234"), &g_Settings->m_aManualBack->m_iKeySelected, &g_Settings->m_aManualBack->m_iModeSelected);

	ImGui::Text(_S("Manual right"));
	ImGui::Keybind(_S("12346"), &g_Settings->m_aManualRight->m_iKeySelected, &g_Settings->m_aManualRight->m_iModeSelected);

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosSecondY));
	ImGui::BeginChild(_S("Angle settings"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::SingleSelect(_S("Pitch"), g_Settings->m_iPitchMode, { _S("None"), _S("Down"), _S("Up"), _S("Fake down"), _S("Fake up") });
	ImGui::SingleSelect(_S("Lby mode"), g_Settings->m_iLbyMode, { _S("Default"), _S("Opposite"), _S("Sway") });

	ImGui::SliderInt(_S("Left fake limit"), g_Settings->m_iLeftFakeLimit, 0, 60);
	ImGui::SliderInt(_S("Right fake limit"), g_Settings->m_iRightFakeLimit, 0, 60);

	ImGui::MultiCombo(_S("Auto inverter"), g_Settings->m_aInverterConditions, m_aInverterConditions, ARRAYSIZE(m_aInverterConditions), preview);

	ImGui::Text(_S("Inverter"));
	ImGui::Keybind(_S("InvertButton"), &g_Settings->m_aInverter->m_iKeySelected, &g_Settings->m_aInverter->m_iModeSelected);

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosFirstY));
	ImGui::BeginChild(_S("Fakelag settings"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::Checkbox(_S("Enable fakelag"), g_Settings->m_bFakeLagEnabled);

	ImGui::SliderInt(_S("Lag limit"), g_Settings->m_iLagLimit, 1, 14);

	ImGui::MultiCombo(_S("Lag triggers"), g_Settings->m_aFakelagTriggers, m_aFakelagTriggers, ARRAYSIZE(m_aFakelagTriggers), preview);
	ImGui::SliderInt(_S("Trigger limit"), g_Settings->m_iTriggerLimit, 1, 14);

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosSecondY));
	ImGui::BeginChild(_S("Movement"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::Checkbox(_S("Jitter move"), g_Settings->m_bJitterMove);

	ImGui::Text(_S("Slowwalk"));
	ImGui::Keybind(_S("SW"), &g_Settings->m_aSlowwalk->m_iKeySelected, &g_Settings->m_aSlowwalk->m_iModeSelected);

	ImGui::Text(_S("Fakeduck"));
	ImGui::Keybind(_S("FD"), &g_Settings->m_aFakeDuck->m_iKeySelected, &g_Settings->m_aFakeDuck->m_iModeSelected);

	ImGui::Text(_S("Auto peek"));
	ImGui::Keybind(_S("AP"), &g_Settings->m_aAutoPeek->m_iKeySelected, &g_Settings->m_aAutoPeek->m_iModeSelected);

	ImGui::SingleSelect(_S("Leg movement"), g_Settings->m_iLegMovement, { _S("Default"), _S("Slide") });

	ImGui::EndChild();
}
void C_Menu::DrawPlayersTab()
{

	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 50) / 2;
	int iChildDoubleSizeY = (565 - 45) / 2;

	int iChildSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 40 - 30);
	int iChildSizeY = (565 - 30);

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 30;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 10;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;
	ImGui::PopFont();


	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild("ESP", ImVec2(iChildDoubleSizeX, iChildSizeY - 240));
	static int iPlayerESPType = 0;

	ImGui::SingleSelect(_S("Player Type"), &iPlayerESPType, { _S("Enemy"), _S("Team"), _S("Local") });

	static C_PlayerSettings* Settings = NULL;
	switch (iPlayerESPType)
	{
	case 0: Settings = g_Settings->m_Enemies; break;
	case 1: Settings = g_Settings->m_Teammates; break;
	case 2: Settings = g_Settings->m_LocalPlayer; break;
	}

	ImGui::Checkbox(_S("Box"), &Settings->m_BoundaryBox);
	this->DrawColorEdit4(_S("Box##color"), &Settings->m_aBoundaryBox);

	ImGui::Checkbox(_S("Name"), &Settings->m_RenderName);
	this->DrawColorEdit4(_S("Name##color"), &Settings->m_aNameColor);

	ImGui::Checkbox(_S("Health bar"), &Settings->m_RenderHealthBar);
	this->DrawColorEdit4(_S("m_aHealth##color"), &Settings->m_aHealthBar);

	ImGui::Checkbox(_S("Health bar text"), &Settings->m_RenderHealthText);
	this->DrawColorEdit4(_S("m_aHealthText##color"), &Settings->m_aHealthText);

	ImGui::Checkbox(_S("Ammo bar"), &Settings->m_RenderAmmoBar);
	this->DrawColorEdit4(_S("m_aAmmoBar##color"), &Settings->m_aAmmoBar);

	ImGui::Checkbox(_S("Ammo bar text"), &Settings->m_RenderAmmoBarText);
	this->DrawColorEdit4(_S("m_aAmmoBarText##color"), &Settings->m_aAmmoBarText);

	ImGui::Checkbox(_S("Weapon Text"), &Settings->m_RenderWeaponText);
	this->DrawColorEdit4(_S("m_aWeaponText##color"), &Settings->m_aWeaponText);

	ImGui::Checkbox(_S("Weapon Icon"), &Settings->m_RenderWeaponIcon);
	this->DrawColorEdit4(_S("m_aWeaponIcon##color"), &Settings->m_aWeaponIcon);

	ImGui::MultiCombo(_S("Flags"), Settings->m_Flags, flags, ARRAYSIZE(flags), preview);

	ImGui::Checkbox(_S("Out of view arrows"), g_Settings->m_bOutOfViewArrows);
	this->DrawColorEdit4(_S("awerqweqw2e123412er412q4##color"), &g_Settings->m_aOutOfViewArrows);

	static char aFontName[32];
	static int nFontSize = 16;
	if (ImGui::InputText(_S("Custom font"), aFontName, 32, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		g_Settings->m_szCustomFont.Get() = aFontName;
		g_Render->ForceReload(g_Settings->m_szCustomFont.Get(), nFontSize);
	}

	ImGui::SliderInt(_S("Font size"), &nFontSize, 1, 32);

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY + iChildSizeY - 240 + 15));
	ImGui::BeginChild(_S("Glow"), ImVec2(iChildDoubleSizeX, 225));

	ImGui::Checkbox(_S("Player Glow"), &Settings->m_bRenderGlow);
	this->DrawColorEdit4(_S("##m_aGlowcolor"), &Settings->m_aGlow);
	ImGui::SingleSelect(_S("Glow style##1"), &Settings->m_iGlowStyle, { _S("Outline"), _S("Pulse"), _S("Thin"), _S("Thin Pulse") });

	ImGui::Checkbox(_S("C4 Glow"), g_Settings->m_bRenderC4Glow);
	this->DrawColorEdit4(_S("##C4Glowcolor"), &g_Settings->m_aC4Glow);
	ImGui::SingleSelect(_S("Glow style##m_iC4GlowStyle"), g_Settings->m_iC4GlowStyle, { _S("Outline"), _S("Pulse"), _S("Thin"), _S("Thin Pulse") });

	ImGui::Checkbox(_S("Dropped Weapon Glow"), g_Settings->m_bRenderDroppedWeaponGlow);
	this->DrawColorEdit4(_S("m_aDroppedWeaponGlow##color"), &g_Settings->m_aDroppedWeaponGlow);
	ImGui::SingleSelect(_S("Glow style##m_iDroppedWeaponGlowStyle"), g_Settings->m_iDroppedWeaponGlowStyle, { _S("Outline"), _S("Pulse"), _S("Thin"), _S("Thin Pulse") });

	ImGui::Checkbox(_S("Projectiles Glow"), g_Settings->m_bRenderProjectileGlow);
	this->DrawColorEdit4(_S("##Projectile"), &g_Settings->m_aProjectileGlow);
	ImGui::SingleSelect(_S("Glow style##m_iProjectileGlowStyle"), g_Settings->m_iProjectileGlowStyle, { _S("Outline"), _S("Pulse"), _S("Thin"), _S("Thin Pulse") });
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::EndChild();

	g_DrawModel->SetGlow(Settings->m_iGlowStyle);
	if (!Settings->m_bRenderGlow)
		g_DrawModel->SetGlow(-1);
	else
		g_DrawModel->SetGlowColor(Color(Settings->m_aGlow));

	ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosFirstY));
	ImGui::BeginChild(_S("Chams"), ImVec2(iChildDoubleSizeX, iChildSizeY));

	static int32_t iChamsGroup = 0;
	ImGui::SingleSelect(_S("Chams group"), &iChamsGroup, { _S("Enemy visible"), _S("Enemy invisble"), _S("Backtrack"), _S("Shot chams"), _S("Team visible"), _S("Team invisible"), _S("Local"), _S("Desync"), _S("Lag"), _S("Hands") });

	ImGui::Checkbox(_S("Enable Chams"), &g_Settings->m_aChamsSettings[iChamsGroup].m_bRenderChams);
	this->DrawColorEdit4(_S("##qweqwe"), &g_Settings->m_aChamsSettings[iChamsGroup].m_Color);
	ImGui::SingleSelect(_S("Material"), &g_Settings->m_aChamsSettings[iChamsGroup].m_iMainMaterial, { _S("Flat"), _S("Regular") });

	ImGui::Checkbox(_S("Enable Skybox"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiers[0]);
	this->DrawColorEdit4(_S("##51241441 color"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiersColors[0]);
	ImGui::Checkbox(_S("Enable glow"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiers[1]);
	this->DrawColorEdit4(_S("##512414 color"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiersColors[1]);
	ImGui::Checkbox(_S("Enable ghost"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiers[2]);
	this->DrawColorEdit4(_S("##235235 color"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiersColors[2]);
	ImGui::Checkbox(_S("Enable glass"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiers[3]);
	this->DrawColorEdit4(_S("##4124124 color"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiersColors[3]);
	ImGui::Checkbox(_S("Enable pulsation"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiers[4]);
	this->DrawColorEdit4(_S("##123123 color"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiersColors[4]);
	ImGui::Checkbox(_S("Ragdoll chams"), g_Settings->m_bDrawRagdolls);

	if (iChamsGroup < 8)
		g_DrawModel->SetChamsSettings(g_Settings->m_aChamsSettings[iChamsGroup]);

	ImGui::EndChild();

	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 654 + 110, vecWindowPosition.y), ImVec2(vecWindowPosition.x + 950 + 125, vecWindowPosition.y + 440), ImColor(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f), 7.f);
	if (g_DrawModel->GetTexture())
	{
		ImGui::GetForegroundDrawList()->AddImage(
			g_DrawModel->GetTexture()->pTextureHandles[0]->lpRawTexture,
			ImVec2(vecWindowPosition.x + 610 + 110, vecWindowPosition.y - 130),
			ImVec2(vecWindowPosition.x + 610 + 110 + g_DrawModel->GetTexture()->GetActualWidth(), vecWindowPosition.y + g_DrawModel->GetTexture()->GetActualHeight() - 130),
			ImVec2(0, 0), ImVec2(1, 1),
			ImColor(1.0f, 1.0f, 1.0f, 1.0f));
	}

	static ImVec2 vecPreviousMousePosition = ImVec2(0, 0);
	static ImVec2 vecLastMousePosition = ImVec2(0, 0);
	ImVec2 vecCurrentCursorPosition = ImGui::GetMousePos();

	static bool bIsActive = false;

	// render box
	Color aBox = Color(Settings->m_aBoundaryBox);
	if (Settings->m_BoundaryBox)
	{
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 694 + 110, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 886 + 130, vecWindowPosition.y + 386), ImColor(0, 0, 0, 255));
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 696 + 110, vecWindowPosition.y + 41), ImVec2(vecWindowPosition.x + 884 + 130, vecWindowPosition.y + 384), ImColor(0, 0, 0, 255));
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 695 + 110, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 885 + 130, vecWindowPosition.y + 385), ImColor(aBox.r(), aBox.g(), aBox.b(), aBox.a()));
	}

	// render name
	Color aName = Color(Settings->m_aNameColor);
	if (Settings->m_RenderName)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 110 + 85 - ImGui::CalcTextSize(_S("Agent Ava")).x / 2, vecWindowPosition.y + 22), ImColor(aName.r(), aName.g(), aName.b(), aName.a()), _S("Agent Ava"));
		ImGui::PopFont();
	}

	// render health
	Color aHealthBar = Color(Settings->m_aHealthBar);
	if (Settings->m_RenderHealthBar)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 689 + 110, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 693 + 110, vecWindowPosition.y + 385), ImColor(0, 0, 0, 100));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 690 + 110, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 692 + 110, vecWindowPosition.y + 385), ImColor(aHealthBar.r(), aHealthBar.g(), aHealthBar.b(), aHealthBar.a()));
	}

	Color aHealthText = Color(Settings->m_aHealthText);
	if (Settings->m_RenderHealthText)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);

		if (Settings->m_RenderHealthBar)
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 110 - ImGui::CalcTextSize(_S("100")).x, vecWindowPosition.y + 37), ImColor(aHealthText.r(), aHealthText.g(), aHealthText.b(), aHealthText.a()), _S("100"));
		else
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 691 + 110 - ImGui::CalcTextSize(_S("100")).x, vecWindowPosition.y + 37), ImColor(aHealthText.r(), aHealthText.g(), aHealthText.b(), aHealthText.a()), _S("100"));

		ImGui::PopFont();
	}

	Color aWeaponText = Color(Settings->m_aWeaponText);
	if (Settings->m_RenderWeaponText)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 110 + 85 - ImGui::CalcTextSize(_S("P2000")).x / 2, vecWindowPosition.y + 385 + 6), ImColor(aWeaponText.r(), aWeaponText.g(), aWeaponText.b(), aWeaponText.a()), _S("P2000"));
		ImGui::PopFont();
	}

	Color aWeaponIcon = Color(Settings->m_aWeaponIcon);
	if (Settings->m_RenderWeaponIcon)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_WeaponIcon);

		if (Settings->m_RenderWeaponText)
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 110 + 85 - ImGui::CalcTextSize(_S("E")).x / 2, vecWindowPosition.y + 385 + 22), ImColor(aWeaponIcon.r(), aWeaponIcon.g(), aWeaponIcon.b(), aWeaponIcon.a()), _S("E"));
		else
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 110 + 85 - ImGui::CalcTextSize(_S("E")).x / 2, vecWindowPosition.y + 385 + 8), ImColor(aWeaponIcon.r(), aWeaponIcon.g(), aWeaponIcon.b(), aWeaponIcon.a()), _S("E"));

		ImGui::PopFont();
	}

	Color aAmmoBar = Color(Settings->m_aAmmoBar);
	if (Settings->m_RenderAmmoBar)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 110, vecWindowPosition.y + 387), ImVec2(vecWindowPosition.x + 906 + 110, vecWindowPosition.y + 391), ImColor(0, 0, 0, 100));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 110, vecWindowPosition.y + 388), ImVec2(vecWindowPosition.x + 905 + 110, vecWindowPosition.y + 390), ImColor(aAmmoBar.r(), aAmmoBar.g(), aAmmoBar.b(), aAmmoBar.a()));
	}

	Color aAmmoText = Color(Settings->m_aAmmoBarText);
	if (Settings->m_RenderAmmoBarText)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 902 + 110 + ImGui::CalcTextSize(_S("13")).x / 2, vecWindowPosition.y + 386), ImColor(aAmmoText.r(), aAmmoText.g(), aAmmoText.b(), aAmmoText.a()), _S("13"));
		ImGui::PopFont();
	}

	int iyOffset = 0;

	if (Settings->m_Flags[0])
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 902 + 115, vecWindowPosition.y + 37), ImColor(255, 255, 255), _S("SCOPED"));
		ImGui::PopFont();
		iyOffset += 12;
	}

	if (Settings->m_Flags[1])
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 902 + 115, vecWindowPosition.y + 37 + iyOffset), ImColor(82, 155, 255), _S("HK"));
		ImGui::PopFont();
		iyOffset += 12;
	}

	if (Settings->m_Flags[2])
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 902 + 115, vecWindowPosition.y + 37 + iyOffset), ImColor(218, 255, 82), _S("FLASHED"));
		ImGui::PopFont();
		iyOffset += 12;
	}

	if (Settings->m_Flags[3])
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 902 + 115, vecWindowPosition.y + 37 + iyOffset), ImColor(104, 211, 95), _S("$800"));
		ImGui::PopFont();
		iyOffset += 12;
	}

	if (Settings->m_Flags[4])
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 902 + 115, vecWindowPosition.y + 37 + iyOffset), ImColor(255, 0, 0, 255), _S("FD"));
		ImGui::PopFont();
		iyOffset += 12;
	}

	//ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 653 + 110, vecWindowPosition.y + 2), ImVec2(vecWindowPosition.x + 948 + 110, vecWindowPosition.y + 438), ImColor(10.0f / 255.0f, 12.0f / 255.0f, 14.0f / 255.0f, 1.0f));
	//ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 652 + 110, vecWindowPosition.y + 1), ImVec2(vecWindowPosition.x + 949 + 110, vecWindowPosition.y + 439), ImColor(45.0f / 255.0f, 47.0f / 255.0f, 54.0f / 255.0f, 1.0f));
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 654 + 110, vecWindowPosition.y), ImVec2(vecWindowPosition.x + 950 + 125, vecWindowPosition.y + 440), ImColor(60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f, 1.0f), 7.f);

}
void C_Menu::DrawWorldTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 50) / 2;
	int iChildDoubleSizeY = (565 - 45) / 2;

	int iChildSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 40 - 30);
	int iChildSizeY = (565 - 30);

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 30;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 10;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;
	ImGui::PopFont();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild(_S("View settings"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::Text(_S("Thirdperson"));
	ImGui::Keybind(_S("ThirdPerson Bind"), &g_Settings->m_aThirdPerson->m_iKeySelected, &g_Settings->m_aThirdPerson->m_iModeSelected); // note - AkatsukiSun: Crashing af.
	ImGui::SliderInt(_S("Distance"), g_Settings->m_iThirdPersonDistance, 50, 300);
	ImGui::SliderInt(_S("Camera distance"), g_Settings->m_iCameraDistance, 90, 140);
	ImGui::Checkbox(_S("Force distance while scoped"), g_Settings->m_bOverrideFOVWhileScoped);
	ImGui::SliderFloat(_S("Aspect ratio"), g_Settings->m_flAspectRatio, 0.01f, 3.0f, "%.1f");

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosSecondY));
	ImGui::BeginChild(_S("Viewmodel"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	ImGui::SliderInt(_S("Viewmodel distance"), g_Settings->m_iViewmodelDistance, 60, 140);
	ImGui::SliderInt(_S("Viewmodel X axis"), g_Settings->m_iViewmodelX, -10, 10);
	ImGui::SliderInt(_S("Viewmodel Y axis"), g_Settings->m_iViewmodelY, -10, 10);
	ImGui::SliderInt(_S("Viewmodel Z axis"), g_Settings->m_iViewmodelZ, -10, 10);
	ImGui::SliderInt(_S("Viewmodel roll"), g_Settings->m_iViewmodelRoll, -90, 90);

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosFirstY));
	ImGui::BeginChild(_S("World modulation"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	std::vector < const char* > aSkyboxList =
	{
		_S("None"),
		_S("Tibet"),
		_S("Baggage"),
		_S("Italy"),
		_S("Aztec"),
		_S("Vertigo"),
		_S("Daylight"),
		_S("Daylight 2"),
		_S("Clouds"),
		_S("Clouds 2"),
		_S("Gray"),
		_S("Clear"),
		_S("Canals"),
		_S("Cobblestone"),
		_S("Assault"),
		_S("Clouds dark"),
		_S("Night"),
		_S("Night 2"),
		_S("Night flat"),
		_S("Dusty"),
		_S("Rainy"),
		_S("Custom")
	};

	ImGui::Checkbox(_S("Hold fire animation"), g_Settings->m_bHoldFireAnimation);
	ImGui::Checkbox(_S("Preserve killfeed"), g_Settings->m_bPreserveKillfeed);
	ImGui::SingleSelect(_S("Skybox Changer"), g_Settings->m_iSkybox.GetPtr(), aSkyboxList);
	if (g_Settings->m_iSkybox == aSkyboxList.size() - 1)
	{
		static char aSkyBox[32];
		if (!g_Settings->m_szCustomSkybox->empty())
			strcpy(aSkyBox, g_Settings->m_szCustomSkybox.Get().c_str());

		if (ImGui::InputText(_S("##324234124"), aSkyBox, 32, ImGuiInputTextFlags_EnterReturnsTrue))
			g_Settings->m_szCustomSkybox.Get() = aSkyBox;
	}

	ImGui::Text(_S("World color"));
	this->DrawColorEdit4(_S("##123123"), &g_Settings->m_WorldModulation, 34);

	ImGui::Text(_S("Props color"));
	this->DrawColorEdit4(_S("##11233"), &g_Settings->m_PropModulation, 34);

	ImGui::Text(_S("Skybox color"));
	this->DrawColorEdit4(_S("##51223"), &g_Settings->m_SkyModulation, 34);

	ImGui::Spacing();

	ImGui::Checkbox(_S("Force crosshair"), g_Settings->m_bForceCrosshair);
	ImGui::Checkbox(_S("Hitmarker"), g_Settings->m_bHitMarker);
	ImGui::Checkbox(_S("Hitsound"), g_Settings->m_bHitSound);
	ImGui::Checkbox(_S("Damage marker"), g_Settings->m_bDamageMarker);

	ImGui::Checkbox(_S("Client bullet impacts"), g_Settings->m_bDrawClientImpacts);
	this->DrawColorEdit4(_S("##41242354"), &g_Settings->m_ClientImpacts);
	ImGui::Checkbox(_S("Server bullet impacts"), g_Settings->m_bDrawServerImpacts);
	this->DrawColorEdit4(_S("##412423154"), &g_Settings->m_ServerImpacts);

	ImGui::Checkbox(_S("Local bullet tracers"), g_Settings->m_bDrawLocalTracers);
	this->DrawColorEdit4(_S("##43242354"), &g_Settings->m_LocalTracers);
	ImGui::Checkbox(_S("Enemy bullet tracers"), g_Settings->m_bDrawEnemyTracers);
	this->DrawColorEdit4(_S("##432423154"), &g_Settings->m_EnemyTracers);

	std::vector < std::string > aWorldRemovals =
	{
		_S("Visual punch"),
		_S("Visual kick"),
		_S("Scope"),
		_S("Smoke"),
		_S("Flash"),
		_S("Post process"),
		_S("World FOG"),
		_S("Shadows"),
		_S("Landing bob"),
		_S("Hand shaking")
	};

	ImGui::Checkbox(_S("Grenade prediction"), g_Settings->m_bPredictGrenades);
	this->DrawColorEdit4(_S("##1234142124"), &g_Settings->m_GrenadeWarning);

	ImGui::Checkbox(_S("Grenade timers"), g_Settings->m_GrenadeTimers);
	this->DrawColorEdit4(_S("##1234145151"), &g_Settings->m_GrenadeWarningTimer);

	ImGui::MultiCombo(_S("World removals"), g_Settings->m_aWorldRemovals, m_aWorldRemovals, ARRAYSIZE(m_aWorldRemovals), preview);
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosSecondY));
	ImGui::BeginChild(_S("Weather"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));
	ImGui::Checkbox(_S("Enable weather"), g_Settings->m_CustomWeather);

	ImGui::SliderInt(_S("Rain length"), g_Settings->m_RainLength, 1, 100);
	ImGui::SliderInt(_S("Rain width"), g_Settings->m_RainWidth, 1, 100);
	ImGui::SliderInt(_S("Rain speed"), g_Settings->m_RainSpeed, 1, 2000);

	ImGui::EndChild();
}
void C_Menu::DrawMiscTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 50) / 2;
	int iChildDoubleSizeY = (565 - 45) / 2;

	int iChildSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 40 - 30);
	int iChildSizeY = (565 - 30);

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 30;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 10;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;
	ImGui::PopFont();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild(_S("Movement"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY - 70));

	ImGui::Checkbox(_S("Auto jump"), g_Settings->m_bBunnyHop);
	ImGui::Checkbox(_S("Auto strafe"), g_Settings->m_bAutoStrafe);
	ImGui::Checkbox(_S("WASD strafe"), g_Settings->m_bWASDStrafe);
	ImGui::Checkbox(_S("Start speed"), g_Settings->m_bSpeedBoost);
	ImGui::Checkbox(_S("Quick stop"), g_Settings->m_bFastStop);
	ImGui::Checkbox(_S("Edge jump"), g_Settings->m_bEdgeJump);
	ImGui::Checkbox(_S("Infinity duck"), g_Settings->m_bInfinityDuck);
	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosSecondY - 70));
	ImGui::BeginChild(_S("Other"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY + 70));

	ImGui::Checkbox(_S("Anti Untrusted"), g_Settings->m_bAntiUntrusted);

	ImGui::MultiCombo(_S("Event logs"), g_Settings->m_aLogs, m_aLogs, ARRAYSIZE(m_aLogs), preview);

	ImGui::Checkbox(_S("Filter server ads"), g_Settings->m_bAdBlock);
	ImGui::Checkbox(_S("Filter console"), g_Settings->m_bFilterConsole);
	ImGui::Checkbox(_S("Unlock convars"), g_Settings->m_bUnhideConvars);
	ImGui::Checkbox(_S("Reveal ranks"), g_Settings->m_bRevealRanks);
	ImGui::Checkbox(_S("Unlock inventory"), g_Settings->m_bUnlockInventoryAccess);
	ImGui::Checkbox(_S("Clantag"), g_Settings->m_bClantag);



	ImGui::Checkbox(_S("Enable BuyBot"), g_Settings->m_bBuyBotEnabled);

	std::vector < std::string > aEquipment
		=
	{
	_S("Fire grenade/Molotov"),
	_S("Smoke grenade"),
	_S("Flash grenade"),
	_S("Explosive grenade"),
	_S("Taser"),
	_S("Heavy armor"),
	_S("Helmet"),
	_S("Defuser")
	};

	ImGui::SingleSelect
	(
		_S("Primary"),
		g_Settings->m_BuyBotPrimaryWeapon,
		{
			_S("None"),
			_S("SCAR20/G3SG1"),
			_S("Scout"),
			_S("AWP"),
			_S("M4A1/AK47")
		}
	);
	ImGui::SingleSelect(
		_S("Secondary"),
		g_Settings->m_BuyBotSecondaryWeapon,
		{
			_S("None"),
			_S("FN57/TEC9"),
			_S("Dual elites"),
			_S("Deagle/Revolver"),
			_S("P2000/Glock-18"),
			_S("P250")
		}
	);

	ImGui::MultiCombo(_S("Equipment"), g_Settings->m_aEquipment, m_aEquipment, ARRAYSIZE(m_aEquipment), preview);
	ImGui::Checkbox(_S("Don't override AWP"), g_Settings->m_bBuyBotKeepAWP);
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosFirstY));
	ImGui::BeginChild(_S("Configs"), ImVec2(iChildDoubleSizeX, iChildSizeY));

	static int selected_config = 0;
	static auto should_update = true;

	ImGui::ListBoxHeader(_S("Config list"), ImVec2(iChildDoubleSizeX - 20, 200));

	for (int i = 0; i < g_Config->files.size(); i++)
	{
		std::string name_lua = g_Config->files.at(i);

		if (ImGui::Selectable(name_lua.c_str(), selected_config == i))
			selected_config = i;
	}

	ImGui::ListBoxFooter();
	std::vector <std::string> scripts;
	if (ImGui::CustomButton(_S("Load Config"), ImVec2(iChildDoubleSizeX - 20, 30)))
	{
		if (g_Config->files.empty())
			return;

		g_Config->load(g_Config->files.at(selected_config), false);

		c_lua::get().unload_all_scripts();

		for (auto& script : g_Config->scripts.scripts)
			c_lua::get().load_script(c_lua::get().get_script_id(script));

		scripts = c_lua::get().scripts;

		if (selected_script >= scripts.size() && scripts.size() > 1)
			selected_script = scripts.size() - 1; //-V103

		for (auto& current : scripts)
		{
			if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
				current.erase(current.size() - 5, 5);
			else if (current.size() >= 4)
				current.erase(current.size() - 4, 4);
		}

		g_Config->scripts.scripts.clear();

		g_Config->load(g_Config->files.at(selected_config), true);
		g_Config->config_files();
		g_LogManager->PushLog(_S("Config Loaded"), _S("A"));
	}

	static bool is_double = false;
	static bool fforce_save = false;
	static float time_since_sure = 0.f;

	if (time_since_sure > 15.f)
	{
		time_since_sure = 0.f;
		is_double = false;
	}

	if (is_double)
		time_since_sure += 0.1f;

	if (ImGui::CustomButton(is_double ? _S("Sure?") : _S("Save Config"), ImVec2(iChildDoubleSizeX - 20, 30)))
	{
		if (!is_double)
		{
			is_double = true;
		}
		else
		{
			if (!fforce_save)
			{
				fforce_save = true;
				is_double = false;
			}
		}
		if (fforce_save)
		{
			if (g_Config->files.empty())
				return;

			g_Config->scripts.scripts.clear();

			for (auto i = 0; i < c_lua::get().scripts.size(); i++)
			{
				auto script = c_lua::get().scripts.at(i);

				if (c_lua::get().loaded.at(i))
					g_Config->scripts.scripts.emplace_back(script);
			}

			g_Config->save(g_Config->files.at(selected_config));
			g_Config->config_files();

			g_LogManager->PushLog(_S("Config Saved"), _S("A"));
			fforce_save = false;
		}
	}
	if (should_update)
	{
		should_update = false;

		g_Config->config_files();
	}

	if (ImGui::CustomButton(_S("Refresh Config List"), ImVec2(iChildDoubleSizeX - 20, 30)))
		should_update = true;
	if (ImGui::CustomButton(_S("Open Config Folder"), ImVec2(iChildDoubleSizeX - 20, 30)))
		ShellExecute(NULL, "open", "selling.pw\\", NULL, NULL, SW_RESTORE);

	static bool creating_config = false;
	if (!creating_config)
		if (ImGui::CustomButton(_S("Create New Config"), ImVec2(iChildDoubleSizeX - 20, 30)))
			creating_config = true;

	if (creating_config)
	{
		static char config_name[64] = "\0";

		if (ImGui::CustomButton(_S("Create New Config"), ImVec2(iChildDoubleSizeX - 20, 30)))
			creating_config = true;
		if (ImGui::InputText(_S("Config Name"), config_name, sizeof(config_name), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string new_config_name = config_name;

			auto empty = true;

			for (auto current : new_config_name)
			{
				if (current != ' ')
				{
					empty = false;
					break;
				}
			}

			if (empty)
				new_config_name = _S("new config");

			//eventlogs::get().add(crypt_str("Added ") + ctx.m_settings.new_config_name + crypt_str(" config"), false);

			if (new_config_name.find(_S(".og")) == std::string::npos)
				new_config_name += _S(".og");

			g_Config->save(new_config_name);
			g_Config->config_files();

			g_Config->files = g_Config->files;
			selected_config = g_Config->files.size() - 1; //-V103

			creating_config = false;
		}
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			should_update = true;
			creating_config = false;
		}
	}

	ImGui::EndChild();
}
void C_Menu::DrawInventoryTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 50) / 2;
	int iChildDoubleSizeY = (565 - 45) / 2;

	int iChildSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 30);
	int iChildSizeY = (565 - 30);

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 30;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 10;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;
	ImGui::PopFont();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild(_S("Inventory"), ImVec2(iChildSizeX, iChildSizeY));
	static wskin weaponSkin;
	const int every_inventory_item = 4;
	int current_inventory_item = 1;
	static STATE_ID StateID;
	auto weaponName = zweaponnames(weaponSkin.wId);

	if (StateID == STATE_NONE)
	{
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		if (ImGui::CustomButton(_S("+"), ImVec2(137, 85)))
		{
			StateID = STATE_SELECTING_WEAPON;
		}
		ImGui::SameLine(0, 8);
		ImGui::PopFont();
		current_inventory_item = 2;

		for (auto weapon : g_InventorySkins)
		{
			auto texture = g_Inventory->GetTextureByBuiledePatch(weapon.second.build_patch);
			auto rarity = weapon.second.rarity;

			if ((weapon.second.wId >= 500 && weapon.second.wId <= 525) || weapon.second.wId == 59 || weapon.second.wId == 42)
				rarity = 6;

			if (ImGui::SkinCard(!texture, zweaponnames(weapon.second.wId), rarity, texture, ImVec2(137, 85)))
			{
				g_InventorySkins.erase(weapon.first);
				_inv.inventory.itemCount = g_InventorySkins.size();
				g_Globals.m_Interfaces.m_EngineClient->ExecuteClientCmd("econ_clear_inventory_images");
				write.SendClientHello();
				write.SendMatchmakingClient2GCHello();
			}

			if (current_inventory_item >= every_inventory_item) {
				current_inventory_item = 1;
			}
			else {
				current_inventory_item++;
				ImGui::SameLine();
			}
		}
	}
	if (StateID == STATE_SELECTING_WEAPON)
	{
		for (const auto& weapon : k_inventory_names)
		{
			auto texture = g_Inventory->GetTexture(weapon.name, nullptr);
			auto build_patch = g_Inventory->GetBuildPatch(weapon.name, nullptr);

			if (ImGui::SkinCard(!texture, weapon.name, weapon.rarity, texture, ImVec2(135, 85)))
			{
				if ((weapon.index >= 5100 && weapon.index <= 6000) || (weapon.index >= 4619 && weapon.index <= 4800 && weapon.index != 4725))// is_agent
				{
					if (texture)
					{
						StateID = STATE_CONFIGURING_WEAPON;
						weaponSkin.wId = weapon.index;
						weaponSkin.paintKit = 0;
						weaponSkin.build_patch = build_patch;
						weaponSkin.texture = texture;
						weaponSkin.rarity = weapon.rarity;
					}
				}
				else
				{
					if (texture)
					{
						weaponSkin.wId = weapon.index;
						weaponSkin.paintKit = 0;
						StateID = STATE_SELECTING_PAINTKIT;
					}
				}
			}

			if (current_inventory_item >= every_inventory_item) {
				current_inventory_item = 1;
			}
			else {
				current_inventory_item++;
				ImGui::SameLine();
			}

			if (g_Globals.m_KeyData.m_aHoldedKeys[VK_ESCAPE] & 1)
				StateID = STATE_NONE;
		}
	}
	if (StateID == STATE_SELECTING_PAINTKIT)
	{
		for (auto skin : _inv.inventory.skinInfo)
		{
			for (auto names : skin.second.weaponName)
			{
				if (weaponName != names)
					continue;

				auto texture = g_Inventory->GetTexture(names, &skin.second);
				auto build_patch = g_Inventory->GetBuildPatch(names, &skin.second);

				auto rarity = skin.second.rarity;

				if ((weaponSkin.wId >= 500 && weaponSkin.wId <= 525) || weaponSkin.wId == 59 || weaponSkin.wId == 42)
					rarity = 6;
				if (ImGui::SkinCard(!texture, names.c_str(), rarity, texture, ImVec2(135, 85)))
				{
					if (texture)
					{
						weaponSkin.paintKit = skin.first;
						weaponSkin.build_patch = build_patch;
						weaponSkin.texture = texture;
						weaponSkin.rarity = skin.second.rarity;
						StateID = STATE_CONFIGURING_WEAPON;
					}
				}

				if (current_inventory_item >= every_inventory_item) {
					current_inventory_item = 1;
				}
				else {
					current_inventory_item++;
					ImGui::SameLine();
				}

				if (g_Globals.m_KeyData.m_aHoldedKeys[VK_ESCAPE] & 1)
					StateID = STATE_NONE;
			}
		}
	}
	if (StateID == STATE_CONFIGURING_WEAPON)
	{
		static char skinname[64] = "\0";

		ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX + 20, iChildPosFirstY + 40));
		ImGui::BeginChild("Item Preview", ImVec2(iChildDoubleSizeX - 20, iChildSizeY - 60));

		auto rarity = weaponSkin.rarity;

		if ((weaponSkin.wId >= 500 && weaponSkin.wId <= 525) || weaponSkin.wId == 59 || weaponSkin.wId == 42)
			rarity = 6;

		ImGui::SkinCard(!weaponSkin.texture, zweaponnames(weaponSkin.wId), rarity, weaponSkin.texture, ImVec2(230, 164));

		ImGui::EndChild();

		ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX + 20 + iChildDoubleSizeX - 20 + 15, iChildPosFirstY + 40));
		ImGui::BeginChild("Item Settings", ImVec2(iChildDoubleSizeX - 20, iChildSizeY - 60));

		ImGui::InputInt("statrack", &weaponSkin.stattrak, 0);
		ImGui::InputFloat("wear", &weaponSkin.wear);
		ImGui::InputInt("seed", &weaponSkin.seed, 0);
		ImGui::InputText("name", skinname, sizeof(skinname), 0);

		if (g_Globals.m_KeyData.m_aHoldedKeys[VK_RETURN] & 1)
		{
			std::string str(skinname);
			if (str.length() > 0)
				weaponSkin.name = str;
			g_InventorySkins.insert({g_Tools->RandomInt(20000, 200000), weaponSkin });
			_inv.inventory.itemCount = g_InventorySkins.size();

			g_Globals.m_Interfaces.m_EngineClient->ExecuteClientCmd("econ_clear_inventory_images");
			write.SendClientHello();
			write.SendMatchmakingClient2GCHello();
			memset(&skinname[0], 0, sizeof(skinname));
			StateID = STATE_NONE;
		}
		if (g_Globals.m_KeyData.m_aHoldedKeys[VK_ESCAPE] & 1)
		{
			weaponSkin.paintKit = 0;
			weaponSkin.wId = 0;
			weaponSkin.texture = nullptr;
			weaponSkin.rarity = 0;
			StateID = STATE_NONE;
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}
void C_Menu::DrawLuaTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();
	auto wp = ImGui::GetWindowPos();
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 50) / 2;
	int iChildDoubleSizeY = (565 - 45) / 2;

	int iChildSizeX = (760 - ImGui::CalcTextSize(this->GetMenuName()).x - 40 - 30);
	int iChildSizeY = (565 - 30);

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 30;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 10;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;
	ImGui::PopFont();

	ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::BeginChild("Scripts", ImVec2(iChildDoubleSizeX, iChildSizeY));

	if (ImGui::CustomButton(_S("Refresh scripts"), ImVec2(iChildDoubleSizeX - 20, 30)))
		c_lua::get().refresh_scripts();
	if (ImGui::CustomButton(_S("Reload active scripts"), ImVec2(iChildDoubleSizeX - 20, 30)))
		c_lua::get().reload_all_scripts();


	if (c_lua::get().scripts.size())
	{
		ImGui::ListBoxHeader(_S("Scripts list"), ImVec2(iChildDoubleSizeX - 20, 200));

		for (int i = 0; i < c_lua::get().scripts.size(); i++)
		{
			std::string name_lua = c_lua::get().scripts.at(i);

			if (ImGui::Selectable(name_lua.c_str(), c_lua::get().loaded.at(c_lua::get().get_script_id(c_lua::get().scripts.at(i))) || selected_script == i))
				selected_script = i;
		}

		ImGui::ListBoxFooter();

		if (selected_script > 0 || c_lua::get().scripts.size() > 0)
		{
			if (c_lua::get().loaded.at(c_lua::get().get_script_id(c_lua::get().scripts.at(selected_script))))
			{
				if (ImGui::CustomButton(_S("Unload script"), ImVec2(iChildDoubleSizeX - 20, 30)))
					c_lua::get().unload_script(c_lua::get().get_script_id(c_lua::get().scripts.at(selected_script)));
			}
			else
			{
				if (ImGui::CustomButton(_S("Load script"), ImVec2(iChildDoubleSizeX - 20, 30)))
					c_lua::get().load_script(c_lua::get().get_script_id(c_lua::get().scripts.at(selected_script)));
			}
		}

		if (ImGui::CustomButton(_S("Unload all"), ImVec2(iChildDoubleSizeX - 20, 30)))
			c_lua::get().unload_all_scripts();
	}

	ImGui::EndChild();

	ImGui::SetNextWindowPos(ImVec2(iChildPosSecondX, iChildPosFirstY));
	ImGui::BeginChild(_S("Settings"), ImVec2(iChildDoubleSizeX, iChildSizeY));
	auto previous_check_box = false;
	if (c_lua::get().scripts.size())
	{
		if (selected_script >= c_lua::get().scripts.size() && c_lua::get().scripts.size() > 1)
			selected_script = c_lua::get().scripts.size() - 1;

		auto& items = c_lua::get().items.at(selected_script);

		for (auto& item : items)
		{
			std::string item_name;

			auto first_point = false;
			auto item_str = false;

			for (auto& c : item.first)
			{
				if (c == '.')
				{
					if (first_point)
					{
						item_str = true;
						continue;
					}
					else
						first_point = true;
				}

				if (item_str)
					item_name.push_back(c);
			}

			switch (item.second.type)
			{
			case NEXT_LINE:
				previous_check_box = false;
				break;
			case CHECK_BOX:
				previous_check_box = true;
				ImGui::Checkbox(item_name.c_str(), &item.second.check_box_value);
				break;
			case COMBO_BOX:
				previous_check_box = false;
				ImGui::SingleSelect(item_name.c_str(), &item.second.combo_box_value, item.second.combo_box_labels);
				break;
			case SLIDER_INT:
				previous_check_box = false;
				ImGui::SliderInt(item_name.c_str(), &item.second.slider_int_value, item.second.slider_int_min, item.second.slider_int_max);
				break;
			case SLIDER_FLOAT:
				previous_check_box = false;
				ImGui::SliderFloat(item_name.c_str(), &item.second.slider_float_value, item.second.slider_float_min, item.second.slider_float_max);
				break;
			case COLOR_PICKER:
				if (previous_check_box)
					previous_check_box = false;
				else
					ImGui::Text((item_name + ' ').c_str());


				this->DrawColorEdit4((("##") + item_name).c_str(), &item.second.color_picker_value, 35);

				//ImGui::ColorEdit4((("##") + item_name).c_str(), item.second.color_picker_value);
				break;

			case KEY_BIND:
				if (previous_check_box)
					previous_check_box = false;
				else
					ImGui::Text((item_name + ' ').c_str());

				ImGui::SameLine();
				ImGui::Keybind((("##") + item_name).c_str(), &item.second.keybind_key_value, &item.second.keybind_key_mode);
				break;
			}
		}
	}
	ImGui::EndChild();
}

void C_Menu::DrawColorEdit4(const char* strLabel, Color* aColor, int expand)
{
	float aColour[4] =
	{
		aColor->r() / 255.0f,
		aColor->g() / 255.0f,
		aColor->b() / 255.0f,
		aColor->a() / 255.0f
	};

	if (ImGui::ColorEdit4(strLabel, aColour, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_RGB, expand))
		aColor->SetColor(aColour[0], aColour[1], aColour[2], aColour[3]);
}

void C_Menu::Initialize()
{
	ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = ImVec4(45 / 255.f, 45 / 255.f, 45 / 255.f, 1.f);
	ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab] = ImVec4(65 / 255.f, 65 / 255.f, 65 / 255.f, 1.f);
	ImGui::GetStyle().AntiAliasedFill = true;
	ImGui::GetStyle().AntiAliasedLines = true;
	ImGui::GetStyle().ScrollbarSize = 6;

	D3DXCreateTextureFromFileInMemoryEx(g_Globals.m_Interfaces.m_DirectDevice, &avatarka, sizeof(avatarka), 512, 512, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &g_Menu->m_dTexture);
}

void C_Menu::WaterMark( )
{

}

#include "Tools/Tools.hpp"
#define PUSH_BIND( m_Variable, Name )\
if ( g_Tools->IsBindActive( m_Variable ) )\
{\
	if ( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent == 0.0f )\
		m_BindList[ FNV32( #m_Variable ) ].m_szName = _S( Name );\
	m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent = std::clamp( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent + ImGui::GetIO( ).DeltaTime * 10.0f, 0.0f, 1.0f );\
}\
else\
{\
	if ( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent == 0.0f )\
		m_BindList[ FNV32( #m_Variable ) ].m_szName = "";\
	m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent = std::clamp( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent - ImGui::GetIO( ).DeltaTime * 10.0f, 0.0f, 1.0f );\
}\

void C_Menu::DrawKeybindList()
{
	int m_Last = 0;
	PUSH_BIND(g_Settings->m_aFakeDuck, "Fake Duck");
	PUSH_BIND(g_Settings->m_aDoubleTap, "Double tap");
	PUSH_BIND(g_Settings->m_aSlowwalk, "Slow walk");
	PUSH_BIND(g_Settings->m_aHideShots, "Hide shots");
	PUSH_BIND(g_Settings->m_aSafePoint, "Safe points");
	PUSH_BIND(g_Settings->m_aInverter, "Invert side");
	PUSH_BIND(g_Settings->m_aAutoPeek, "Auto peek");
	PUSH_BIND(g_Settings->m_aMinDamage, "Damage override");

	int32_t iCount = 0;
	for (auto& Bind : m_BindList)
	{
		if (Bind.second.m_szName.length())
			iCount++;
	}

	if (iCount <= 0 && !g_Menu->IsMenuOpened())
		return;

	int nAdvancedFlag = 0;
	if (!m_bIsMenuOpened)
		nAdvancedFlag = ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowSize(ImVec2(190, m_BindList.empty() ? 0 : 35 + (27 * iCount)));
	ImGui::DefaultBegin(_S("Keybind List"), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | nAdvancedFlag);
	int32_t x = ImGui::GetCurrentWindow()->Pos.x + 4.5f;
	int32_t y = ImGui::GetCurrentWindow()->Pos.y;

	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + 181, y + 27), ImColor(36, 36, 36, 255), 5, ImDrawCornerFlags_Top);
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + 181, y + 27), ImColor(60, 60, 60, 255), 5, ImDrawCornerFlags_Top);
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x + 5, y + 23), ImVec2(x + 181 - 5, y + 24), ImColor(60, 60, 60, 255));

	ImGui::PushFont(g_Globals.m_Fonts.m_LogFont);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 5, y + 2), ImColor(255, 255, 255), _S("Keybind list"));
	ImGui::PopFont();

	for (auto& Bind : m_BindList)
	{
		if (!Bind.second.m_szName.length())
			continue;

		ImGui::PushFont(g_Globals.m_Fonts.m_LogFont);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 5, 32 + (y + 16 * m_Last)), ImColor(150, 150, 150, static_cast <int>(Bind.second.m_flAlphaPercent * 255.0f)), Bind.second.m_szName.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 160, 32 + (y + 16 * m_Last)), ImColor(150, 150, 150, static_cast <int>(Bind.second.m_flAlphaPercent * 255.0f)), _S("On"));
		ImGui::PopFont();

		m_Last++;
	}
	ImGui::DefaultEnd();
}