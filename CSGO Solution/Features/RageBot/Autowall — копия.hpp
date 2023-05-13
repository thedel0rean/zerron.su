#pragma once

#include <deque>
#include "../SDK/Includes.hpp"
#include "../SDK/Math/Math.hpp"

struct AutoWallInfo {
	int iDamage = -1;
	int iHitgroup = -1;
	int iWalls = 4;
	bool bDidPenerate = false;
	float flTickness = 1.f;
	C_BasePlayer* pPlayer = 0;
	Vector End;
};

struct PenetrationData_t
{
	Vector m_vecShootPosition = Vector(0, 0, 0);
	Vector m_vecTargetPosition = Vector(0, 0, 0);
	Vector m_vecDirection = Vector(0, 0, 0);

	int32_t m_PenetrationCount = 4;

	float_t m_flPenetrationPower = 0.0f;
	float_t m_flPenetrationDistance = 0.0f;

	float_t m_flDamageModifier = 0.5f;
	float_t m_flPenetrationModifier = 1.0f;

	float_t m_flMaxRange = 0.0f;
	float_t m_flWeaponDamage = 0.0f;
	float_t m_flCurrentDamage = 0.0f;
	float_t m_flCurrentDistance = 0.0f;

	CGameTrace m_EnterTrace;
	CGameTrace m_ExitTrace;

	C_CSWeaponData* m_WeaponData = NULL;
	C_BaseCombatWeapon* m_Weapon = NULL;
};

struct AutoWallBullet {
	Vector vecStart;
	Vector vecEnd;
	Vector vecPos;
	Vector vecDir;

	ITraceFilter* Filter = nullptr;
	trace_t       Trace;

	float  flThickness = 1.f;
	float  flDamage = 1.f;
	int    iWalls = 4;
};

class AutoWall
{
public:
	AutoWallInfo Start(Vector start, Vector end, C_BasePlayer* from, C_BasePlayer* to);
	//virtual float GetPointDamage(Vector point, C_BasePlayer* e);

	void ScaleDamage(trace_t* Trace, C_BasePlayer* player, int& iHitgroup, float& flDamage);
	void ClipTraceToPlayers(Vector vecStart, Vector vecEnd, C_BasePlayer* player, unsigned int nMask, CGameTrace* Trace);
	bool HandleBulletPenetration(C_CSWeaponData* inf, AutoWallBullet& bullet);
	bool TraceToExit(trace_t* pEnterTrace, Vector vecStart, Vector vecDirection, trace_t* pExitTrace);
	bool IsBreakable(C_BasePlayer* player);

	virtual Vector GetPointDirection(Vector vecShootPosition, Vector vecTargetPosition);
	virtual float GetPointDamage(Vector vecShootPosition, Vector vecTargetPosition);
	static bool has_armour(C_BasePlayer* player, int hitgroup);
private:
	PenetrationData_t m_PenetrationData;
};

inline AutoWall* g_AutoWall = new AutoWall();