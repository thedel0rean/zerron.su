#include "../Hooks.hpp"
#include "../Features/Packet/PacketManager.hpp"
#include "../Features/Model/Model.hpp"
#include "../Features/Exploits/Exploits.hpp"
#include "../Features/Networking/Networking.hpp"
#include "../SDK/Math/Math.hpp"
#include "../Features/Inventory/protobuf/Protobuffs.h"
#include "../Features/Inventory/items.h"
#include "../Features/Inventory/steam/steam_api.h"
#include "../Features/Inventory/skins.h"

void __cdecl C_Hooks::hkCL_Move(float_t flFrametime, bool bIsFinalTick)
{
	g_Networking->UpdateLatency();
	g_PacketManager->SaveSequence();

	if (!g_ExploitSystem->IsAllowedToRun())
		return;

	g_Globals.m_Hooks.m_Originals.m_CL_Move(flFrametime, g_ExploitSystem->IsFinalTick());

	g_ExploitSystem->PerformPackets();
	return g_PacketManager->FinishNetwork();
}

bool __fastcall C_Hooks::hkDispatchUserMessage(LPVOID pEcx, uint32_t, int32_t iMessageType, int32_t iArgument, int32_t iSecondArgument, LPVOID pData)
{
	if (iMessageType == CS_UM_TextMsg || iMessageType == CS_UM_HudMsg || iMessageType == CS_UM_SayText)
		if (g_Settings->m_bAdBlock && !(*(g_Globals.m_Interfaces.m_GameRules))->IsValveDS())
			return true;

	return g_Globals.m_Hooks.m_Originals.m_DispatchUserMessage(pEcx, iMessageType, iArgument, iSecondArgument, pData);
}

void __fastcall C_Hooks::hkPerformScreenOverlay(LPVOID pEcx, uint32_t, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight)
{
	if (!g_Settings->m_bAdBlock || (*(g_Globals.m_Interfaces.m_GameRules))->IsValveDS())
		return g_Globals.m_Hooks.m_Originals.m_PerformScreenOverlay(pEcx, x, y, iWidth, iHeight);
}

void __cdecl C_Hooks::hkShouldDrawFOG()
{
	if (!g_Settings->m_aWorldRemovals[REMOVALS_VISUAL_FOG])
		return g_Globals.m_Hooks.m_Originals.m_ShouldDrawFog();
}

void C_Hooks::hkFlashDuration(const CRecvProxyData* Data, LPVOID pStruct, LPVOID pOut)
{
	if (g_Settings->m_aWorldRemovals[REMOVALS_VISUAL_FLASH])
	{
		*(float_t*)(pOut) = 0.0f;
		return;
	}

	return g_Globals.m_Hooks.m_Originals.m_FlashDuration->GetOriginal()(Data, pStruct, pOut);
}
void C_Hooks::m_flAbsYawHook(const CRecvProxyData* pData, LPVOID pStruct, LPVOID pOut)
{
	static auto m_hPlayer = g_NetvarManager->GetNetvar(FNV32("DT_CSRagdoll"), FNV32("m_hPlayer"));
	const auto player_handle = reinterpret_cast<CBaseHandle*>(reinterpret_cast<DWORD>(pStruct) + m_hPlayer);
	const auto abs_yaw = Math::NormalizeAngle(pData->m_Value.m_Float);

	if (*player_handle != 0xFFFFFFFF && *player_handle != -1 && g_Globals.m_LocalPlayer != nullptr)
	{
		C_BasePlayer* hplayer = dynamic_cast<C_BasePlayer*>(g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(*player_handle));

		if (hplayer && hplayer->GetC_ClientEntity() != nullptr)
		{
			auto* player = (C_BasePlayer*)hplayer->GetC_ClientEntity()->GetBaseEntity();
			if (player != nullptr && player->EntIndex() > 0 && player->EntIndex() < 64 && player->EntIndex() != g_Globals.m_LocalPlayer->EntIndex() && player->m_iTeamNum() != g_Globals.m_LocalPlayer->m_iTeamNum())
			{
				const auto delta = Math::AngleDiff(abs_yaw, player->m_angEyeAngles().yaw);

				if (abs(delta) > 0.f && abs(delta) < 65.f) {
					g_Globals.m_ResolverData.m_ResolverDelta[player->EntIndex()] = delta;
				}
			}
		}
	}

	return g_Globals.m_Hooks.m_Originals.m_AbsAngles->GetOriginal()(pData, pStruct, pOut);
}
int GetNewAnimation(const uint32_t model, const int sequence, C_BaseViewModel* viewModel) {

	// This only fixes if the original knife was a default knife.
	// The best would be having a function that converts original knife's sequence
	// into some generic enum, then another function that generates a sequence
	// from the sequences of the new knife. I won't write that.
	enum ESequence {
		SEQUENCE_DEFAULT_DRAW = 0,
		SEQUENCE_DEFAULT_IDLE1 = 1,
		SEQUENCE_DEFAULT_IDLE2 = 2,
		SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
		SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
		SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
		SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
		SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
		SEQUENCE_DEFAULT_LOOKAT01 = 12,

		SEQUENCE_BUTTERFLY_DRAW = 0,
		SEQUENCE_BUTTERFLY_DRAW2 = 1,
		SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
		SEQUENCE_BUTTERFLY_LOOKAT03 = 15,

		SEQUENCE_FALCHION_IDLE1 = 1,
		SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
		SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
		SEQUENCE_FALCHION_LOOKAT01 = 12,
		SEQUENCE_FALCHION_LOOKAT02 = 13,

		SEQUENCE_DAGGERS_IDLE1 = 1,
		SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
		SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
		SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
		SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,

		SEQUENCE_BOWIE_IDLE1 = 1,
	};

	auto random_sequence = [](const int low, const int high) -> int {
		return rand() % (high - low + 1) + low;
	};

	// Hashes for best performance.
	switch (model) {
	case hash_32_fnv1a_const(("models/weapons/v_knife_butterfly.mdl")):
	{
		switch (sequence) {
		case SEQUENCE_DEFAULT_DRAW:
			return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03);
		default:
			return sequence + 1;
		}
	}
	case hash_32_fnv1a_const(("models/weapons/v_knife_falchion_advanced.mdl")):
	{
		switch (sequence) {
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_FALCHION_IDLE1;
		case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return random_sequence(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02);
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
		default:
			return sequence - 1;
		}
	}
	case hash_32_fnv1a_const(("models/weapons/v_knife_push.mdl")):
	{
		switch (sequence) {
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_DAGGERS_IDLE1;
		case SEQUENCE_DEFAULT_LIGHT_MISS1:
		case SEQUENCE_DEFAULT_LIGHT_MISS2:
			return random_sequence(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5);
		case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return random_sequence(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1);
		case SEQUENCE_DEFAULT_HEAVY_HIT1:
		case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
		case SEQUENCE_DEFAULT_LOOKAT01:
			return sequence + 3;
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
		default:
			return sequence + 2;
		}
	}
	case hash_32_fnv1a_const(("models/weapons/v_knife_survival_bowie.mdl")):
	{
		switch (sequence) {
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_BOWIE_IDLE1;
		default:
			return sequence - 1;
		}
	}
	case hash_32_fnv1a_const(("models/weapons/v_knife_ursus.mdl")):
	case hash_32_fnv1a_const(("models/weapons/v_knife_skeleton.mdl")):
	case hash_32_fnv1a_const(("models/weapons/v_knife_outdoor.mdl")):
	case hash_32_fnv1a_const(("models/weapons/v_knife_canis.mdl")):
	case hash_32_fnv1a_const(("models/weapons/v_knife_cord.mdl")):
	{
		switch (sequence) {
		case SEQUENCE_DEFAULT_DRAW:
			return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
		default:
			return sequence + 1;
		}
	}
	case hash_32_fnv1a_const(("models/weapons/v_knife_stiletto.mdl")):
	{
		switch (sequence) {
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(12, 13);
		}
	}
	case hash_32_fnv1a_const(("models/weapons/v_knife_widowmaker.mdl")):
	{
		switch (sequence) {
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(14, 15);
		}
	}

	default:
		return sequence;
	}
}
void DoSequenceRemapping(CRecvProxyData* data, C_BaseViewModel* entity) {

	if (!g_Globals.m_Interfaces.m_EngineClient->IsInGame() || !g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive())
		return;

	auto* const owner = g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle((CBaseHandle)entity->m_hOwner());
	if (owner != g_Globals.m_LocalPlayer || !entity->m_hWeapon())
		return;

	auto* const view_model_weapon = (C_BaseCombatWeapon*)g_Globals.m_Interfaces.m_EntityList->GetClientEntityFromHandle(entity->m_hWeapon());

	if (!view_model_weapon || !view_model_weapon->IsKnife())
		return;

	const auto entry = k_weapon_info.find(view_model_weapon->m_iItemDefinitionIndex());

	if (entry == k_weapon_info.end())
		return;

	if (&entry->second == nullptr)
		return;

	const auto weaponInfo = &entry->second;
	auto& sequence = data->m_Value.m_Int;
	sequence = GetNewAnimation(hash_32_fnv1a_const(weaponInfo->model), sequence, entity);
}

void C_Hooks::m_nViewModel(const CRecvProxyData* pData, LPVOID pStruct, LPVOID pOut)
{
	int default_t = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_default_t.mdl");
	int default_ct = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
	int iBayonet = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
	int iButterfly = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
	int iFlip = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
	int iGut = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
	int iKarambit = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
	int iM9Bayonet = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
	int iHuntsman = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
	int iFalchion = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
	int iDagger = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
	int iBowie = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
	int iGunGame = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_gg.mdl");
	int Navaja = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_gypsy_jackknife.mdl");
	int Stiletto = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_stiletto.mdl");
	int Ursus = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_ursus.mdl");
	int Talon = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_widowmaker.mdl");
	int d1 = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_css.mdl");
	int d2 = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_skeleton.mdl");
	int d3 = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_outdoor.mdl");
	int d4 = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_canis.mdl");
	int d5 = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex("models/weapons/v_knife_cord.mdl");

	const auto local = g_Globals.m_LocalPlayer;

	if (local)
	{
		if (local->IsAlive() && (
			pData->m_Value.m_Int == default_t ||
			pData->m_Value.m_Int == default_ct ||
			pData->m_Value.m_Int == iBayonet ||
			pData->m_Value.m_Int == iFlip ||
			pData->m_Value.m_Int == iGunGame ||
			pData->m_Value.m_Int == iGut ||
			pData->m_Value.m_Int == iKarambit ||
			pData->m_Value.m_Int == iM9Bayonet ||
			pData->m_Value.m_Int == iHuntsman ||
			pData->m_Value.m_Int == iBowie ||
			pData->m_Value.m_Int == iButterfly ||
			pData->m_Value.m_Int == iFalchion ||
			pData->m_Value.m_Int == iDagger ||
			pData->m_Value.m_Int == Navaja ||
			pData->m_Value.m_Int == Stiletto ||
			pData->m_Value.m_Int == Ursus ||
			pData->m_Value.m_Int == Talon ||
			pData->m_Value.m_Int == d1 ||
			pData->m_Value.m_Int == d2 ||
			pData->m_Value.m_Int == d3 ||
			pData->m_Value.m_Int == d4 ||
			pData->m_Value.m_Int == d5))
		{
			auto* const proxy_data = const_cast<CRecvProxyData*>(pData);
			if (g_Skins->m_nItemIndex)
				proxy_data->m_Value.m_Int = g_Skins->m_nItemIndex;
		}
	}

	return g_Globals.m_Hooks.m_Originals.m_ModelIndex->GetOriginal()(pData, pStruct, pOut);
}
void C_Hooks::m_nSequence(const CRecvProxyData* Data, LPVOID pStruct, LPVOID pOut) {
	// Remove the constness from the proxy data allowing us to make changes.
	auto* const proxy_data = const_cast<CRecvProxyData*>(Data);

	auto* const view_model = static_cast<C_BaseViewModel*>(pStruct);

	DoSequenceRemapping(proxy_data, view_model);

	return g_Globals.m_Hooks.m_Originals.m_Sequence->GetOriginal()(Data, pStruct, pOut);
}

bool __cdecl C_Hooks::hkHost_ShouldRun()
{
	return true;
}
unsigned long __fastcall C_Hooks::GCRetrieveMessageHook(void* ecx, void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{
	auto status = g_Globals.m_Hooks.m_Originals.m_GCRetrieveMessage(ecx, punMsgType, pubDest, cubDest, pcubMsgSize);
	if (status == k_EGCResultOK)
	{

		void* thisPtr = nullptr;
		__asm mov thisPtr, ebx;
		auto oldEBP = *reinterpret_cast<void**>((uint32_t)_AddressOfReturnAddress() - 4);

		uint32_t messageType = *punMsgType & 0x7FFFFFFF;
		write.ReceiveMessage(thisPtr, oldEBP, messageType, pubDest, cubDest, pcubMsgSize);
	}
	return status;
}
unsigned long __fastcall C_Hooks::GCSendMessageHook(void* ecx, void*, uint32_t unMsgType, const void* pubData, uint32_t cubData)
{
	bool sendMessage = write.PreSendMessage(unMsgType, const_cast<void*>(pubData), cubData);
	if (!sendMessage)
		return k_EGCResultOK;
	return g_Globals.m_Hooks.m_Originals.m_GCSendMessage(ecx, unMsgType, const_cast<void*>(pubData), cubData);
}