#include "skins.h"
#include <codecvt>
#include <cassert>
#include <locale>
#include "inventorychanger.h"
#include "items.h"
#include <Psapi.h>
#include "../Tools/Tools.hpp"

static auto zis_knife(const int i) -> bool
{
	return (i >= 500 && i < 550) || i == 59 || i == 42;
}
static auto zis_glove(const int i) -> bool
{
	return (i >= 5027 && i <= 5035 || i == 4725); //hehe boys
}

static auto get_wearable_create_fn() -> CreateClientClassFn {
	auto client_class = g_Globals.m_Interfaces.m_CHLClient->GetAllClasses();

	for (client_class = g_Globals.m_Interfaces.m_CHLClient->GetAllClasses();
		client_class; client_class = client_class->m_pNext) {

		if (client_class->m_ClassID == (int)ClassId_CEconWearable) {
			return client_class->m_pCreateFn;
		}
	}
}

void C_Skins::SetKnifeIndex(int index)
{
	m_nItemIndex = index;
}

void C_Skins::Instance( ClientFrameStage_t Stage )
{
	if ( !g_Globals.m_LocalPlayer->IsAlive( ) || Stage != ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START )
		return;

	if (!g_Globals.m_Interfaces.m_EngineClient->IsInGame())
		return;

	m_nOriginalModelIndex = -1;
	for ( auto& aWeapon : g_InventorySkins )
	{
		if ( ( !aWeapon.second.m_bInUseCT && !aWeapon.second.m_bInUseT ) )
		{
			m_nOriginalModelIndex = g_Globals.m_LocalPlayer->m_nModelIndex( );
			continue;
		}

		if ( !aWeapon.second.m_bInUseT && g_Globals.m_LocalPlayer->m_iTeamNum( ) == 2 )
		{
			m_nOriginalModelIndex = g_Globals.m_LocalPlayer->m_nModelIndex();
			continue;
		}

		if ( !aWeapon.second.m_bInUseCT && g_Globals.m_LocalPlayer->m_iTeamNum( ) == 3 )
		{
			m_nOriginalModelIndex = g_Globals.m_LocalPlayer->m_nModelIndex( );
			continue;
		}

		if ( !( ( aWeapon.second.wId >= 5100 && aWeapon.second.wId <= 6000 ) || aWeapon.second.wId >= 4619 && aWeapon.second.wId <= 4800 ) )
			continue;
	
		if ( aWeapon.second.m_bInUseT && g_Globals.m_LocalPlayer->m_iTeamNum() == 2 )
		{
			int32_t iModelIndex = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex( k_weapon_info.at( aWeapon.second.wId ).model );
			if ( iModelIndex != -1 )
				g_Globals.m_LocalPlayer->SetModelIndex( iModelIndex );
			
			continue;
		}

		if ( !aWeapon.second.m_bInUseCT || g_Globals.m_LocalPlayer->m_iTeamNum( ) != 3 )
			continue;

		int32_t iModelIndex = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex( k_weapon_info.at( aWeapon.second.wId ).model );
		if ( iModelIndex != -1 )
			g_Globals.m_LocalPlayer->SetModelIndex( iModelIndex );
	}
	C_PlayerInfo player_info;
	if (!g_Globals.m_Interfaces.m_EngineClient->GetPlayerInfo(g_Globals.m_LocalPlayer->EntIndex(), &player_info))
		return;

	const auto m_LocalWeapons = g_Globals.m_LocalPlayer->weapons();
	if (!m_LocalWeapons)
		return;

	const auto m_LocalWearables = g_Globals.m_LocalPlayer->m_hMyWearables();
	if (!m_LocalWearables)
		return;

	static auto glove_handle = CBaseHandle(0);

	auto glove = reinterpret_cast<C_BaseAttributableItem*>(g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(m_LocalWearables[0]));
	if (!glove) // There is no glove
	{
		const auto our_glove = reinterpret_cast<C_BaseAttributableItem*>(g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(glove_handle));

		if (our_glove) // Try to get our last created glove
		{
			m_LocalWearables[0] = glove_handle;
			glove = our_glove;
		}
	}
	if (!g_Globals.m_LocalPlayer ||!g_Globals.m_LocalPlayer->IsAlive())
	{
		if (glove) {
			glove->SetDestroyedOnRecreateEntities();
			glove->Release();
		}
		return;
	}
	if (!glove) // We don't have a glove, but we should
	{
		static auto create_wearable_fn = get_wearable_create_fn();
		const auto entry = g_Globals.m_Interfaces.m_EntityList->GetHighestEntityIndex() + 1;
		const auto serial = rand() % 0x1000;

		create_wearable_fn(entry, serial);
		glove = reinterpret_cast<C_BaseAttributableItem*>(g_Globals.m_Interfaces.m_EntityList->GetClientEntity(entry));
		glove->SetAbsoluteOrigin({ 10000.f, 10000.f, 10000.f });
		const auto wearable_handle = reinterpret_cast<CBaseHandle*>(&m_LocalWearables[0]);
		*wearable_handle = entry | serial << 16;
		glove_handle = m_LocalWearables[0];
	}

	for (auto& wglove : g_InventorySkins)
	{
		if (!zis_glove(wglove.second.wId))
			continue;

		if (!wglove.second.m_bInUseT && g_Globals.m_LocalPlayer->m_iTeamNum() == 2)
			continue;

		if (!wglove.second.m_bInUseCT && g_Globals.m_LocalPlayer->m_iTeamNum() == 3)
			continue;

		if (glove)
		{
			glove->m_iItemDefinitionIndex() = wglove.second.wId;
			glove->m_iItemIDHigh() = -1;
			glove->m_iEntityQuality() = 4;
			glove->m_iAccountID() = player_info.m_iSteamID64;
			glove->m_nFallbackSeed() = wglove.second.seed;
			glove->m_nFallbackStatTrak() = -1;
			glove->m_flFallbackWear() = wglove.second.wear;
			glove->m_nFallbackPaintKit() = wglove.second.paintKit;
			const auto& replacement_item = k_weapon_info.at(glove->m_iItemDefinitionIndex());
			glove->SetModelIndex(g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex(replacement_item.model) + 2 /*enjoy nopaste things*/);


			static auto fnEquip = reinterpret_cast<int(__thiscall*)(void*, void*)>(g_Tools->FindPattern(g_Globals.m_ModuleList.m_ClientDll, _S("55 8B EC 83 EC 10 53 8B 5D 08 57 8B F9")));
			fnEquip(glove, g_Globals.m_LocalPlayer);
			g_Globals.m_LocalPlayer->m_nBody() = 1;

			//g_pIClientLeafSystem->CreateRenderableHandle(glove); //render our glove in 3th person mode

			glove->PreDataUpdate(0);
		}
	}

	for (auto i = 0; m_LocalWeapons[i] != 0xFFFFFFFF; i++)
	{
		auto weapon = reinterpret_cast<C_BaseCombatWeapon*>(g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(m_LocalWeapons[i]));
		if (!weapon)
			continue;

		const auto weaponIndex = weapon->m_iItemDefinitionIndex();
		ApplyCustomSkin(weapon, weaponIndex);
		weapon->m_iAccountID() = player_info.m_iSteamID64;
	}
}

bool C_Skins::ApplyCustomSkin( C_BaseCombatWeapon* pWeapon, short nWeaponIndex )
{
	switch ( nWeaponIndex )
	{
		case WEAPON_KNIFE_BAYONET:
		case WEAPON_KNIFE_BUTTERFLY:
		case WEAPON_KNIFE_FALCHION:
		case WEAPON_KNIFE_FLIP:
		case WEAPON_KNIFE_GUT:
		case WEAPON_KNIFE_GYPSY_JACKKNIFE:
		case WEAPON_KNIFE_KARAMBIT:
		case WEAPON_KNIFE_M9_BAYONET:
		case WEAPON_KNIFE_PUSH:
		case WEAPON_KNIFE_STILETTO:
		case WEAPON_KNIFE_SURVIVAL_BOWIE:
		case WEAPON_KNIFE_TACTICAL:
		case WEAPON_KNIFE_URSUS:
		case WEAPON_KNIFE_WIDOWMAKER:
		case WEAPON_KNIFE_SKELETON:
		case WEAPON_KNIFE_OUTDOOR:
		case WEAPON_KNIFE_CANIS:
		case WEAPON_KNIFE_CORD:
		case WEAPON_KNIFE_CSS:
		case WEAPON_KNIFE_T:
		case WEAPON_KNIFE:
			nWeaponIndex = WEAPON_KNIFE;
	}

	for ( auto& aWeapon : g_InventorySkins )
	{
		if ( ( !aWeapon.second.m_bInUseCT && !aWeapon.second.m_bInUseT))
			continue;

		if ( aWeapon.second.wId != nWeaponIndex)
			continue;

		if ( !aWeapon.second.m_bInUseT && g_Globals.m_LocalPlayer->m_iTeamNum( ) == 2 )
			continue;

		if ( !aWeapon.second.m_bInUseCT && g_Globals.m_LocalPlayer->m_iTeamNum( ) == 3 )
			continue;

		pWeapon->m_nFallbackPaintKit( ) = aWeapon.second.paintKit;
		pWeapon->m_iEntityQuality( ) = aWeapon.second.quality;
		pWeapon->m_nFallbackSeed( ) = aWeapon.second.seed;
		pWeapon->m_nFallbackStatTrak( ) = aWeapon.second.stattrak;
		pWeapon->m_flFallbackWear( ) = fmax( aWeapon.second.wear, 0.00001f );

		pWeapon->m_iItemIDHigh() = -1;
		if ( aWeapon.second.name.length( ) )
			strcpy( &pWeapon->m_iCustomName( ), aWeapon.second.name.c_str( ) );
	}

	auto IsKnife = [ ]( const int Index ) -> bool
	{
		return ( Index >= 500 && Index < 550) || Index == 59 || Index == 42;
	};

	auto IsGlove = [ ]( const int Index ) -> bool
	{
		return ( Index >= 5027 && Index <= 5035 || Index == 4725 );
	};

	for ( auto& aWeapon : g_InventorySkins )
	{
		if ( ( !aWeapon.second.m_bInUseCT && !aWeapon.second.m_bInUseT ) )
			continue;

		if ( IsKnife( aWeapon.second.wId ) && IsKnife( nWeaponIndex ) )
		{
			if ( !aWeapon.second.m_bInUseT && g_Globals.m_LocalPlayer->m_iTeamNum( ) == 2 )
				continue;

			if ( !aWeapon.second.m_bInUseCT && g_Globals.m_LocalPlayer->m_iTeamNum( ) == 3 )
				continue;

			pWeapon->m_nFallbackPaintKit( ) = aWeapon.second.paintKit;
			pWeapon->m_iEntityQuality( ) = aWeapon.second.quality;
			pWeapon->m_nFallbackSeed( ) = aWeapon.second.seed;
			pWeapon->m_nFallbackStatTrak( ) = aWeapon.second.stattrak;
			pWeapon->m_flFallbackWear( ) = fmax( aWeapon.second.wear, 0.00001f );

			pWeapon->m_iItemIDHigh() = -1;
			if ( aWeapon.second.name.length( ) )
				strcpy( &pWeapon->m_iCustomName( ), aWeapon.second.name.c_str( ) );

			if ( aWeapon.second.wId == WEAPON_NONE || !IsKnife( aWeapon.second.wId ) )
				continue;

			pWeapon->m_iItemDefinitionIndex( ) = aWeapon.second.wId;
			pWeapon->m_iEntityQuality( ) = 3;

			C_BaseEntity* pWorldModel = ( C_BaseEntity* )( pWeapon->m_hWeaponWorldModel( ).Get( ) );
			if ( !pWorldModel )
				return false;

			int32_t iModelIndex = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex( k_weapon_info.at( pWeapon->m_iItemDefinitionIndex( ) ).model );
	
			pWeapon->SetModelIndex( iModelIndex );
			SetKnifeIndex( iModelIndex );
			pWorldModel->SetModelIndex( iModelIndex + 1 );
		}
	}

	return true;
}
