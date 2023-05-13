#pragma once
#include "../../SDK/Globals.hpp"
#include "../../SDK/Includes.hpp"

class C_Skins
{
public:
	virtual void Instance( ClientFrameStage_t Stage );
	virtual bool ApplyCustomSkin( C_BaseCombatWeapon* pWeapon, short nWeaponIndex );
	virtual void SetKnifeIndex(int index);
	int m_nOriginalModelIndex = 0;
	int m_nItemIndex = 0;
};

inline C_Skins* g_Skins = new C_Skins( );