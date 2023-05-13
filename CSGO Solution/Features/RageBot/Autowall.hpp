#pragma once

#include "../SDK/Includes.hpp"
#include "../SDK/Math/Math.hpp"
//#include "../SDK/Game/BaseCombatWeapon.hpp"

namespace penetration {
	struct PenetrationInput_t
	{
		C_BasePlayer* m_from;
		C_BasePlayer* m_target;
		Vector  m_pos;
		Vector  m_start;
		float	m_damage;
		float   m_damage_pen;
		bool	m_can_pen;
	};

	struct FireBulletData_t
	{
		Vector m_start;
		Vector m_end;
		Vector m_current_position;
		Vector m_direction;

		CTraceFilter* m_filter;
		CGameTrace m_enter_trace;

		float m_thickness;
		float m_current_damage;
		int m_penetration_count;
	};

	struct PenetrationOutput_t {
		C_BasePlayer* m_target;
		float   m_damage;
		int     m_hitgroup;
		bool    m_pen;

		__forceinline PenetrationOutput_t() : m_target{ nullptr }, m_damage{ 0.f }, m_hitgroup{ -1 }, m_pen{ false } {}
	};

	bool HandleBulletPenetration(C_CSWeaponData* info, FireBulletData_t& data, bool extracheck, Vector point);
	void TraceLine(Vector& start, Vector& end, unsigned int mask, C_BasePlayer* ignore, CGameTrace* trace);
	bool CanHitFloatingPoint(const Vector& point, const Vector& source);

	float scale(C_BasePlayer* player, float damage, float armor_ratio, int hitgroup);
	bool  TraceToExit(const Vector& start, const Vector& dir, Vector& out, CGameTrace* enter_trace, CGameTrace* exit_trace);
	void  ClipTraceToPlayer(const Vector& start, const Vector& end, uint32_t mask, CGameTrace* tr, C_BasePlayer* player, float min);
	bool  run(PenetrationInput_t* in, PenetrationOutput_t* out);
}