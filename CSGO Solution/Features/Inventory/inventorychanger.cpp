#include "inventorychanger.h"
#include "parser.h"
#include <set>
#include "protobuf/Protobuffs.h"
#include "protobuf/ProtoWriter.h"
#include "protobuf/Messages.h"
#include "steam/steam_api.h"
#include "../../SDK/Utils/Enums.hpp"
#include "../../SDK/Globals.hpp"
#include <Psapi.h>
#include <d3dx9.h>
#pragma comment (lib, "d3dx9.lib")

#include "../Tools/Obfuscation/XorStr.hpp"

inline static auto buildTexturePath = [](std::string weapon, skinInfo* item) -> std::string {
	if (item)
	{
		std::string name = _S( "resource/flash/econ/default_generated/" );
		if (strstr(weapon.c_str(), _S("gloves")) || strstr(weapon.c_str(), _S("leather_handwraps")))
		{
			if (strstr(weapon.c_str(), _S("leather_handwraps")))
				name += _S("leather_handwraps");
			else
				name += weapon;
		}
		else
		{
			name += _S("weapon_");
			name += weapon;
		}

		name += "_";
		name += std::string(item->cdnName);	
		name += _S("_light_large.png");
		return name;
	}
	else
	{
		if (strstr(weapon.c_str(), "customplayer"))
			return _S("resource/flash/econ/characters/") + weapon + _S(".png");

		else if (strstr(weapon.c_str(), _S("studded_bloodhound_gloves")))
			return _S("resource/flash/econ/default_generated/studded_bloodhound_gloves_bloodhound_guerrilla_light_large.png");

		else if (strstr(weapon.c_str(), _S("sporty_gloves")))
			return _S("resource/flash/econ/default_generated/sporty_gloves_sporty_light_blue_light_large.png");

		else if (strstr(weapon.c_str(), _S("slick_gloves")))
			return _S("resource/flash/econ/default_generated/slick_gloves_slick_plaid_purple_light_large.png");

		else if (strstr(weapon.c_str(), _S("leather_handwraps")))
			return _S("resource/flash/econ/default_generated/leather_handwraps_handwrap_fabric_orange_camo_light_large.png");

		else if (strstr(weapon.c_str(), _S("motorcycle_gloves")))
			return _S("resource/flash/econ/default_generated/motorcycle_gloves_motorcycle_triangle_blue_light_large.png");

		else if (strstr(weapon.c_str(), _S("specialist_gloves")))
			return _S("resource/flash/econ/default_generated/specialist_gloves_specialist_kimono_diamonds_red_light_large.png");

		else if (strstr(weapon.c_str(), _S("studded_hydra_gloves")))
			return _S("resource/flash/econ/default_generated/studded_hydra_gloves_operation_10_metalic_green_light_large.png");

		else
			return _S("resource/flash/econ/weapons/base_weapons/") + weapon + _S(".png");
	}
};
#define ARRAY_SEARCH(array, elem) (std::find(array.begin(), array.end(), elem) != array.end())

LPDIRECT3DTEXTURE9 Inventory::GetTexture(std::string weapon, skinInfo* item) {
	auto texture_path = buildTexturePath(weapon, item);
	LPDIRECT3DTEXTURE9 ret = Textures[texture_path];
	if (ret)
		return ret;

	if (!ARRAY_SEARCH(Errors, texture_path) && !ARRAY_SEARCH(Queue, texture_path))
		Queue.emplace_back(texture_path);

	return nullptr;
}

LPDIRECT3DTEXTURE9 Inventory::GetTextureByBuiledePatch(std::string path) {
	auto texture_path = path;
	LPDIRECT3DTEXTURE9 ret = Textures[texture_path];
	if (ret)
		return ret;

	if (!ARRAY_SEARCH(Errors, texture_path) && !ARRAY_SEARCH(Queue, texture_path))
		Queue.emplace_back(texture_path);

	return nullptr;
}

std::string Inventory::GetBuildPatch(std::string weapon, skinInfo* item) {
	auto texture_path = buildTexturePath(weapon, item);
	return texture_path;
}
int Inventory::GetProperRarity() {
	/*if (!item || !paint_kit)
		return 0;
	auto v13 = paint_kit->GetRarityValue();
	auto v23 = (v13 == 7) + 6;
	auto v14 = item->GetRarityValue() + v13 - 1;
	if (v14 >= 0) {
		if (v14 > v23)
			v14 = v23;
	}
	else
		v14 = 0;
	return v14;*/
}

bool Inventory::CreateTexture(const char* path, LPDIRECT3DTEXTURE9* pImage)
{
	const auto handle = g_Globals.m_Interfaces.m_FileSystem->Open(path, "r", "GAME");
	if (handle) {
		int file_len = g_Globals.m_Interfaces.m_FileSystem->Size(handle);
		char* image = new char[file_len];

		g_Globals.m_Interfaces.m_FileSystem->Read(image, file_len, handle);
		g_Globals.m_Interfaces.m_FileSystem->Close(handle);

		D3DXCreateTextureFromFileInMemory(g_Globals.m_Interfaces.m_DirectDevice, image, file_len, pImage);

		delete[] image;
		return true;
	}
	return false;
}

void Inventory::Proceed()
{
	if (!this)
		return;

	if (Queue.empty())
		return;

	auto item = Queue.front();

	LPDIRECT3DTEXTURE9 texture;
	if (item != _S("not_implemented_yet") && CreateTexture(item.c_str(), &texture))
		this->Textures[item] = texture;
	else
		this->Errors.push_back(item);

	Queue.pop_front();
}

void Inventory::Clear() {
	Queue = {};
}
int GetAgentRarity(int index)
{
	/*
	6-red
	5-light purple
	4-fiolet
	3-blue
	*/

	switch (index)
	{
	case 5105: return 3;
	case 5106: return 4;
	case 5107: return 5;
	case 5108: return 6;
	case 5205: return 3;
	case 5206: return 3;
	case 5207: return 4;
	case 5208: return 3;
	case 5305: return 3;
	case 5306: return 4;
	case 5307: return 5;
	case 5308: return 6;
	case 5400: return 3;
	case 5401: return 3;
	case 5402: return 4;
	case 5403: return 5;
	case 5404: return 6;
	case 4619: return 5;
	case 4680: return 5;
	case 5500: return 4;
	case 5501: return 4;
	case 5502: return 5;
	case 5503: return 5;
	case 5504: return 6;
	case 4718: return 5;
	case 5505: return 3;
	case 5601: return 3;
	case 4711: return 6;
	case 4712: return 5;
	case 4713: return 4;
	case 4714: return 3;
	case 4715: return 3;
	case 4716: return 3;
	case 4726: return 6;
	case 4733: return 6;
	case 4734: return 6;
	case 4735: return 6;
	case 4736: return 6;
	case 4727: return 5;
	case 4728: return 4;
	case 4732: return 5;
	case 4730: return 4;
	}

	return 0;
}
int Inventory::GetRarityByItemId(int itemid)
{

}

std::string Inventory::Changer(void* pubDest, uint32_t* pcubMsgSize)
{
	ProtoWriter msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8, 11);
	if (msg.getAll(CMsgClientWelcome::outofdate_subscribed_caches).empty())
		return msg.serialize();

	ProtoWriter cache(msg.get(CMsgClientWelcome::outofdate_subscribed_caches).String(), 4);
	// If not have items in inventory, Create null inventory
	FixNullInventory(cache);
	// Add custom items
	auto objects = cache.getAll(CMsgSOCacheSubscribed::objects);
	for (size_t i = 0; i < objects.size(); i++)
	{
		ProtoWriter object(objects[i].String(), 2);

		if (!object.has(SubscribedType::type_id))
			continue;

		if (object.get(SubscribedType::type_id).Int32() == 1)
		{
			object.clear(SubscribedType::object_data);

			ClearEquipState(object);
			AddAllItems(object);
			
			cache.replace(Field(CMsgSOCacheSubscribed::objects, TYPE_STRING, object.serialize()), i);
		}
	}
	msg.replace(Field(CMsgClientWelcome::outofdate_subscribed_caches, TYPE_STRING, cache.serialize()), 0);

	return msg.serialize();
}

void Inventory::FixNullInventory(ProtoWriter& cache)
{
	bool inventory_exist = false;
	auto objects = cache.getAll(CMsgSOCacheSubscribed::objects);
	for (size_t i = 0; i < objects.size(); i++)
	{
		ProtoWriter object(objects[i].String(), 2);
		if (!object.has(SubscribedType::type_id))
			continue;
		if (object.get(SubscribedType::type_id).Int32() != 1)
			continue;
		inventory_exist = true;
		break;
	}
	if (!inventory_exist)
	{
		ProtoWriter null_object(2);
		null_object.add(Field(SubscribedType::type_id, TYPE_INT32, (int64_t)1));

		cache.add(Field(CMsgSOCacheSubscribed::objects, TYPE_STRING, null_object.serialize()));
	}
}

void Inventory::ClearEquipState(ProtoWriter& object)
{
	auto object_data = object.getAll(SubscribedType::object_data);
	for (size_t j = 0; j < object_data.size(); j++)
	{
		ProtoWriter item(object_data[j].String(), 19);

		if (item.getAll(CSOEconItem::equipped_state).empty())
			continue;

		// create NOT equiped state for item 
		ProtoWriter null_equipped_state(2);
		null_equipped_state.replace(Field(CSOEconItemEquipped::new_class, TYPE_UINT32, (int64_t)0));
		null_equipped_state.replace(Field(CSOEconItemEquipped::new_slot, TYPE_UINT32, (int64_t)0));
		// unequip all 
		auto equipped_state = item.getAll(CSOEconItem::equipped_state);
		for (size_t k = 0; k < equipped_state.size(); k++)
			item.replace(Field(CSOEconItem::equipped_state, TYPE_STRING, null_equipped_state.serialize()), k);

		object.replace(Field(SubscribedType::object_data, TYPE_STRING, item.serialize()), j);
	}
}

static auto fis_knife(const int i) -> bool
{
	return (i >= 500 && i <= 525) || i == 59 || i == 42;
}

static auto fis_glove(const int i) -> bool
{
	return (i >= 5027 && i <= 5035 || i == 4725); //hehe boys
}
int GetWeaponRarity(std::string rarity)
{
	if (rarity == _S("default"))
		return 0;
	else if (rarity == _S("common"))
		return 1;
	else if (rarity == _S("uncommon"))
		return 2;
	else if (rarity == _S("rare"))
		return 3;
	else if (rarity == _S("mythical"))
		return 4;
	else if (rarity == _S("legendary"))
		return 5;
	else if (rarity == _S("ancient"))
		return 6;
	else if (rarity == _S("immortal"))
		return 7;
	else if (rarity == _S("unusual"))
		return 99;

	return 0;
}

void Inventory::AddAllItems(ProtoWriter& object)
{
	for (auto& x : g_InventorySkins)
	{
		if (x.second.m_bInUseCT && x.second.m_bInUseT)
			AddItem(object, x.first, x.second.wId, x.second.quality, x.second.paintKit, x.second.seed, x.second.wear, x.second.name, x.second.sicker, 4);
		else if (x.second.m_bInUseCT)
			AddItem(object, x.first, x.second.wId, x.second.quality, x.second.paintKit, x.second.seed, x.second.wear, x.second.name, x.second.sicker, 3);
		else if (x.second.m_bInUseT)
			AddItem(object, x.first, x.second.wId, x.second.quality, x.second.paintKit, x.second.seed, x.second.wear, x.second.name, x.second.sicker, 2);
		else
			AddItem(object, x.first, x.second.wId, x.second.quality, x.second.paintKit, x.second.seed, x.second.wear, x.second.name, x.second.sicker);
	}
}

bool is_uncommon(int index)
{
	switch (index)
	{
	case WEAPON_DEAGLE:
	case WEAPON_GLOCK:
	case WEAPON_AK47:
	case WEAPON_AWP:
	case WEAPON_M4A1:
	case WEAPON_M4A1_SILENCER:
	case WEAPON_HKP2000:
	case WEAPON_USP_SILENCER:
		return true;
	default:
		return false;
	}
}

void Inventory::AddItem(ProtoWriter& object, int index, int itemIndex, int rarity, int paintKit, int seed, float wear, std::string name, int ctickers[4], int inusefix)
{
	uint32_t steamid = SteamUser->GetSteamID().GetAccountID();
	if (!steamid)
		return;
	ProtoWriter item(19);
	item.add(Field(CSOEconItem::id, TYPE_UINT64, (int64_t)index));
	item.add(Field(CSOEconItem::account_id, TYPE_UINT32, (int64_t)steamid));
	item.add(Field(CSOEconItem::def_index, TYPE_UINT32, (int64_t)itemIndex));
	item.add(Field(CSOEconItem::inventory, TYPE_UINT32, (int64_t)index));
	item.add(Field(CSOEconItem::origin, TYPE_UINT32, (int64_t)24));
	item.add(Field(CSOEconItem::quantity, TYPE_UINT32, (int64_t)1));
	item.add(Field(CSOEconItem::level, TYPE_UINT32, (int64_t)1));
	item.add(Field(CSOEconItem::style, TYPE_UINT32, (int64_t)0));
	item.add(Field(CSOEconItem::flags, TYPE_UINT32, (int64_t)0));
	item.add(Field(CSOEconItem::in_use, TYPE_BOOL, (int64_t)false));
	item.add(Field(CSOEconItem::original_id, TYPE_UINT64, (int64_t)itemIndex));
	if (is_uncommon(itemIndex))
		rarity++;
	if (fis_knife(itemIndex) || fis_glove(itemIndex))
	{
		item.add(Field(CSOEconItem::quality, TYPE_UINT32, (int64_t)3));
		item.add(Field(CSOEconItem::rarity, TYPE_UINT32, (int64_t)6));
	}
	if (paintKit == 309)
		item.add(Field(CSOEconItem::rarity, TYPE_UINT32, (int64_t)7));
	else
	{
		if (itemIndex == 4725)
			item.add(Field(CSOEconItem::rarity, TYPE_UINT32, (int64_t)7));
		else
		{
			if ((itemIndex >= 5100 && itemIndex <= 6000) || (itemIndex >= 4619 && itemIndex <= 4800 && itemIndex != 4725))
				item.add(Field(CSOEconItem::rarity, TYPE_UINT32, (int64_t)GetAgentRarity(itemIndex)));
			else
			{
				if ((!fis_knife(itemIndex) && !fis_glove(itemIndex)))
					item.add(Field(CSOEconItem::rarity, TYPE_UINT32, (int64_t)std::clamp(rarity_skins[paintKit].rarity + 1, 0, 6)));
			}
		}
	}

	if (name.length() > 0)
		item.add(Field(CSOEconItem::custom_name, TYPE_STRING, name));


	enum TeamID : int
	{
		TEAM_UNASSIGNED,
		TEAM_SPECTATOR,
		TEAM_TERRORIST,
		TEAM_COUNTER_TERRORIST,
	};


	if (inusefix == 4)
	{
		ProtoWriter eqip_state(3);
		eqip_state.add(Field(CSOEconItemEquipped::new_class, TYPE_UINT64, (int64_t)(inusefix - 1)));
		eqip_state.add(Field(CSOEconItemEquipped::new_slot, TYPE_UINT64, (int64_t)(Protobuffs::GetSlotID(itemIndex))));
		item.add(Field(CSOEconItem::equipped_state, TYPE_STRING, eqip_state.serialize()));

		ProtoWriter zeqip_state(3);
		zeqip_state.add(Field(CSOEconItemEquipped::new_class, TYPE_UINT64, (int64_t)(inusefix - 2)));
		zeqip_state.add(Field(CSOEconItemEquipped::new_slot, TYPE_UINT64, (int64_t)(Protobuffs::GetSlotID(itemIndex))));
		item.add(Field(CSOEconItem::equipped_state, TYPE_STRING, zeqip_state.serialize()));
	}
	else if (inusefix)
	{
		ProtoWriter eqip_state(3);
		eqip_state.add(Field(CSOEconItemEquipped::new_class, TYPE_UINT64, (int64_t)(inusefix)));
		eqip_state.add(Field(CSOEconItemEquipped::new_slot, TYPE_UINT64, (int64_t)(Protobuffs::GetSlotID(itemIndex))));
		item.add(Field(CSOEconItem::equipped_state, TYPE_STRING, eqip_state.serialize()));

	}

	// Paint Kit
	float _PaintKitAttributeValue = (float)paintKit;
	auto PaintKitAttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_PaintKitAttributeValue), 4 };
	ProtoWriter PaintKitAttribute(3);
	PaintKitAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)6));
	PaintKitAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, PaintKitAttributeValue));
	item.add(Field(CSOEconItem::attribute, TYPE_STRING, PaintKitAttribute.serialize()));

	// Paint Seed
	float _SeedAttributeValue = (float)seed;
	auto SeedAttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_SeedAttributeValue), 4 };
	ProtoWriter SeedAttribute(3);
	SeedAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)7));
	SeedAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, SeedAttributeValue));
	item.add(Field(CSOEconItem::attribute, TYPE_STRING, SeedAttribute.serialize()));

	// Paint Wear
	float _WearAttributeValue = wear;
	auto WearAttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_WearAttributeValue), 4 };
	ProtoWriter WearAttribute(3);
	WearAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)8));
	WearAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, WearAttributeValue));
	item.add(Field(CSOEconItem::attribute, TYPE_STRING, WearAttribute.serialize()));




	// Stickers
	for (int j = 0; j < 4; j++)
	{
		int _StickerAtributeValue = ctickers[j];
		auto StickerAtributeValue = std::string{ reinterpret_cast<const char*>((void*)&_StickerAtributeValue), 4 };

		ProtoWriter WearAttribute(3);
		WearAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)113 + 4 * j));
		WearAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, StickerAtributeValue));
		item.add(Field(CSOEconItem::attribute, TYPE_STRING, WearAttribute.serialize()));


		float _AttributeValue = 1.0f;
		auto AttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_AttributeValue), 4 };

		ProtoWriter Attribute(3);
		Attribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)115 + 4 * j));
		Attribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, AttributeValue));
		item.add(Field(CSOEconItem::attribute, TYPE_STRING, Attribute.serialize()));


		//item.attribute().add(make_econ_item_attribute(113 + 4 * j, uint32_t(289 + j))); // Sticker Kit
		//item.attribute().add(make_econ_item_attribute(114 + 4 * j, float(0.001f)));     // Sticker Wear
		//item.attribute().add(make_econ_item_attribute(115 + 4 * j, float(1.f)));        // Sticker Scale
		//item.attribute().add(make_econ_item_attribute(116 + 4 * j, float(0.f)));        // Sticker Rotation
	}







	object.add(Field(SubscribedType::object_data, TYPE_STRING, item.serialize()));
}

uint8_t* ssdgfadefault(HMODULE hModule, const char* szSignature)
{
	static auto pattern_to_byte = [](const char* pattern) {
		auto bytes = std::vector<int>{};
		auto start = const_cast<char*>(pattern);
		auto end = const_cast<char*>(pattern) + strlen(pattern);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;
				if (*current == '?')
					++current;
				bytes.push_back(-1);
			}
			else {
				bytes.push_back(strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	//auto Module = GetModuleHandleA(szModule);

	auto dosHeader = (PIMAGE_DOS_HEADER)hModule;
	auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)hModule + dosHeader->e_lfanew);

	auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
	auto patternBytes = pattern_to_byte(szSignature);
	auto scanBytes = reinterpret_cast<std::uint8_t*>(hModule);

	auto s = patternBytes.size();
	auto d = patternBytes.data();

	for (auto i = 0ul; i < sizeOfImage - s; ++i) {
		bool found = true;
		for (auto j = 0ul; j < s; ++j) {
			if (scanBytes[i + j] != d[j] && d[j] != -1) {
				found = false;
				break;
			}
		}
		if (found) {
			return &scanBytes[i];
		}
	}

	return nullptr;
};

template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(ssdgfadefault(GetModuleHandleA(_S("client.dll")), _S(("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08"))) + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(ssdgfadefault(GetModuleHandleA(_S("client.dll")), _S("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28")));
	return (T*)find_hud_element(pThis, name);
}

struct hud_weapons_t {
	std::int32_t* get_weapon_count() {
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};



void Inventory::force_full_update()
{
	static auto fn = reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(ssdgfadefault(GetModuleHandleA(_S("client.dll")), _S(("55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C"))));
	
	auto element = FindHudElement<std::uintptr_t*>(_S(("CCSGO_HudWeaponSelection")));

	auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xA0);

	if (hud_weapons == nullptr)
		return;

	if (!*hud_weapons->get_weapon_count())
		return;

	for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
		i = fn(hud_weapons, i);


}

bool Inventory::Presend(uint32_t& unMsgType, void* pubData, uint32_t& cubData)
{
	uint32_t MessageType = unMsgType & 0x7FFFFFFF;
	if (MessageType == k_EMsgGCAdjustItemEquippedState) {

		ProtoWriter msg((void*)((DWORD)pubData + 8), cubData - 8, 19);

		if (!msg.has(CMsgAdjustItemEquippedState::item_id)
			|| !msg.has(CMsgAdjustItemEquippedState::new_class)
			|| !msg.has(CMsgAdjustItemEquippedState::new_slot))
			return true;

		



		uint32_t item_id = msg.get(CMsgAdjustItemEquippedState::item_id).UInt32();
		uint32_t new_class = msg.get(CMsgAdjustItemEquippedState::new_class).UInt32();
		
		if (item_id > 200000)
		{
			for (auto& f : g_MedalSkins)
				f.second.equip_state = 0;


			auto medals = g_MedalSkins[item_id];
			g_MedalSkins[item_id].equip_state = 1;
		}
		

		if (item_id > 20000 && item_id < 200000)
		{
			auto weapon = g_InventorySkins[item_id];

			if (new_class == 2)
			{
				for (auto& skins : g_InventorySkins)
				{
					if (Protobuffs::GetSlotID(skins.second.wId) == Protobuffs::GetSlotID(weapon.wId))
						skins.second.m_bInUseT = false;
				}
				g_InventorySkins[item_id].m_bInUseT = true;
				
			
			}
			else if (new_class == 3)
			{
				for (auto& skins : g_InventorySkins)
				{
					if (Protobuffs::GetSlotID(skins.second.wId) == Protobuffs::GetSlotID(weapon.wId))
						skins.second.m_bInUseCT = false;
				}

				g_InventorySkins[item_id].m_bInUseCT = true;
			
			
			}
		}


		g_Globals.m_Interfaces.m_EngineClient->ExecuteClientCmd(_S("econ_clear_inventory_images"));

		write.SendClientHello();
		write.SendMatchmakingClient2GCHello();

		g_Globals.m_Interfaces.m_ClientState->m_nDeltaTick() = -1;
		force_full_update();

		return false;
	}
	return true;
}

void ParseMedals()
{
	valve_parser::Document items_document;
	bool item_document_loaded = items_document.Load(_S(".\\csgo\\scripts\\items\\items_game.txt"), valve_parser::ENCODING::UTF8);
	if (!item_document_loaded)
		return;

	auto items = items_document.BreadthFirstSearch(_S("items"));
	if (!items || !items->ToObject())
		return;

	int i = 0;

	for (auto child : items->children)
	{
		if (child->ToObject())
		{
			std::string object_id = child->ToObject()->name.toString();

			if ((atoi(object_id.c_str()) >= 874) && (atoi(object_id.c_str()) <= 6133))
			{
				std::string object_prefab = child->ToObject()->GetKeyByName(_S("prefab"))->ToKeyValue()->Value.toString();

				if ((object_prefab != _S("collectible_untradable")) && (object_prefab != _S("operation_coin")) && (object_prefab != _S("prestige_coin"))
					&& (object_prefab != _S("attendance_pin")) && (object_prefab != _S("commodity_pin")) && (object_prefab != _S("season1_coin"))
					&& (object_prefab != _S("season2_coin")) && (object_prefab != _S("season3_coin")) && (object_prefab != _S("season4_coin"))
					&& (object_prefab != _S("season5_coin")) && (object_prefab != _S("season6_coin")) && (object_prefab != _S("season7_coin"))
					&& (object_prefab != _S("season8_coin"))) continue;

				std::string object_name = child->ToObject()->GetKeyByName(_S("name"))->ToKeyValue()->Value.toString();


				fosso[atoi(object_id.c_str())] = { object_name, atoi(object_id.c_str()) };

				i++;
				if ((atoi(object_id.c_str()) == 6133))
					break;
			}
		}
	}

	//for (size_t i = 0; i < G::MedalMap.size(); i++)
	//{
	//	G::MedalList.push_back(G::MedalMap.at(i).medal_name.c_str());
	//}
}

namespace fzdetail
{
	constexpr auto process_pattern_byte(const std::pair<char, char> pair) -> std::pair<std::uint8_t, bool>
	{
		return { std::uint8_t(pair.first), pair.second == 'x' };
	}

	template <std::size_t N>
	constexpr auto select_pattern_byte(const char(&pattern)[N], const char(&mask)[N], std::size_t n) -> std::pair<char, char>
	{
		return { pattern[n], mask[n] };
	}

	template <typename>
	struct sig_processor {};

	template <std::size_t... Indices>
	struct sig_processor<std::index_sequence<Indices...>>
	{
		template <std::size_t N>
		static constexpr auto process(const char(&pattern)[N], const char(&mask)[N])->std::array<std::pair<std::uint8_t, bool>, N>
		{
			return { process_pattern_byte(select_pattern_byte(pattern, mask, Indices))... };
		}
	};
}

auto get_module_info(const char* module_name) -> std::pair<std::uintptr_t, std::size_t>
{
	const auto module = GetModuleHandleA(module_name);
	if (!module)
		return { 0, 0 };
	MODULEINFO module_info;
	K32GetModuleInformation(GetCurrentProcess(), module, &module_info, sizeof(MODULEINFO));
	return { std::uintptr_t(module_info.lpBaseOfDll), module_info.SizeOfImage };
}



template <std::size_t N>
auto find_pattern(const char* module_name, const char(&pattern)[N], const char(&mask)[N]) -> std::uintptr_t
{
	const std::array<std::pair<std::uint8_t, bool>, N> signature = fzdetail::sig_processor<std::make_index_sequence<N>>::process(pattern, mask);

	const auto info = get_module_info(module_name);
	const auto address = reinterpret_cast<std::uint8_t*>(info.first);
	const auto size = info.second;

	const auto ret = std::search(address, address + size, signature.begin(), signature.end(),
		[](std::uint8_t curr, std::pair<std::uint8_t, bool> curr_pattern)
		{
			return (!curr_pattern.second) || curr == curr_pattern.first;
		});

	return ret == address + size ? 0 : std::uintptr_t(ret);
}

template <typename Key, typename Value>
struct Node_t
{
	int previous_id;		//0x0000
	int next_id;			//0x0004
	void* _unknown_ptr;		//0x0008
	int _unknown;			//0x000C
	Key key;				//0x0010
	Value value;			//0x0014
};


template <typename Key, typename Value>
struct Head_t
{
	Node_t<Key, Value>* memory;		//0x0000
	int allocation_count;			//0x0004
	int grow_size;					//0x0008
	int start_element;				//0x000C
	int next_available;				//0x0010
	int _unknown;					//0x0014
	int last_element;				//0x0018
}; //Size=0x001C

// could use CUtlString but this is just easier and CUtlString isn't needed anywhere else
struct String_t
{
	char* buffer;	//0x0000
	int capacity;	//0x0004
	int grow_size;	//0x0008
	int length;		//0x000C
}; //Size=0x0010

class CCStrike15ItemSystem;
class CCStrike15ItemSchema;
struct CPaintKit
{
	int id;						//0x0000

	String_t name;				//0x0004
	String_t description;		//0x0014
	String_t item_name;			//0x0024
	String_t material_name;		//0x0034
	String_t image_inventory;	//0x0044

	char pad_0x0054[0x8C];		//0x0054
}; //Size=0x00E0

struct CStickerKit
{
	int id;

	int item_rarity;

	String_t name;
	String_t description;
	String_t item_name;
	String_t material_name;
	String_t image_inventory;

	int tournament_event_id;
	int tournament_team_id;
	int tournament_player_id;
	bool is_custom_sticker_material;

	float rotate_end;
	float rotate_start;

	float scale_min;
	float scale_max;

	float wear_min;
	float wear_max;

	String_t image_inventory2;
	String_t image_inventory_large;

	std::uint32_t pad0[4];
};
auto zget_export(const char* module_name, const char* export_name) -> void*
{
	HMODULE mod;
	while (!((mod = GetModuleHandleA(module_name))))
		Sleep(100);

	return reinterpret_cast<void*>(GetProcAddress(mod, export_name));
}

struct skin_info
{
	int seed = -1;
	int paintkit;
	int rarity = 0;
	std::string tag_name;
	std::string cdn_name;
};
void Inventory::ParseSkins() const
{
	parser::parse();
	const auto V_UCS2ToUTF8 = static_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(zget_export(_S("vstdlib.dll"), _S("V_UCS2ToUTF8")));
	const auto sig_address = find_pattern(_S("client.dll"), "\xE8\x00\x00\x00\x00\xFF\x76\x0C\x8D\x48\x04\xE8", "x????xxxxxxx");

	// Skip the opcode, read rel32 address
	const auto item_system_offset = *reinterpret_cast<std::int32_t*>(sig_address + 1);

	// Add the offset to the end of the instruction
	const auto item_system_fn = reinterpret_cast<CCStrike15ItemSystem * (*)()>(sig_address + 5 + item_system_offset);

	// Skip VTable, first member variable of ItemSystem is ItemSchema
	const auto item_schema = reinterpret_cast<CCStrike15ItemSchema*>(std::uintptr_t(item_system_fn()) + sizeof(void*));

	{
		const auto sticker_sig = find_pattern(_S("client.dll"), "\x53\x8D\x48\x04\xE8\x00\x00\x00\x00\x8B\x4D\x10", "xxxxx????xxx") + 4;

		// Skip the opcode, read rel32 address
		const auto get_sticker_kit_definition_offset = *reinterpret_cast<std::intptr_t*>(sticker_sig + 1);

		// Add the offset to the end of the instruction
		const auto get_sticker_kit_definition_fn = reinterpret_cast<CPaintKit * (__thiscall*)(CCStrike15ItemSchema*, int)>(sticker_sig + 5 + get_sticker_kit_definition_offset);

		// Skip instructions, skip opcode, read offset
		const auto start_element_offset = *reinterpret_cast<intptr_t*>(std::uintptr_t(get_sticker_kit_definition_fn) + 8 + 2);

		// Calculate head base from start_element's offset
		const auto head_offset = start_element_offset - 12;

		const auto map_head = reinterpret_cast<Head_t<int, CStickerKit*>*>(std::uintptr_t(item_schema) + head_offset);

		for (auto i = 0; i <= map_head->last_element; ++i)
		{
			const auto sticker_kit = map_head->memory[i].value;

			char sticker_name_if_valve_fucked_up_their_translations[64];

			auto sticker_name_ptr = sticker_kit->item_name.buffer + 1;

			if (strstr(sticker_name_ptr, _S("StickerKit_dhw2014_dignitas")))
			{
				strcpy_s(sticker_name_if_valve_fucked_up_their_translations, _S("StickerKit_dhw2014_teamdignitas"));
				strcat_s(sticker_name_if_valve_fucked_up_their_translations, sticker_name_ptr + 27);
				sticker_name_ptr = sticker_name_if_valve_fucked_up_their_translations;
			}

			const auto wide_name = g_Globals.m_Interfaces.m_Localize->Find(sticker_name_ptr);
			char name[256];
			g_Stickers[sticker_kit->id] = { name, sticker_kit->id };
		}
		g_Stickers[0] = { "None", 0 };
	}

	ParseMedals();

	std::unordered_map<std::string, std::set<std::string>> weaponSkins;
	std::unordered_map<std::string, skin_info> skinMap;
	std::unordered_map<std::string, std::string> skinNames;

	valve_parser::Document doc;
	auto r = doc.Load(_S(R"(.\csgo\scripts\items\items_game.txt)"), valve_parser::ENCODING::UTF8);
	if (!r)
		return;

	valve_parser::Document english;
	r = english.Load(_S(R"(.\csgo\resource\csgo_english.txt)"), valve_parser::ENCODING::UTF16_LE);
	if (!r)
		return;

	auto weapon_skin_combo = doc.BreadthFirstSearch(_S("weapon_icons"));
	if (!weapon_skin_combo || !weapon_skin_combo->ToObject())
		return;

	auto paint_kits_rarity = doc.BreadthFirstSearchMultiple(_S("paint_kits_rarity"));
	if (paint_kits_rarity.empty())
		return;

	auto skin_data_vec = doc.BreadthFirstSearchMultiple(_S("paint_kits"));
	if (skin_data_vec.empty())
		return;

	auto paint_kit_names = english.BreadthFirstSearch(_S("Tokens"));
	if (!paint_kit_names || !paint_kit_names->ToObject())
		return;

	std::array weaponNames = {
		std::string(_S("deagle")),
		std::string(_S("elite")),
		std::string(_S("fiveseven")),
		std::string(_S("glock")),
		std::string(_S("ak47")),
		std::string(_S("aug")),
		std::string(_S("awp")),
		std::string(_S("famas")),
		std::string(_S("g3sg1")),
		std::string(_S("galilar")),
		std::string(_S("m249")),
		std::string(_S("m4a1_silencer")),
		std::string(_S("m4a1")),
		std::string(_S("mac10")),
		std::string(_S("p90")),
		std::string(_S("ump45")),
		std::string(_S("xm1014")),
		std::string(_S("bizon")),
		std::string(_S("mag7")),
		std::string(_S("negev")),
		std::string(_S("sawedoff")),
		std::string(_S("tec9")),
		std::string(_S("hkp2000")),
		std::string(_S("mp5sd")),
		std::string(_S("mp7")),
		std::string(_S("mp9")),
		std::string(_S("nova")),
		std::string(_S("p250")),
		std::string(_S("scar20")),
		std::string(_S("sg556")),
		std::string(_S("ssg08")),
		std::string(_S("usp_silencer")),
		std::string(_S("cz75a")),
		std::string(_S("revolver")),
		std::string(_S("knife_m9_bayonet")),
		std::string(_S("bayonet")),
		std::string(_S("knife_flip")),
		std::string(_S("knife_gut")),
		std::string(_S("knife_karambit")),
		std::string(_S("knife_tactical")),
		std::string(_S("knife_falchion")),
		std::string(_S("knife_survival_bowie")),
		std::string(_S("knife_butterfly")),
		std::string(_S("knife_push")),
		std::string(_S("knife_ursus")),
		std::string(_S("knife_gypsy_jackknife")),
		std::string(_S("knife_stiletto")),
		std::string(_S("knife_widowmaker")),
		std::string(_S("knife_skeleton")),
		std::string(_S("knife_outdoor")),
		std::string(_S("knife_canis")),
		std::string(_S("knife_cord")),
		std::string(_S("knife_css")),
		std::string(_S("studded_bloodhound_gloves")),
		std::string(_S("sporty_gloves")),
		std::string(_S("slick_gloves")),
		std::string(_S("leather_handwraps")),
		std::string(_S("motorcycle_gloves")),
		std::string(_S("specialist_gloves")),
		std::string(_S("studded_brokenfang_gloves")),
		std::string(_S("studded_hydra_gloves")),

		//brokenfang_gloves
	};

	for (const auto& child : weapon_skin_combo->children)
	{
		if (child->ToObject())
		{
			for (const auto& weapon : weaponNames)
			{
				auto skin_name = child->ToObject()->GetKeyByName(_S("icon_path"))->ToKeyValue()->Value.toString();
				const auto pos = skin_name.find(weapon);
				if (pos != std::string::npos)
				{
					const auto pos2 = skin_name.find_last_of('_');
					weaponSkins[weapon].insert(
						skin_name.substr(pos + weapon.length() + 1,
							pos2 - pos - weapon.length() - 1)
					);
					break;
				}
			}
		}
	}

	for (const auto& skin_data : skin_data_vec)
	{
		if (skin_data->ToObject())
		{
			for (const auto& skin : skin_data->children)
			{
				if (skin->ToObject())
				{
					skin_info si;
					si.paintkit = skin->ToObject()->name.toInt();

					if (si.paintkit == 0)
						continue;

					auto skin_name = skin->ToObject()->GetKeyByName(_S("name"))->ToKeyValue()->Value.toString();
					si.cdn_name = skin_name;
					auto tag_node = skin->ToObject()->GetKeyByName(_S("description_tag"));
					if (tag_node)
					{
						auto tag = tag_node->ToKeyValue()->Value.toString();
						tag = tag.substr(1, std::string::npos);
						std::transform(tag.begin(), tag.end(), tag.begin(), towlower);
						si.tag_name = tag;
					}



					auto key_val = skin->ToObject()->GetKeyByName(_S("seed"));
					if (key_val != nullptr)
						si.seed = key_val->ToKeyValue()->Value.toInt();

					skinMap[skin_name] = si;
				}
			}
		}
	}

	for (const auto& child : paint_kit_names->children)
	{
		if (child->ToKeyValue())
		{
			auto key = child->ToKeyValue()->Key.toString();
			std::transform(key.begin(), key.end(), key.begin(), towlower);
			if (key.find(_S("paintkit")) != std::string::npos &&
				key.find(_S("tag")) != std::string::npos)
			{
				skinNames[key] = child->ToKeyValue()->Value.toString();
			}
		}
	}

	for (const auto& rarity : paint_kits_rarity)
	{
		if (rarity->ToObject())
		{
			for (const auto& child : rarity->children)
			{
				if (child->ToKeyValue())
				{
					std::string paint_kit_name = child->ToKeyValue()->Key.toString();
					std::string paint_kit_rarity = child->ToKeyValue()->Value.toString();

					auto skinInfo = &skinMap[paint_kit_name];

					skinInfo->rarity = GetWeaponRarity(paint_kit_rarity);
				}
			}
		}
	}

	for (auto weapon : weaponNames)
	{
		for (auto skin : weaponSkins[weapon])
		{
			skinInfo* info = &_inv.inventory.skinInfo[skinMap[skin].paintkit];
			info->weaponName.push_back(weapon);
			info->cdnName = skin;
			info->SkinName = skinNames[skinMap[skin].tag_name].c_str();
			info->rarity = skinMap[skin].rarity;
			_inv.inventory.skinInfo.insert({ skinMap[skin].paintkit, *info });
		}
	}
}