#pragma once
#include "../SDK/Includes.hpp"
#include "../SDK/Math/Math.hpp"
#include "Autowall.hpp"
#include "../Tools/Tools.hpp"

bool AutoWall::IsBreakable(C_BasePlayer* e) {
	using func = bool(__fastcall*)(C_BasePlayer*);
	static auto fn = reinterpret_cast<func>(g_Tools->GetInterface(GetModuleHandleA("client.dll"), "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE"));

	if (!e || !e->EntIndex())
		return false;

	auto take_damage{ (char*)((uintptr_t)e + *(size_t*)((uintptr_t)fn + 38)) };
	auto take_damage_backup{ *take_damage };

	auto cclass = e->GetClientClass();

	if ((cclass->m_strNetworkName[1]) != 'F'
		|| (cclass->m_strNetworkName[4]) != 'c'
		|| (cclass->m_strNetworkName[5]) != 'B'
		|| (cclass->m_strNetworkName[9]) != 'h')
		*take_damage = 2;

	auto breakable = fn(e);
	*take_damage = take_damage_backup;

	return breakable;
}

bool AutoWall::has_armour(C_BasePlayer* player, int hitgroup)
{
	if (hitgroup == HITGROUP_HEAD && player->m_bHasHelmet())
		return true;
	else if (hitgroup >= HITGROUP_CHEST && hitgroup <= HITGROUP_RIGHTARM)
		return true;

	return false;
}

float AutoWall::GetPointDamage(Vector vecShootPosition, Vector vecTargetPosition)
{
	if (!m_PenetrationData.m_Weapon || !m_PenetrationData.m_WeaponData)
		return false;

	m_PenetrationData.m_vecShootPosition = vecShootPosition;
	m_PenetrationData.m_vecTargetPosition = vecTargetPosition;
	m_PenetrationData.m_vecDirection = GetPointDirection(vecShootPosition, vecTargetPosition);
	m_PenetrationData.m_flCurrentDamage = m_PenetrationData.m_flWeaponDamage;
	m_PenetrationData.m_flCurrentDistance = 0.0f;
	m_PenetrationData.m_PenetrationCount = 4;

	m_PenetrationData.m_flDamageModifier = 0.5f;
	m_PenetrationData.m_flPenetrationModifier = 1.0f;

	return 0.0f;
}

Vector AutoWall::GetPointDirection(Vector vecShootPosition, Vector vecTargetPosition)
{
	Vector vecDirection;
	QAngle angDirection;

	Math::VectorAngles(vecTargetPosition - vecShootPosition, angDirection);
	Math::AngleVectors(angDirection, vecDirection);

	vecDirection.NormalizeInPlace();
	return vecDirection;
}

void AutoWall::ScaleDamage(trace_t* Trace, C_BasePlayer* player, int& iHitgroup, float& flDamage) {

	auto weapon = g_Globals.m_LocalPlayer->m_hActiveWeapon();
	bool has_heavy = false;
	int armor_value = ((C_BasePlayer*)Trace->hit_entity)->m_ArmourValue();
	int hit_group = Trace->hitgroup;

	if (!g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive())
		return;

	if (/*weapon->IsTaser() || */weapon->IsKnife() || weapon->IsGrenade())
		return;

	auto is_armored = [&Trace]()->bool
	{
		auto* target = (C_BasePlayer*)Trace->hit_entity;
		switch (Trace->hitgroup)
		{
		case HITGROUP_HEAD:
			return !!target->m_bHasHelmet();
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (hit_group)
	{
	case HITGROUP_HEAD:
		if (player->m_bHasHeavyArmor())
			flDamage *= 2.0f;
		else
			flDamage *= 4.f;
		break;
	case HITGROUP_CHEST:
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:

		flDamage *= 1.f;
		break;
	case HITGROUP_STOMACH:
		flDamage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		flDamage *= 0.75f;
		break;
	default:
		break;
	}

	if (armor_value > 0 && is_armored()) {
		float bonus_value = 1.f, armor_bonus_ratio = 0.5f, armor_ratio = player->m_ArmourValue() / 2.f;

		if (has_heavy)
		{
			armor_bonus_ratio = 0.33f;
			armor_ratio *= 0.5f;
			bonus_value = 0.33f;
		}

		auto new_damage = flDamage * armor_ratio;

		if (((flDamage - (flDamage * armor_ratio)) * (bonus_value * armor_bonus_ratio)) > armor_value)
		{
			new_damage = flDamage - (armor_value / armor_bonus_ratio);
		}

		flDamage = new_damage;
	}


}

void AutoWall::ClipTraceToPlayers(Vector vecStart, Vector vecEnd, C_BasePlayer* player, unsigned int nMask, CGameTrace* Trace)
{
	if (!player)
		return;

	auto mins = player->GetCollideable()->OBBMins();
	auto maxs = player->GetCollideable()->OBBMaxs();

	Vector dir(vecEnd - vecStart);
	Vector center = (maxs + mins) / 2, pos(center + player->GetAbsOrigin());

	auto to = pos - vecStart;
	auto range_along = dir.Dot(to);

	float range;
	if (range_along < 0.f)
		range = -to.Length();
	else if (range_along > dir.Length())
		range = -(pos - vecEnd).Length();
	else {
		auto ray(pos - (dir * range_along + vecStart));
		range = ray.Length();
	}

	if (range <= 60.f) {
		trace_t newtrace;

		Ray_t ray;
		ray.Init(vecStart, vecEnd);

		g_Globals.m_Interfaces.m_EngineTrace->ClipRayToCollideable(ray, nMask, player->GetCollideable(), &newtrace);

		if (Trace->fraction > newtrace.fraction)
			*Trace = newtrace;
	}
}

bool AutoWall::HandleBulletPenetration(C_CSWeaponData* Inf, AutoWallBullet& Bullet) {
	trace_t trace;

	auto sdata = g_Globals.m_Interfaces.m_PropPhysics->GetSurfaceData(Bullet.Trace.surface.surfaceProps);
	auto mat = sdata->game.material;

	auto sp_mod = sdata->game.flPenetrationModifier; //*(float*)((uintptr_t)sdata + 88);
	auto dmg_mod = sdata->game.flDamageModifier;
	auto pnt_mod = 0.f;

	auto solid_surf = (Bullet.Trace.contents >> 3) & CONTENTS_SOLID;
	auto light_surf = (Bullet.Trace.surface.flags >> 7) & SURF_LIGHT;

	if (Bullet.iWalls <= 0
		|| (!Bullet.iWalls && !light_surf && !solid_surf && mat != CHAR_TEX_GLASS && mat != CHAR_TEX_GRATE)
		|| Inf->m_flPenetration <= 0.f
		|| (!this->TraceToExit(&Bullet.Trace, Bullet.Trace.endpos, Bullet.vecDir, &trace)
			&& !(g_Globals.m_Interfaces.m_EngineTrace->GetPointContents(Bullet.Trace.endpos, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL) & (MASK_SHOT_HULL | CONTENTS_HITBOX))))
		return false;

	auto e_sdata = g_Globals.m_Interfaces.m_PropPhysics->GetSurfaceData(trace.surface.surfaceProps);
	auto e_mat = e_sdata->game.material;
	auto e_sp_mod = e_sdata->game.flPenetrationModifier;

	if (mat == CHAR_TEX_GRATE || mat == CHAR_TEX_GLASS)
	{
		pnt_mod = 3.f;
		dmg_mod = 0.05f;
	}
	else if (light_surf || solid_surf)
	{
		pnt_mod = 1.f;
		dmg_mod = 0.16f;
	}
	else if (mat == CHAR_TEX_FLESH)
	{
		pnt_mod = 1.f;
		dmg_mod = 0.16f;
	}
	else
	{
		pnt_mod = (sp_mod + e_sp_mod) / 2.f;
		dmg_mod = 0.16f;
	}

	if (mat == e_mat)
	{
		if (e_mat == CHAR_TEX_CARDBOARD || e_mat == CHAR_TEX_WOOD)
			pnt_mod = 3.f;
		else if (e_mat == CHAR_TEX_PLASTIC)
			pnt_mod = 2.f;
	}

	auto thickness = (trace.endpos - Bullet.Trace.endpos).LengthSqr();
	auto modifier = fmaxf(0.f, 1.f / pnt_mod);

	auto lost_damage = fmaxf(
		((modifier * thickness) / 24.f)
		+ ((Bullet.flDamage * dmg_mod)
			+ (fmaxf(3.75f / Inf->m_flPenetration, 0.f) * 3.f * modifier)), 0.f);

	if (lost_damage > Bullet.flDamage)
		return false;

	if (lost_damage > 0.f)
		Bullet.flDamage -= lost_damage;

	if (Bullet.flDamage < 1.f)
		return false;

	Bullet.vecPos = trace.endpos;
	Bullet.iWalls--;

	return true;
}

bool AutoWall::TraceToExit(trace_t* pEnterTrace, Vector vecStart, Vector vecDirection, trace_t* pExitTrace)
{

	Vector end;
	auto distance = 0.f;
	auto distance_check = 23;
	auto first_contents = 0;

	do {
		distance += 4.f;
		end = vecStart + vecDirection * distance;

		if (!first_contents)
			first_contents = g_Globals.m_Interfaces.m_EngineTrace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		auto point_contents = g_Globals.m_Interfaces.m_EngineTrace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);
		if (!(point_contents & (MASK_SHOT_HULL /*| CONTENTS_HITBOX*/)) || (point_contents & CONTENTS_HITBOX && point_contents != first_contents)) {
			auto new_end = end - (vecDirection * 4.f);

			Ray_t ray;
			ray.Init(end, new_end);

			g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, nullptr, pExitTrace);

			if (pExitTrace->startsolid && pExitTrace->surface.flags & SURF_HITBOX) {
				Ray_t ray1;
				ray1.Init(vecStart, end);

				CTraceFilter filter;
				filter.pSkip = pExitTrace->hit_entity;

				g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, pExitTrace);

				if (pExitTrace->DidHit() && !pExitTrace->startsolid)
					return true;

				continue;
			}

			if (pExitTrace->DidHit() && !pExitTrace->startsolid) {
				if (pEnterTrace->surface.flags & SURF_NODRAW || !(pExitTrace->surface.flags & SURF_NODRAW)) {
					if (pExitTrace->plane.normal.Dot(vecDirection) <= 1.f)
						return true;

					continue;
				}

				if (this->IsBreakable((C_BasePlayer*)pEnterTrace->hit_entity) && this->IsBreakable((C_BasePlayer*)pExitTrace->hit_entity))
					return true;

				continue;
			}

			if (pExitTrace->surface.flags & SURF_NODRAW) {
				if (this->IsBreakable((C_BasePlayer*)pEnterTrace->hit_entity) && this->IsBreakable((C_BasePlayer*)pExitTrace->hit_entity))
					return true;
				else if (!(pEnterTrace->surface.flags & SURF_NODRAW))
					continue;
			}

			if ((!pEnterTrace->hit_entity || pEnterTrace->hit_entity->EntIndex() == 0) && this->IsBreakable((C_BasePlayer*)pExitTrace->hit_entity)) {
				pExitTrace = pEnterTrace;
				pExitTrace->endpos = vecStart + vecDirection;
				return true;
			}

			continue;
		}

		distance_check--;
	} while (distance_check);

	return false;

}


AutoWallInfo AutoWall::Start(Vector start, Vector end, C_BasePlayer* from, C_BasePlayer* to) {
	AutoWallInfo rt{};

	AutoWallBullet bullet;
	bullet.vecStart = start;
	bullet.vecEnd = end;
	bullet.vecPos = start;
	bullet.flThickness = 0.f;
	bullet.iWalls = 4;
	//bullet.vecDir = Math::CalcAngle(start, end).vector();

	auto flt_player = CTraceFilterOneEntity();
	flt_player.pEntity = to;

	auto flt_self = CTraceFilter();
	flt_self.pSkip = from;

	if (to)
		bullet.Filter = &flt_player;
	else
		bullet.Filter = &flt_self;

	auto wep = g_Globals.m_LocalPlayer->m_hActiveWeapon();
	if (!wep)
		return rt;

	auto inf = wep->GetWeaponData();
	if (!inf)
		return rt;

	end = start + bullet.vecDir * (inf->m_iWeaponType == WEAPONTYPE_KNIFE ? 45.f : inf->m_flRange);
	bullet.flDamage = inf->m_iDamage;

	while (bullet.flDamage > 0 && bullet.iWalls > 0) {
		rt.iWalls = bullet.iWalls;

		Ray_t ray;
		ray.Init(bullet.vecPos, end);

		CTraceFilter filter;
		filter.pSkip = from;

		g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &bullet.Trace);
		this->ClipTraceToPlayers(bullet.vecPos, bullet.vecPos + bullet.vecDir * 40.f, to, MASK_SHOT | CONTENTS_GRATE, &bullet.Trace);

		bullet.flDamage *= powf(inf->m_flRangeModifier, (bullet.Trace.endpos - start).Length() / 500.f);

		if (bullet.Trace.fraction == 1.f) {
			rt.iDamage = bullet.flDamage;
			rt.iHitgroup = -1;
			rt.End = bullet.Trace.endpos;
			rt.pPlayer = nullptr;
		}

		if (bullet.Trace.hitgroup > 0 && bullet.Trace.hitgroup <= 7) {
			if ((to && bullet.Trace.hit_entity->EntIndex() != to->EntIndex()) ||
				((C_BasePlayer*)bullet.Trace.hit_entity)->m_iTeamNum() == from->m_iTeamNum()) {
				rt.iDamage = -1;

				return rt;
			}

			this->ScaleDamage(&bullet.Trace, from, bullet.Trace.hitgroup, bullet.flDamage);

			rt.iDamage = bullet.flDamage;
			rt.iHitgroup = bullet.Trace.hitgroup;
			rt.End = bullet.Trace.endpos;
			rt.pPlayer = (C_BasePlayer*)bullet.Trace.hit_entity;

			break;
		}

		if (!this->HandleBulletPenetration(inf, bullet))
			break;

		rt.bDidPenerate = true;
	}

	rt.iWalls = bullet.iWalls;
	return rt;
}


/*float AutoWall::GetPointDamage(Vector point, C_BasePlayer* e) {
	return  g_AutoWall->Start(e->m_vecOrigin() + e->m_vecViewOffset(), point, g_Globals.m_LocalPlayer, e).iDamage;
}*/