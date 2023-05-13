// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "CLua.h"	
#include <ShlObj_core.h>
#include <Windows.h>
#include <any>
#include "../../SDK/Includes.hpp"
#include "../../SDK/Math/Math.hpp"
#include "../../SDK/Game/BaseEntity.hpp"
#include "../Log Manager/LogManager.hpp"
#include "../Render.hpp"
#include "menu_item.h"
#include "../Config/Configs.hpp"
#define ZERO Vector(0.0f, 0.0f, 0.0f)
enum FontCenteringFlags
{
	HFONT_CENTERED_NONE = (1 << 0),
	HFONT_CENTERED_X = (1 << 1),
	HFONT_CENTERED_Y = (1 << 2)
};
void lua_panic(sol::optional <std::string> message)
{
	if (!message)
		return;

	//auto log = /*_S*/("Lua error: ") + message.value_or("unknown");
	//eventlogs::get().add(log, false);
}
std::string get_current_script(sol::this_state s)
{
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = source.substr(1).c_str();

	filename.erase(0, 19);

	//"darkraihook\\lua"
	//selling.pw\\scripts\\
	//"selling.pw\\scripts"
	return filename;
}
int get_current_script_id(sol::this_state s)
{
	return c_lua::get().get_script_id(get_current_script(s));
}
std::vector <std::pair <std::string, menu_item>>::iterator find_item(std::vector <std::pair <std::string, menu_item>>& items, const std::string& name)
{
	for (auto it = items.begin(); it != items.end(); ++it)
		if (it->first == name)
			return it;

	return items.end();
}
menu_item find_item(std::vector <std::vector <std::pair <std::string, menu_item>>>& scripts, const std::string& name)
{
	for (auto& script : scripts)
	{
		for (auto& item : script)
		{
			std::string item_name;

			auto first_point = false;
			auto second_point = false;

			for (auto& c : item.first)
			{
				if (c == '.')
				{
					if (first_point)
					{
						second_point = true;
						continue;
					}
					else
					{
						first_point = true;
						continue;
					}
				}

				if (!second_point)
					continue;

				item_name.push_back(c);
			}

			if (item_name == name)
				return item.second;
		}
	}

	return menu_item();
}

namespace ns_menu
{
	auto next_line_counter = 0;

	void next_line(sol::this_state s)
	{
		c_lua::get().items.at(get_current_script_id(s)).emplace_back(std::make_pair(_S("next_line_") + std::to_string(next_line_counter), menu_item()));
		++next_line_counter;
	}
	std::unordered_map <std::string, bool> first_update;
	std::unordered_map <std::string, menu_item> stored_values;
	std::unordered_map <std::string, void*> config_items;

	void add_check_box(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(false)));
	}

	void add_combo_box(sol::this_state s, std::string name, std::vector <const char*> labels) //-V813
	{
		if (labels.empty())
			return;

		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(labels, 0)));
	}

	void add_slider_int(sol::this_state s, const std::string& name, int min, int max)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, min)));
	}

	void add_slider_float(sol::this_state s, const std::string& name, float min, float max)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, min)));
	}

	void add_color_picker(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(Color(255, 255, 255, 255))));
	}

	void add_key_bind(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(0, 0)));
	}

	bool find_config_item(std::string name, std::string type)
	{
		if (config_items.find(name) == config_items.end())
		{
			auto found = false;

			for (auto item : g_Config->items)
			{
				if (item->name == name)
				{
					if (item->type != type)
					{
						//eventlogs::get().add(_S("Lua error: invalid config item type, must be ") + type, false);
						auto log = _S("Lua error: invalid config item type, must be ") + type;
						g_LogManager->PushLog(log);
						return false;
					}

					found = true;
					config_items[name] = item->pointer;
					break;
				}
			}

			if (!found)
			{
				//eventlogs::get().add(_S("Lua error: cannot find config variable \"") + name + '\"', false);

				auto log = _S("Lua error: cannot find config variable \"") + name + '\"';
				g_LogManager->PushLog(log);
				return false;
			}
		}

		return true;
	}

	bool get_bool(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!g_Menu->IsMenuOpened() && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].check_box_value;
			else if (config_items.find(name) != config_items.end())
				return *(bool*)config_items[name];
			else
				return false;
		}

		auto it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, _S("bool")))
				return *(bool*)config_items[name];

			//eventlogs::get().add(_S("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = _S("Lua error: cannot find menu variable \"") + name + '\"';
			g_LogManager->PushLog(log);
			return false;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.check_box_value;
	}

	int get_int(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!g_Menu->IsMenuOpened() && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].type == COMBO_BOX ? stored_values[name].combo_box_value : stored_values[name].slider_int_value;
			else if (config_items.find(name) != config_items.end())
				return *(int*)config_items[name]; //-V206
			else
				return 0;
		}

		auto it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, _S("int")))
				return *(int*)config_items[name]; //-V206

			//eventlogs::get().add(_S("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = _S("Lua error: cannot find menu variable \"") + name + '\"';
			g_LogManager->PushLog(log);
			return 0;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.type == COMBO_BOX ? it.combo_box_value : it.slider_int_value;
	}

	float get_float(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!g_Menu->IsMenuOpened() && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].slider_float_value;
			else if (config_items.find(name) != config_items.end())
				return *(float*)config_items[name];
			else
				return 0.0f;
		}

		auto it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, _S("float")))
				return *(float*)config_items[name];

			//eventlogs::get().add(_S("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = _S("Lua error: cannot find menu variable \"") + name + '\"';
			g_LogManager->PushLog(log);
			return 0.0f;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.slider_float_value;
	}

	Color get_color(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!g_Menu->IsMenuOpened() && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return Color(stored_values[name].color_picker_value[0] * 255.f, stored_values[name].color_picker_value[1] * 255.f, stored_values[name].color_picker_value[2] * 255.f, stored_values[name].color_picker_value[3] * 255.f);
			else if (config_items.find(name) != config_items.end())
				return *(Color*)config_items[name];
			else
				return Color(255, 255, 255, 255);
		}

		auto it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, _S("Color")))
				return *(Color*)config_items[name];

			//eventlogs::get().add(_S("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = _S("Lua error: cannot find menu variable \"") + name + '\"';
			g_LogManager->PushLog(log);

			return Color(255, 255, 255, 255);
		}

		first_update[name] = true;
		stored_values[name] = it;

		return Color(it.color_picker_value[0] * 255.f, it.color_picker_value[1] * 255.f, it.color_picker_value[2] * 255.f, it.color_picker_value[3] * 255.f);
	}

	void set_bool(std::string name, bool value)
	{
		if (!find_config_item(name, _S("bool")))
			return;

		*(bool*)config_items[name] = value;
	}

	void set_int(std::string name, int value)
	{
		if (!find_config_item(name, _S("int")))
			return;

		*(int*)config_items[name] = value; //-V206
	}

	void set_float(std::string name, float value)
	{
		if (!find_config_item(name, _S("float")))
			return;

		*(float*)config_items[name] = value;
	}

	void set_color(std::string name, Color value)
	{
		if (!find_config_item(name, _S("Color")))
			return;

		*(Color*)config_items[name] = value;
	}
}
namespace ns_client
{
	void add_callback(sol::this_state s, std::string eventname, sol::protected_function func)
	{
		if (c_lua::get().loaded.at(get_current_script_id(s)))//new
			c_lua::get().hooks.registerHook(eventname, get_current_script_id(s), func);
	}

	void load_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().load_script(c_lua::get().get_script_id(name));
	}

	void unload_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().unload_script(c_lua::get().get_script_id(name));
	}

	void log(std::string text)
	{
		g_LogManager->PushLog(text);
	}
}
namespace ns_globals
{
	
	std::string get_server_address()
	{
		if (!g_Globals.m_Interfaces.m_EngineClient->IsInGame())
			return "Unknown";

		auto nci = g_Globals.m_Interfaces.m_EngineClient->GetNetChannelInfo();

		if (!nci)
			return "Unknown";

		auto server = nci->GetAddress();

		if (!strcmp(server, "loopback"))
			server = "Local server";
		else if ((*(g_Globals.m_Interfaces.m_GameRules))->IsValveDS())
			server = "Valve server";

		return server;
	}

	float get_realtime()
	{
		return g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
	}

	float get_curtime()
	{
		return g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime;
	}

	float get_frametime()
	{
		return g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime;
	}

	int get_tickcount()
	{
		return g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount;
	}

	int get_framecount()
	{
		return g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount;
	}

	float get_intervalpertick()
	{
		return g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
	}

	int get_maxclients()
	{
		return g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients;
	}
}
namespace ns_engine
{
	static int width, height;

	int get_screen_width()
	{
		g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(width, height);
		return width;
	}

	int get_screen_height()
	{
		g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(width, height);
		return height;
	}

	C_PlayerInfo get_player_info(int i)
	{
		C_PlayerInfo player_info;
		g_Globals.m_Interfaces.m_EngineClient->GetPlayerInfo(i, &player_info);

		return player_info;
	}

	int get_player_for_user_id(int i)
	{
		return g_Globals.m_Interfaces.m_EngineClient->GetPlayerForUserID(i);
	}

	int get_local_player_index()
	{
		return g_Globals.m_Interfaces.m_EngineClient->GetLocalPlayer();
	}

	QAngle get_view_angles()
	{
		QAngle view_angles;
		g_Globals.m_Interfaces.m_EngineClient->GetViewAngles(&view_angles);

		return view_angles;
	}

	void set_view_angles(QAngle view_angles)
	{
		Math::NormalizeAngle(view_angles.pitch);
		Math::NormalizeAngle(view_angles.yaw);
		Math::NormalizeAngle(view_angles.roll);
		g_Globals.m_Interfaces.m_EngineClient->SetViewAngles(&view_angles);
	}

	bool is_in_game()
	{
		return g_Globals.m_Interfaces.m_EngineClient->IsInGame();
	}

	bool is_connected()
	{
		return g_Globals.m_Interfaces.m_EngineClient->IsConnected();
	}

	std::string get_level_name()
	{
		return g_Globals.m_Interfaces.m_EngineClient->GetLevelName();
	}

	std::string get_level_name_short()
	{
		return g_Globals.m_Interfaces.m_EngineClient->GetLevelNameShort();
	}

	std::string get_map_group_name()
	{
		return g_Globals.m_Interfaces.m_EngineClient->GetMapGroupName();
	}

	bool is_playing_demo()
	{
		return g_Globals.m_Interfaces.m_EngineClient->IsPlayingDemo();
	}

	bool is_recording_demo()
	{
		return g_Globals.m_Interfaces.m_EngineClient->IsRecordingDemo();
	}

	bool is_paused()
	{
		return g_Globals.m_Interfaces.m_EngineClient->IsPaused();
	}

	bool is_taking_screenshot()
	{
		return g_Globals.m_Interfaces.m_EngineClient->IsTakingScreenshot();
	}

	bool is_hltv()
	{
		return g_Globals.m_Interfaces.m_EngineClient->IsHLTV();
	}
}
namespace ns_render
{
	Vector world_to_screen(const Vector& world)
	{
		Vector screen;

		if (!Math::WorldToScreen(world, screen))
			return ZERO;

		return screen;
	}

	ImFont* CreateCustomFont(const char* font_name, float size)
	{
		ImFontConfig cfg;
		cfg.PixelSnapH = 0;
		cfg.OversampleH = 5;
		cfg.OversampleV = 5;
		cfg.RasterizerMultiply = 1.2f;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
			0x2DE0, 0x2DFF, // Cyrillic Extended-A
			0xA640, 0xA69F, // Cyrillic Extended-B
			0xE000, 0xE226, // icons
			0,
		};

		cfg.GlyphRanges = ranges;
		return ImGui::GetIO().Fonts->AddFontFromFileTTF("C:/windows/fonts/calibri.ttf", size, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	}

	void RenderRectFilled(float x1, float y1, float x2, float y2, Color aColor, float rounding = 0.f)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImColor(aColor.r(), aColor.b(), aColor.g(), aColor.a()));
	}
	void RenderRect(float x1, float y1, float x2, float y2, Color aColor, float rounding = 0.f)
	{
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), ImColor(aColor.r(), aColor.b(), aColor.g(), aColor.a()));
	}
	void RenderArc(float x, float y, float radius, float min_angle, float max_angle, Color aColor, float thickness)
	{
		ImGui::GetOverlayDrawList()->PathArcTo(ImVec2(x, y), radius, DEG2RAD(min_angle), DEG2RAD(max_angle), 32);
		ImGui::GetOverlayDrawList()->PathStroke(ImColor(aColor.r(), aColor.b(), aColor.g(), aColor.a()), false, thickness);
	}
	void RenderCircle2D(Vector vecPosition, int32_t iPointCount, float_t flRadius, Color aColor)
	{
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(vecPosition.x, vecPosition.y), flRadius, ImColor(aColor.r(), aColor.b(), aColor.g(), aColor.a()), iPointCount);
	}
	void RenderCircle2DFilled(Vector vecPosition, int32_t iPointCount, float_t flRadius, Color aColor)
	{
		ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2(vecPosition.x, vecPosition.y), flRadius, ImColor(aColor.r(), aColor.b(), aColor.g(), aColor.a()), iPointCount);
	}
	void RenderLine(float x1, float y1, float x2, float y2, Color aColor, float_t flThickness)
	{
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImColor(aColor.r(), aColor.b(), aColor.g(), aColor.a()), flThickness);
	}
	void RenderText(ImFont* font, float font_size, float x1, float y1, Color aColor, std::string text)
	{
		if (font)
			ImGui::GetOverlayDrawList()->AddText(font, font_size, ImVec2(x1, y1), ImColor(aColor.r(), aColor.b(), aColor.g(), aColor.a()), text.c_str());
		else
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x1, y1), ImColor(aColor.r(), aColor.b(), aColor.g(), aColor.a()), text.c_str());
	}
}
namespace ns_console
{
	void execute(std::string& command)
	{
		if (command.empty())
			return;

		g_Globals.m_Interfaces.m_EngineClient->ExecuteClientCmd(command.c_str());
	}

	std::unordered_map <std::string, ConVar*> convars;

	bool get_bool(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_Globals.m_Interfaces.m_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = _S("Lua error: cannot find ConVar \"") + convar_name + '\"';
				g_LogManager->PushLog(log);
				return false;
			}
		}

		if (!convars[convar_name])
			return false;

		return convars[convar_name]->GetBool();
	}

	int get_int(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_Globals.m_Interfaces.m_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = _S("Lua error: cannot find ConVar \"") + convar_name + '\"';
				g_LogManager->PushLog(log);
				return 0;
			}
		}

		if (!convars[convar_name])
			return 0;

		return convars[convar_name]->GetInt();
	}

	float get_float(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_Globals.m_Interfaces.m_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = _S("Lua error: cannot find ConVar \"") + convar_name + '\"';
				g_LogManager->PushLog(log);
				return 0.0f;
			}
		}

		if (!convars[convar_name])
			return 0.0f;

		return convars[convar_name]->GetFloat();
	}

	std::string get_string(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_Globals.m_Interfaces.m_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = _S("Lua error: cannot find ConVar \"") + convar_name + '\"';
				g_LogManager->PushLog(log);
				return _S("");
			}
		}

		if (!convars[convar_name])
			return "";

		return convars[convar_name]->GetString();
	}

	void set_bool(const std::string& convar_name, bool value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_Globals.m_Interfaces.m_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = _S("Lua error: cannot find ConVar \"") + convar_name + '\"';
				g_LogManager->PushLog(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetBool() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_int(const std::string& convar_name, int value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_Globals.m_Interfaces.m_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = _S("Lua error: cannot find ConVar \"") + convar_name + '\"';
				g_LogManager->PushLog(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetInt() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_float(const std::string& convar_name, float value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_Globals.m_Interfaces.m_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = _S("Lua error: cannot find ConVar \"") + convar_name + '\"';
				g_LogManager->PushLog(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetFloat() != value) //-V550
			convars[convar_name]->SetValue(value);
	}

	void set_string(const std::string& convar_name, const std::string& value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = g_Globals.m_Interfaces.m_CVar->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = _S("Lua error: cannot find ConVar \"") + convar_name + '\"';
				g_LogManager->PushLog(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetString() != value)
			convars[convar_name]->SetValue(value.c_str());
	}
}
namespace ns_entitylist
{
	sol::optional <C_BasePlayer*> get_local_player()
	{
		if (!g_Globals.m_Interfaces.m_EngineClient->IsInGame())
			return sol::optional <C_BasePlayer*>(sol::nullopt);

		return (C_BasePlayer*)g_Globals.m_Interfaces.m_EntityList->GetClientEntity(g_Globals.m_Interfaces.m_EngineClient->GetLocalPlayer());
	}

	sol::optional <C_BasePlayer*> get_player_by_index(int i)
	{
		if (!g_Globals.m_Interfaces.m_EngineClient->IsInGame())
			return sol::optional <C_BasePlayer*>(sol::nullopt);

		return (C_BasePlayer*)g_Globals.m_Interfaces.m_EntityList->GetClientEntity(i);
	}

}
namespace ns_cmd
{
	int get_choke()
	{
		return g_Globals.m_Interfaces.m_ClientState->m_nChokedCommands();
	}
}

sol::state lua;

void c_lua::initialize()
{
	lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::ffi, sol::lib::jit, sol::lib::utf8);

	lua[/*_S*/("collectgarbage")] = sol::nil;
	lua[/*_S*/("dofilsse")] = sol::nil;
	lua[/*_S*/("load")] = sol::nil;
	lua[/*_S*/("loadfile")] = sol::nil;
	lua[/*_S*/("pcall")] = sol::nil;
	lua[/*_S*/("print")] = sol::nil;
	lua[/*_S*/("xpcall")] = sol::nil;
	lua[/*_S*/("getmetatable")] = sol::nil;
	lua[/*_S*/("setmetatable")] = sol::nil;
	lua[/*_S*/("__nil_callback")] = []() {};

	//lua.new_usertype<C_BaseEntity>(_S("Entity"), // 
	//	(std::string)_S("GetPropInt"), &C_BaseEntity::GetPropInt,
	//	(std::string)_S("GetPropFloat"), &C_BaseEntity::GetPropFloat,
	//	(std::string)_S("GetPropBool"), &C_BaseEntity::GetPropBool,
	//	(std::string)_S("GetPropString"), &C_BaseEntity::GetPropString,
	//	(std::string)_S("SetPropInt"), &C_BaseEntity::SetPropInt,
	//	(std::string)_S("SetPropFloat"), &C_BaseEntity::SetPropFloat,
	//	(std::string)_S("SetPropBool"), &C_BaseEntity::SetPropBool,
	//	(std::string)_S("SetPropString"), &C_BaseEntity::SetPropString
	//	);

	lua.new_usertype <Color>(_S("Color"), sol::constructors <Color(), Color(int, int, int), Color(int, int, int, int)>(),
		(std::string)_S("r"), &Color::r,
		(std::string)_S("g"), &Color::g,
		(std::string)_S("b"), &Color::b,
		(std::string)_S("a"), &Color::a
		);

	lua.new_usertype <Vector>(_S("Vector"), sol::constructors <Vector(), Vector(float, float, float)>(),
		(std::string)_S("x"), &Vector::x,
		(std::string)_S("y"), &Vector::y,
		(std::string)_S("z"), &Vector::z,
		(std::string)_S("Length"), &Vector::Length,
		(std::string)_S("LengthSqr"), &Vector::LengthSqr,
		(std::string)_S("Length2D"), &Vector::Length2D,
		(std::string)_S("Length2DSqr"), &Vector::Length2DSqr,
		(std::string)_S("IsZero"), &Vector::IsZero,
		(std::string)_S("SetZero"), &Vector::Zero,
		(std::string)_S("DistTo"), &Vector::DistTo,
		(std::string)_S("DistToSqr"), &Vector::DistToSqr,
		(std::string)_S("Cross"), &Vector::Cross,
		(std::string)_S("Normalize"), &Vector::Normalize
		);

	lua.new_usertype <C_GameEvent>(_S("GameEvent"),
		(std::string)_S("GetBool"), &C_GameEvent::GetBool,
		(std::string)_S("GetInt"), &C_GameEvent::GetInt,
		(std::string)_S("GetFloat"), &C_GameEvent::GetFloat,
		(std::string)_S("GetString"), &C_GameEvent::GetString,
		(std::string)_S("SetBool"), &C_GameEvent::SetBool,
		(std::string)_S("SetInt"), &C_GameEvent::SetInt,
		(std::string)_S("SetFloat"), &C_GameEvent::SetFloat,
		(std::string)_S("SetString"), &C_GameEvent::SetString
		);

	lua.new_enum(_S("Hitboxes"),
		_S("Head"), HITBOX_HEAD,
		_S("Neck"), HITBOX_NECK,
		_S("Pelvis"), HITBOX_PELVIS,
		_S("Stomach"), HITBOX_STOMACH,
		_S("Chest"), HITBOX_CHEST,
		_S("Upper_Chest"), HITBOX_UPPER_CHEST,
		_S("Right_Thign"), HITBOX_RIGHT_THIGH,
		_S("Left_Thigh"), HITBOX_LEFT_THIGH,
		_S("Right_Calf"), HITBOX_RIGHT_CALF,
		_S("Left_Calf"), HITBOX_LEFT_CALF,
		_S("Right_Foot"), HITBOX_RIGHT_FOOT,
		_S("Left_Foot"), HITBOX_LEFT_FOOT,
		_S("Right_Hand"), HITBOX_RIGHT_HAND,
		_S("Left_Hand"), HITBOX_LEFT_HAND,
		_S("Right_Upper_arm"), HITBOX_RIGHT_UPPER_ARM,
		_S("Right_Forearm"), HITBOX_RIGHT_FOREARM,
		_S("Left_Upper_arm"), HITBOX_LEFT_UPPER_ARM,
		_S("Left_Forearm"), HITBOX_LEFT_FOREARM
	);

	lua.new_usertype<ImFont>(_S("ImFont"));

	lua.new_usertype <C_BasePlayer>(_S("Player"), sol::base_classes, sol::bases<C_BaseEntity>(), //new
		(std::string)_S("GetIndex"), &C_BasePlayer::EntIndex,
		(std::string)_S("IsDormant"), &C_BasePlayer::IsDormant,
		(std::string)_S("GetTeam"), &C_BasePlayer::m_iTeamNum,
		(std::string)_S("IsAlive"), &C_BasePlayer::IsAlive,
		(std::string)_S("GetVelocity"), &C_BasePlayer::m_vecVelocity,
		(std::string)_S("GetOrigin"), &C_BasePlayer::SetAbsoluteOrigin,
		(std::string)_S("GetAngles"), &C_BasePlayer::m_angEyeAngles,
		(std::string)_S("HasHelmet"), &C_BasePlayer::m_bHasHelmet,
		(std::string)_S("HasHeavyArmor"), &C_BasePlayer::m_bHasHeavyArmor,
		(std::string)_S("IsScoped"), &C_BasePlayer::m_bIsScoped,
		(std::string)_S("GetHealth"), &C_BasePlayer::m_iHealth
		//(std::string)_S("GetWeapon"), &C_BasePlayer::GetActiveWeapon
		);

	lua.new_usertype <C_BaseCombatWeapon> (_S("weapon"), sol::base_classes, sol::bases<C_BaseEntity>(),
		(std::string)_S("CanShift"), &C_BaseCombatWeapon::CanShift,
		(std::string)_S("IsGun"), &C_BaseCombatWeapon::IsGun,
		(std::string)_S("IsGrenade"), &C_BaseCombatWeapon::IsGrenade,
		(std::string)_S("IsSMG"), &C_BaseCombatWeapon::IsSMG,
		(std::string)_S("m_iItemDefinitionIndex"), &C_BaseCombatWeapon::m_iItemDefinitionIndex,
		(std::string)_S("m_flNextSecondaryAttack"), &C_BaseCombatWeapon::m_flNextSecondaryAttack,
		(std::string)_S("m_flPostponeFireReadyTime"), &C_BaseCombatWeapon::m_flPostponeFireReadyTime,
		(std::string)_S("m_flNextPrimaryAttack"), &C_BaseCombatWeapon::m_flNextPrimaryAttack,
		(std::string)_S("m_Activity"), &C_BaseCombatWeapon::m_Activity,
		(std::string)_S("GetSpread"), &C_BaseCombatWeapon::GetSpread,
		(std::string)_S("GetInaccuracy"), &C_BaseCombatWeapon::GetInaccuracy,
		(std::string)_S("GetWeaponData"), &C_BaseCombatWeapon::GetWeaponData,
		(std::string)_S("UpdateAccuracyPenalty"), &C_BaseCombatWeapon::UpdateAccuracyPenalty,
		(std::string)_S("m_hWeaponWorldModel"), &C_BaseCombatWeapon::m_hWeaponWorldModel,
		(std::string)_S("m_nZoomLevel"), &C_BaseCombatWeapon::m_nZoomLevel,
		(std::string)_S("m_fThrowTime"), &C_BaseCombatWeapon::m_fThrowTime,
		(std::string)_S("m_flThrowStrength"), &C_BaseCombatWeapon::m_flThrowStrength,
		(std::string)_S("m_iViewModelIndex"), &C_BaseCombatWeapon::m_iViewModelIndex,
		(std::string)_S("m_iWorldModelIndex"), &C_BaseCombatWeapon::m_iWorldModelIndex,
		(std::string)_S("m_flLastShotTime"), &C_BaseCombatWeapon::m_flLastShotTime,
		(std::string)_S("m_fAccuracyPenalty"), &C_BaseCombatWeapon::m_fAccuracyPenalty,
		(std::string)_S("m_flRecoilIndex"), &C_BaseCombatWeapon::m_flRecoilIndex,
		(std::string)_S("m_iClip2"), &C_BaseCombatWeapon::m_iClip2,
		(std::string)_S("m_iClip1"), &C_BaseCombatWeapon::m_iClip1,
		(std::string)_S("IsKnife"), &C_BaseCombatWeapon::IsKnife,
		(std::string)_S("IsRifle"), &C_BaseCombatWeapon::IsRifle,
		(std::string)_S("IsSniper"), &C_BaseCombatWeapon::IsSniper,
		(std::string)_S("m_hThrower"), &C_BaseCombatWeapon::m_hThrower,
		(std::string)_S("m_flDamage"), &C_BaseCombatWeapon::m_flDamage,
		(std::string)_S("m_flDamageRadius"), &C_BaseCombatWeapon::m_flDamageRadius
	);

	lua.new_enum(_S("Buttons"),
		_S("in_attack"), IN_ATTACK,
		_S("in_jump"), IN_JUMP,
		_S("in_duck"), IN_DUCK,
		_S("in_forward"), IN_FORWARD,
		_S("in_back"), IN_BACK,
		_S("in_use"), IN_USE,
		_S("in_cancel"), IN_CANCEL,
		_S("in_left"), IN_LEFT,
		_S("in_right"), IN_RIGHT,
		_S("in_moveleft"), IN_MOVELEFT,
		_S("in_moveright"), IN_MOVERIGHT,
		_S("in_attack2"), IN_ATTACK2,
		_S("in_run"), IN_RUN,
		_S("in_reload"), IN_RELOAD,
		_S("in_alt1"), IN_ALT1,
		_S("in_alt2"), IN_ALT2,
		_S("in_score"), IN_SCORE,
		_S("in_speed"), IN_SPEED,
		_S("in_walk"), IN_WALK,
		_S("in_zoom"), IN_ZOOM,
		_S("in_weapon1"), IN_WEAPON1,
		_S("in_weapon2"), IN_WEAPON2,
		_S("in_bullrush"), IN_BULLRUSH,
		_S("in_grenade1"), IN_GRENADE1,
		_S("in_grenade2"), IN_GRENADE2
	);

	auto client = lua.create_table();
	client[_S("add_callback")] = ns_client::add_callback;
	client[_S("load_script")] = ns_client::load_script;
	client[_S("unload_script")] = ns_client::unload_script;
	client[_S("add_log")] = ns_client::log;

	auto globals = lua.create_table();
	globals[_S("get_server_address")] = ns_globals::get_server_address;
	globals[_S("get_realtime")] = ns_globals::get_realtime;
	globals[_S("get_curtime")] = ns_globals::get_curtime;
	globals[_S("get_frametime")] = ns_globals::get_frametime;
	globals[_S("get_tickcount")] = ns_globals::get_tickcount;
	globals[_S("get_framecount")] = ns_globals::get_framecount;
	globals[_S("get_intervalpertick")] = ns_globals::get_intervalpertick;
	globals[_S("get_maxclients")] = ns_globals::get_maxclients;

	auto engine = lua.create_table();
	engine[_S("get_screen_width")] = ns_engine::get_screen_width;
	engine[_S("get_screen_height")] = ns_engine::get_screen_height;
	engine[_S("get_level_name")] = ns_engine::get_level_name;
	engine[_S("get_level_name_short")] = ns_engine::get_level_name_short;
	engine[_S("get_local_player_index")] = ns_engine::get_local_player_index;
	engine[_S("get_map_group_name")] = ns_engine::get_map_group_name;
	engine[_S("get_player_for_user_id")] = ns_engine::get_player_for_user_id;
	engine[_S("get_player_info")] = ns_engine::get_player_info;
	engine[_S("get_view_angles")] = ns_engine::get_view_angles;
	engine[_S("is_connected")] = ns_engine::is_connected;
	engine[_S("is_hltv")] = ns_engine::is_hltv;
	engine[_S("is_in_game")] = ns_engine::is_in_game;
	engine[_S("is_paused")] = ns_engine::is_paused;
	engine[_S("is_playing_demo")] = ns_engine::is_playing_demo;
	engine[_S("is_recording_demo")] = ns_engine::is_recording_demo;
	engine[_S("is_taking_screenshot")] = ns_engine::is_taking_screenshot;
	engine[_S("set_view_angles")] = ns_engine::set_view_angles;

	auto render = lua.create_table();
	render[_S("WorldToScreen")] = ns_render::world_to_screen;
	render[_S("CreateFont")] = ns_render::CreateCustomFont;
	render[_S("RenderRectFilled")] = ns_render::RenderRectFilled;
	render[_S("RenderRect")] = ns_render::RenderRect;
	render[_S("RenderArc")] = ns_render::RenderArc;
	render[_S("RenderCircle2D")] = ns_render::RenderCircle2D;
	render[_S("RenderCircle2DFilled")] = ns_render::RenderCircle2DFilled;
	render[_S("RenderLine")] = ns_render::RenderLine;
	render[_S("RenderText")] = ns_render::RenderText;


	auto entitylist = lua.create_table();
	entitylist[_S("get_local_player")] = ns_entitylist::get_local_player;
	entitylist[_S("get_player_by_index")] = ns_entitylist::get_player_by_index;

	auto menu = lua.create_table();
	menu[_S("AddCheckbox")] = ns_menu::add_check_box;
	menu[_S("AddCombo")] = ns_menu::add_combo_box;
	menu[_S("AddSliderInt")] = ns_menu::add_slider_int;
	menu[_S("AddSliderFloat")] = ns_menu::add_slider_float;
	menu[_S("AddColorPicker")] = ns_menu::add_color_picker;
	menu[_S("GetBool")] = ns_menu::get_bool;
	menu[_S("GetFloat")] = ns_menu::get_float;
	menu[_S("GetInt")] = ns_menu::get_int;
	menu[_S("GetColor")] = ns_menu::get_color;
	menu[_S("SetBool")] = ns_menu::set_bool;
	menu[_S("SetFloat")] = ns_menu::set_float;
	menu[_S("SetInt")] = ns_menu::set_int;
	menu[_S("SetColor")] = ns_menu::set_color;

	auto cmd = lua.create_table();
	cmd[_S("get_choke")] = ns_cmd::get_choke;

	lua[_S("Cheat")] = client;
	lua[_S("Globals")] = globals;
	lua[_S("Engine")] = engine;
	lua[_S("Ui")] = menu;
	lua[_S("Draw")] = render;
	lua[_S("EntList")] = entitylist;
	lua[_S("Cmd")] = cmd;
	refresh_scripts();
}

int c_lua::get_script_id(const std::string& name)
{
	for (auto i = 0; i < scripts.size(); i++)
		if (scripts.at(i) == name) //-V106
			return i;

	return -1;
}

void c_lua::refresh_scripts()
{
	auto oldLoaded = loaded;
	auto oldScripts = scripts;

	loaded.clear();
	scripts.clear();
	std::string folder;

	auto get_dir = [&folder]() -> void
	{
		static TCHAR path[MAX_PATH];

		folder = "selling.pw\\scripts\\";

		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	scripts.clear();

	std::string path = folder + ("/*.lua");
	WIN32_FIND_DATA fd;

	HANDLE hFind = FindFirstFile(path.c_str(), &fd);
	auto i = 0;
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				//auto path = entry.path();
				auto filename = fd.cFileName;

				auto didPut = false;

				for (auto i = 0; i < oldScripts.size(); i++)
				{
					if (filename == oldScripts.at(i)) //-V106
					{
						loaded.emplace_back(oldLoaded.at(i)); //-V106
						didPut = true;
					}
				}

				if (!didPut)
					loaded.emplace_back(false);

				//pathes.emplace_back(path);
				scripts.emplace_back(filename);

				items.emplace_back(std::vector <std::pair <std::string, menu_item>>());
				++i;
			}
		} while (FindNextFile(hFind, &fd));

		FindClose(hFind);
	}
}

void c_lua::load_script(int id)
{
	if (id == -1)
		return;

	if (loaded.at(id)) //-V106
		return;

	auto path = get_script_path(id);

	if (path == /*_S*/(""))
		return;

	auto error_load = false;
	loaded.at(id) = true;
	lua.script_file(path,
		[&error_load](lua_State*, sol::protected_function_result result)
		{
			if (!result.valid())
			{
				sol::error error = result;
				auto log = /*_S*/("Lua error: ") + (std::string)error.what();

				g_LogManager->PushLog({ log });
				error_load = true;

			}

			return result;
		}
	);

	if (error_load | loaded.at(id) == false)
	{
		loaded.at(id) = false;
		return;
	}
}

void c_lua::unload_script(int id)
{
	if (id == -1)
		return;

	if (!loaded.at(id)) //-V106
		return;

	items.at(id).clear(); //-V106

	if (c_lua::get().events.find(id) != c_lua::get().events.end()) //-V807
		c_lua::get().events.at(id).clear();

	hooks.unregisterHooks(id);
	loaded.at(id) = false; //-V106
}

void c_lua::reload_all_scripts()
{
	for (auto current : scripts)
	{
		if (!loaded.at(get_script_id(current))) //-V106
			continue;

		unload_script(get_script_id(current));
		load_script(get_script_id(current));
	}
}

void c_lua::unload_all_scripts()
{
	for (auto s : scripts)
		unload_script(get_script_id(s));
}

std::string c_lua::get_script_path(const std::string& name)
{
	return get_script_path(get_script_id(name));
}

std::string c_lua::get_script_path(int id)
{
	if (id == -1)
		return /*_S*/("");

	std::stringstream asd;

	asd << "selling.pw\\scripts\\" << scripts.at(id).c_str();
	//ogclub.pw

	return asd.str().c_str(); //-V106
}