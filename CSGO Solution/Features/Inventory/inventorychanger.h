#pragma once
#define START_MUSICKIT_INDEX 1500000
#define START_ITEM_INDEX     2000000
#include <string>
#include "parser.h"
#include <map>
#include <unordered_map>
#include "../../SDK/Utils/Enums.hpp"
#include <d3d9.h>

struct wskin
{
	int wId;
	int paintKit;
	int quality;
	float wear;
	int seed;
	std::string name;
	bool m_bInUseT;
	bool m_bInUseCT;
	int stattrak;

	LPDIRECT3DTEXTURE9 texture;
	int rarity;
	int sicker[4];
	std::string build_patch;
};

struct skinInfo
{
	skinInfo()
	{
		weaponName.clear();
		SkinName = {};
		cdnName = {};
		rarity = -1;
	}
	std::string SkinName;
	std::string cdnName;
	std::vector<std::string> weaponName;
	int rarity;
};

class inv_fix
{
public:
	struct xd
	{
		xd()
		{
			skinInfo = { };
			itemCount = 0;
			enabled = false;
		}

		std::unordered_map<int, skinInfo> skinInfo;
		int itemCount = 0;
		bool enabled = false;
	} inventory;
};
struct stickers_oops
{
	std::string name;
	int paintkit;
};

inline std::map<int, stickers_oops> g_Stickers;
inline inv_fix _inv;

inline std::unordered_map<int, wskin> g_InventorySkins;

struct wmedal
{
	int paintKit;
	int equip_state;
};

inline std::map<int, wmedal> g_MedalSkins;

struct wmedalp
{
	std::string name;
	int paintKit;
};

inline std::map<int, wmedalp> fosso;

class ProtoWriter;
class Inventory
{
public:
	void FixNullInventory(ProtoWriter& cache);
	void ClearEquipState(ProtoWriter& object);
	void AddAllItems(ProtoWriter& object);
	void AddItem(ProtoWriter& object, int index, int itemIndex, int rarity, int paintKit, int seed, float wear, std::string name, int ctickers[4], int inusefix = 0);
	bool Presend(uint32_t& unMsgType, void* pubData, uint32_t& cubData);
	static int GetAvailableClassID(int definition_index);
	std::string Changer(void* pubDest, uint32_t* pcubMsgSize);
	void ParseSkins() const;
	void force_full_update();
	LPDIRECT3DTEXTURE9 GetTexture(std::string weapon, skinInfo* item);
	LPDIRECT3DTEXTURE9 GetTextureByBuiledePatch(std::string path);

	std::string GetBuildPatch(std::string weapon, skinInfo* item);

	int GetRarityByItemId(int itemid);
	int GetProperRarity();
	void Proceed();
	void Clear();
	bool CreateTexture(const char* path, LPDIRECT3DTEXTURE9* pImage);

	std::vector<std::string> Errors = {};
	std::deque<std::string> Queue = {};
	std::map<std::string, LPDIRECT3DTEXTURE9> Textures = {};
};

inline Inventory* g_Inventory = new Inventory();

static const char* zweaponnames(const short id)
{
	switch (id)
	{
	case WEAPON_DEAGLE:
		return "deagle";
	case WEAPON_ELITE:
		return "elite";
	case WEAPON_FIVESEVEN:
		return "fiveseven";
	case WEAPON_GLOCK:
		return "glock";
	case WEAPON_AK47:
		return "ak47";
	case WEAPON_AUG:
		return "aug";
	case WEAPON_AWP:
		return "awp";
	case WEAPON_FAMAS:
		return "famas";
	case WEAPON_G3SG1:
		return "g3sg1";
	case WEAPON_GALILAR:
		return "galilar";
	case WEAPON_M249:
		return "m249";
	case WEAPON_M4A1_SILENCER:
		return "m4a1_silencer";
	case WEAPON_M4A1:
		return "m4a1";
	case WEAPON_MAC10:
		return "mac10";
	case WEAPON_P90:
		return "p90";
	case WEAPON_UMP45:
		return "ump45";
	case WEAPON_XM1014:
		return "xm1014";
	case WEAPON_BIZON:
		return "bizon";
	case WEAPON_MAG7:
		return "mag7";
	case WEAPON_NEGEV:
		return "negev";
	case WEAPON_SAWEDOFF:
		return "sawedoff";
	case WEAPON_TEC9:
		return "tec9";
	case WEAPON_HKP2000:
		return "hkp2000";
	case WEAPON_MP5SD:
		return "mp5sd";
	case WEAPON_MP7:
		return "mp7";
	case WEAPON_MP9:
		return "mp9";
	case WEAPON_NOVA:
		return "nova";
	case WEAPON_P250:
		return "p250";
	case WEAPON_SCAR20:
		return "scar20";
	case WEAPON_SG553:
		return "sg556";
	case WEAPON_SSG08:
		return "ssg08";
	case WEAPON_USP_SILENCER:
		return "usp_silencer";
	case WEAPON_CZ75A:
		return "cz75a";
	case WEAPON_REVOLVER:
		return "revolver";
	case WEAPON_KNIFE:
		return "knife";
	case WEAPON_KNIFE_T:
		return "knife_t";
	case WEAPON_KNIFE_M9_BAYONET:
		return "knife_m9_bayonet";
	case WEAPON_KNIFE_BAYONET:
		return "bayonet";
	case WEAPON_KNIFE_FLIP:
		return "knife_flip";
	case WEAPON_KNIFE_GUT:
		return "knife_gut";
	case WEAPON_KNIFE_KARAMBIT:
		return "knife_karambit";
	case WEAPON_KNIFE_TACTICAL:
		return "knife_tactical";
	case WEAPON_KNIFE_FALCHION:
		return "knife_falchion";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "knife_survival_bowie";
	case WEAPON_KNIFE_BUTTERFLY:
		return "knife_butterfly";
	case WEAPON_KNIFE_PUSH:
		return "knife_push";
	case WEAPON_KNIFE_URSUS:
		return "knife_ursus";
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
		return "knife_gypsy_jackknife";
	case WEAPON_KNIFE_STILETTO:
		return "knife_stiletto";
	case WEAPON_KNIFE_WIDOWMAKER:
		return "knife_widowmaker";
	case WEAPON_KNIFE_SKELETON:
		return "knife_skeleton";
	case WEAPON_KNIFE_OUTDOOR:
		return "knife_outdoor";
	case WEAPON_KNIFE_CANIS:
		return "knife_canis";
	case WEAPON_KNIFE_CORD:
		return "knife_cord";
	case WEAPON_KNIFE_CSS:
		return "knife_css";
	case GLOVE_STUDDED_BLOODHOUND:
		return "studded_bloodhound_gloves";
	case GLOVE_T_SIDE:
		return "t_gloves";
	case GLOVE_CT_SIDE:
		return "ct_gloves";
	case GLOVE_SPORTY:
		return "sporty_gloves";
	case GLOVE_SLICK:
		return "slick_gloves";
	case GLOVE_LEATHER_WRAP:
		return "leather_handwraps";
	case GLOVE_MOTORCYCLE:
		return "motorcycle_gloves";
	case GLOVE_SPECIALIST:
		return "specialist_gloves";
	case GLOVE_HYDRA:
		return "studded_hydra_gloves";
	default:
		return "";
	}
}
