#include "parser.h"
#include "optional.hpp"
#include <vector>
#include <algorithm>
#include "../../SDK/Globals.hpp"

namespace parser
{
	bool starts_with(std::string str, std::string v)
	{
		if (str.length() > 0)
			return str.find(v) == 0;
		else
			return false;
	}

	bool ends_with(std::string str, std::string v) {
		if (str.length() >= v.length())
			return (0 == str.compare(str.length() - v.length(), v.length(), v));
		else
			return false;
	}

	std::string to_lower(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str;
	}

	std::wstring to_wlower(std::wstring str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str;
	}

	vdf::document document;
	weapon_list_t weapons;
	knife_list_t knifes;
	knife_list_t default_knifes;
	rarity_list_t rarities;
	agents_list_t agentes;

	std::vector<skin_t*> get_skins_by_name(std::string name) {
		std::vector<skin_t*> vec;

		for (auto& weapon : parser::weapons.list) {
			for (auto& skin : weapon.skins.list) {
				if (skin.item_name == name) {
					vec.push_back(&skin);
				}
			}
		}

		return vec;
	};

	const wchar_t* translate(std::string token)
	{
		// this method was coded like ages ago, lmao. please some1 fix this ahaha
		if (token.empty()) return L"";

		if (token[0] == '#')
			token = token.erase(0, 1);

		if (!g_Globals.m_Interfaces.m_Localize)
			return L"";

		auto p_wstr = g_Globals.m_Interfaces.m_Localize->Find(token.data());
		if (!p_wstr) return L"";

		//auto wstr = std::wstring(p_wstr);
		if (p_wstr == nullptr || wcslen(p_wstr) < 1) return L"";

		/*int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);*/
		return p_wstr;
	}

	bool parse_rarities()
	{
		auto items = document.breadth_first_search(_S("rarities"));
		for (auto child : items->children)
		{
			auto object = child->to_object();
			if (!object) continue;

			auto value_node = object->get_key_by_name(_S("value"));
			if (!value_node) continue;

			auto value_kv = value_node->to_keyvalue();
			if (!value_kv) continue;

			auto Color_node = object->get_key_by_name(_S("color"));
			if (!Color_node) continue;

			auto Color_kv = Color_node->to_keyvalue();
			if (!Color_kv) continue;

			rarity_t rarity;
			rarity.id = value_kv->value.to_int();
			rarity.name = object->name.to_string();
			rarity.Color_name = Color_kv->value.to_string();
			if (rarity.name == _S("unusual"))
				continue;

			rarities.list.push_back(rarity);
		}

		auto Colors = document.breadth_first_search(_S("colors"));

		for (auto child : Colors->children)
		{
			auto object = child->to_object();
			if (!object) continue;

			auto Color_node = object->get_key_by_name(_S("hex_color"));
			if (!Color_node) continue;

			auto Color_kv = Color_node->to_keyvalue();
			if (!Color_kv) continue;

			auto get_rarity_by_Color_name = [](std::string Color_name) -> tl::optional<rarity_t&>
			{
				for (auto& rarity : parser::rarities.list) {
					if (rarity.Color_name == Color_name) {
						return rarity;
					}
				}

				return tl::nullopt;
			};

			auto rarity = get_rarity_by_Color_name(object->name.to_string());
			if (!rarity.has_value()) continue;

			uint32_t value = stoi(Color_kv->value.to_string().erase(0, 1), nullptr, 16);
			rarity->Color = Color(value);
			rarity->Color[3] = 255;
		}

		return true;
	}

	bool parse_weapons()
	{
		auto items = document.breadth_first_search(_S("items"));
		for (auto child : items->children)
		{
			auto object = child->to_object();
			if (!object) continue;

			auto name_node = object->get_key_by_name(_S("name"));
			if (!name_node) continue;

			auto name_kv = name_node->to_keyvalue();
			if (!name_kv) continue;
			if (!starts_with(name_kv->value.to_string(), _S("weapon_"))) continue;

			auto prefab_node = object->get_key_by_name(_S("prefab"));
			if (!prefab_node) continue;

			auto prefab_kv = prefab_node->to_keyvalue();
			if (!prefab_kv) continue;
			if (!starts_with(prefab_kv->value.to_string(), _S("weapon_")) &&
				!starts_with(prefab_kv->value.to_string(), _S("melee_unusual"))) continue;

			/*auto sub_node = object->get_key_by_name("item_sub_position");
			if (!sub_node) continue;

			auto sub_kv = sub_node->to_keyvalue();

			auto piska = sub_kv->value.to_string();*/

			weapon_t weapon;
			weapon.id = object->name.to_int();
			weapon.item_name = name_kv->value.to_string();
			weapon.prefab = prefab_kv->value.to_string();

			auto item_name_node = object->get_key_by_name(_S("item_name"));
			if (item_name_node)
			{
				auto item_name_kv = item_name_node->to_keyvalue();
				if (item_name_kv)
				{
					weapon.translated_name = translate(item_name_kv->value.to_string());
					//feature::visuals->weapon_names[(int)weapon.id] = translate(item_name_kv->value.to_string());
				}
			}

			weapons.list.push_back(weapon);
		}

		auto prefabs = document.breadth_first_search_multiple(_S("prefabs"));
		for (auto prefab : prefabs)
		{
			for (auto child : prefab->children)
			{
				auto object = child->to_object();
				if (!object) continue;

				auto get_weapon_by_prefab = [](std::string prefab) -> tl::optional<weapon_t&> {
					for (auto& weapon : parser::weapons.list) {
						if (weapon.prefab == prefab) {
							return weapon;
						}
					}

					return tl::nullopt;
				};

				auto weapon = get_weapon_by_prefab(object->name.to_string());
				if (!weapon.has_value()) continue;

				if (weapon->translated_name.empty())
				{
					auto item_name_node = object->get_key_by_name(_S("item_name"));
					if (!item_name_node) continue;

					auto item_name_kv = item_name_node->to_keyvalue();
					if (!item_name_kv) continue;

					weapon->translated_name = translate(item_name_kv->value.to_string());
					//feature::visuals->weapon_names[(int)weapon->id] = translate(item_name_kv->value.to_string());
				}
			}
		}

		return true;
	}

	bool parse_skins()
	{
		// Parse skins
		auto weapon_icons = document.breadth_first_search(_S("weapon_icons"));
		for (auto child : weapon_icons->children)
		{
			auto object = child->to_object();
			if (!object) continue;

			auto path_node = object->get_key_by_name(_S("icon_path"));
			if (!path_node) continue;

			auto path_kv = path_node->to_keyvalue();
			if (!path_kv) continue;
			auto path = path_kv->value.to_string();
			if (!ends_with(path, _S("_light"))) continue;

			for (auto& weapon : weapons.list)
			{
				auto pos = path.find(weapon.item_name);
				if (pos == std::string::npos) continue;

				if (path.find(_S("silencer")) != std::string::npos &&
					weapon.item_name.find(_S("silencer")) == std::string::npos)
					continue;

				auto pos2 = path.find_last_of('_');
				//if (pos == string::npos) continue;

				auto item_name = path.substr(pos + weapon.item_name.length() + 1,
					pos2 - pos - weapon.item_name.length() - 1);

				//if (get_skin_by_name(item_name).has_value())
				//	continue;

				skin_t skin;
				skin.item_name = item_name;

				weapon.skins.list.push_back(skin);
				break;
			}
		}

		// Parse skins id and translated name
		auto paint_kits = document.breadth_first_search_multiple(_S("paint_kits"));
		for (auto nodes : paint_kits)
		{
			for (auto child : nodes->children)
			{
				auto object = child->to_object();
				if (!object) continue;

				auto skin_id = object->name.to_int();
				if (skin_id == 0) continue;

				auto description_node = object->get_key_by_name(_S("description_tag"));
				if (!description_node) continue;

				auto description_kv = description_node->to_keyvalue();
				if (!description_kv) continue;

				auto description_tag = description_kv->value.to_string();
				description_tag = description_tag.substr(1, std::string::npos);

				auto name_node = object->get_key_by_name(_S("name"));
				if (!name_node) continue;

				auto name_kv = name_node->to_keyvalue();
				if (!name_kv) continue;
				auto skin_name = name_kv->value.to_string();

				auto skins = get_skins_by_name(skin_name);
				if (skins.size() == 0) continue;

				for (auto& skin : skins)
				{
					skin->id = skin_id;
					skin->translated_name = translate(description_tag);
				}
			}
		}

		auto get_rarity_by_name = [](std::string name) -> int32_t {\

			/*if (name == "default")
				return 0;
			else if (name == "common")
				return 1;
			else if (name == "uncommon")
				return 2;
			else if (name == "rare")
				return 3;
			else if (name == "mythical")
				return 4;
			else if (name == "legendary")
				return 5;
			else if (name == "ancient")
				return 6;
			else if (name == "immortal")
				return 7;
			else if (name == "unusual")
				return 99;
			*/
			for (auto& rarity : parser::rarities.list)
			{
				if (ends_with(name, /*"_" + */rarity.name))
					return rarity.id;
			}

		return -1;
		};

		// Parse skins rarities again, lol
		auto paint_kits_rarity = document.breadth_first_search_multiple(_S("paint_kits_rarity"));
		for (auto paint_kit_rarity : paint_kits_rarity)
		{
			for (auto child : paint_kit_rarity->children)
			{
				auto object = child->to_keyvalue();
				if (!object) continue;

				auto name = object->key.to_string();

				auto skins = get_skins_by_name(name);
				if (skins.size() == 0) continue;

				for (auto& skin : skins)
				{
					if (skin->rarity != 0)
					{
						rarity_skins[skin->id] = { skin->id ,skin->rarity };
						continue;
					}
					else
					{
						skin->rarity = get_rarity_by_name(object->value.to_string());
						rarity_skins[skin->id] = { skin->id ,skin->rarity };
						continue;
					}
				}
			}
		}
		/*auto client_loot_lists = document.breadth_first_search_multiple(_S("client_loot_lists"));
		for (auto node : client_loot_lists)
		{
			for (auto child : node->children)
			{
				auto object = child->to_object();
				if (!object) continue;

				auto object_name = object->name.to_string();
				if (object_name.find(_S("sticker_pack")) != std::string::npos ||
					object_name.find(_S("signature_pack")) != std::string::npos)
					continue;

				auto rarity = get_rarity_by_name(object_name);
				if (rarity == -1) continue;

				for (auto item : child->children)
				{
					auto kv = item->to_keyvalue();
					if (!kv) continue;

					auto name = kv->key.to_string();
					if (!starts_with(name, _S("["))) continue;

					auto end_idx = name.find_first_of(']');
					if (end_idx == std::string::npos) continue;

					auto skin_name = name.substr(1, end_idx - 1);
					auto weapon_name = name.substr(end_idx + 1, std::string::npos);
					if (weapon_name == _S("sticker")) continue;

					auto weapon = weapons.get_by_item_name(weapon_name);
					if (!weapon.has_value()) continue;

					auto skin = weapon.value().skins.get_by_item_name(skin_name);
					if (!skin.has_value()) continue;

					skin->rarity = rarity;
					rarity_skins[skin->id] = { skin->id, rarity };
				}
			}
		}*/
		return true;
	}

	bool parse_knifes()
	{
		auto items = document.breadth_first_search(_S("items"));
		for (auto child : items->children)
		{
			auto object = child->to_object();
			if (!object) continue;

			auto name_node = object->get_key_by_name(_S("name"));
			if (!name_node) continue;

			auto name_kv = name_node->to_keyvalue();
			if (!name_kv) continue;
			if (name_kv->value.to_string() == _S("weapon_knifegg")) continue;

			auto item_name_node = object->get_key_by_name(_S("item_name"));
			if (!item_name_node) continue;

			auto item_name_kv = item_name_node->to_keyvalue();
			if (!item_name_kv) continue;

			auto prefab_node = object->get_key_by_name(_S("prefab"));
			if (!prefab_node) continue;

			auto prefab_kv = prefab_node->to_keyvalue();
			if (!prefab_kv) continue;

			auto prefab_value = prefab_kv->value.to_string();
			auto is_melee = (prefab_value == _S("melee"));
			auto is_melee_unusual = (prefab_value == _S("melee_unusual"));
			if (!is_melee && !is_melee_unusual) continue;

			auto model_node = object->get_key_by_name(_S("model_player"));
			if (!model_node) continue;

			auto model_kv = model_node->to_keyvalue();
			if (!model_kv) continue;

			auto model_world = object->get_key_by_name(_S("model_world"));
			if (!model_world) continue;

			auto model_index = model_world->to_keyvalue();
			if (!model_index) continue;

			knife_t knife;

			knife.id = object->name.to_int();

			knife.model_player_path = model_kv->value.to_string();
			knife.model_world_path = model_index->value.to_string();

			knife.model_world = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex(knife.model_world_path.c_str());
			knife.model_player = g_Globals.m_Interfaces.m_ModelInfo->GetModelIndex(knife.model_player_path.c_str());



			knife.item_name = name_kv->value.to_string();
			knife.translated_name = translate(item_name_kv->value.to_string());

			//feature::visuals->weapon_names[(int)knife.id] = translate(item_name_kv->value.to_string());

			if (is_melee) // default
				default_knifes.list.push_back(knife);
			else
				knifes.list.push_back(knife);
		}

		return true;
	}
	bool sort_weapons()
	{
		auto& vec = weapons.list;
		auto weapon = vec.begin();
		while (weapon != vec.end())
		{
			if (weapon->skins.list.size() <= 0)
				weapon = vec.erase(weapon);
			else
				weapon++;
		}

		return true;
	}

	bool parse()
	{
		char filename[MAX_PATH];
		if (GetModuleFileNameA(GetModuleHandleA(("client.dll")), filename, MAX_PATH) == 0) {
			return false;
		}

		std::string tmp_path(filename);
		int pos = tmp_path.find(("csgo"));
		tmp_path = tmp_path.substr(0, pos);
		tmp_path = tmp_path + ("csgo\\scripts\\items\\items_game.txt");

		if (!document.load(_strdup(tmp_path.c_str()), vdf::ENCODING::UTF8)) {
			//logger::error("items_game.txt not found!");
			return false;
		}

		parse_rarities();
		parse_weapons();
		parse_knifes();
		parse_skins();
		sort_weapons();



		return true;
	}
}

namespace valve_parser
{
	Object::Object(Document* doc) : Node(doc) {}

	Object* Object::ToObject()
	{
		return this;
	}

	std::shared_ptr<Node> Object::GetKeyByName(char* name)
	{
		for (auto& child : children)
		{
			if (child->ToKeyValue())
			{
				if (Util::StrEqu(child->ToKeyValue()->Key, name))
					return child;
			}
		}

		return 0;
	}

	bool Object::Parse()
	{
		std::shared_ptr<Node> n;

		while (*_doc->p)
		{
			//check for object close
			auto string_begin = Str::ParseTextExpectedTag(_doc->p, STRING, true);
			if (!string_begin)
			{
				auto obj_close = Str::ParseTextExpectedTag(_doc->p, OBJECT_CLOSE, true);
				if (obj_close)
				{
					_doc->p = obj_close + 1;
					return true;
				}
				else
					return false;
			}

			if (!_doc->identify(n))
				return false;

			if (n->ToKeyValue())
			{
				this->children.push_back(n);
			}

			if (n->ToObject())
			{
				this->children.push_back(n);
				Object* obj = n->ToObject();
				if (!obj->Parse())
					return false;
			}
		}

		return false;
	}

	bool Node::Parse()
	{
		std::shared_ptr<Node> n;

		while (*_doc->p)
		{
			if (!_doc->identify(n))
			{
				if (!Str::EndReached(_doc->p, OBJECT_OPEN) &&
					!Str::EndReached(_doc->p, OBJECT_CLOSE) &&
					!Str::EndReached(_doc->p, STRING))
					return true;
				else
					return false;
			}

			if (n->ToKeyValue())
			{
				this->children.push_back(n);
			}

			if (n->ToObject())
			{
				this->children.push_back(n);
				Object* obj = n->ToObject();
				if (!obj->Parse())
					return false;
			}
		}

		return false;
	}
}
