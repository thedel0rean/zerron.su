#pragma once

#include <map>
#include <vector>
#include <unordered_map>
struct weapon_infos
{
	constexpr weapon_infos(const char* model, const char* icon = nullptr) :
		model(model),
		icon(icon)
	{}
	const char* model;
	const char* icon;
};

struct CWeaponSelectList
{
	constexpr CWeaponSelectList(int index, const char* name, int rarity) :
		rarity(rarity),
		index(index),
		name(name)
	{}

	int rarity = 0;
	int index = 0;
	const char* name = "weaponname";
};

extern const std::map<short, weapon_infos> k_weapon_info;
extern const std::map<short, const char*> k_glove_names;
extern const std::map<short, const char*> k_knife_names;
extern const std::map<short, const char*> k_item_names;
extern const std::vector<CWeaponSelectList> k_inventory_names;
extern const std::vector<std::pair<int, const char*>> save_guns;