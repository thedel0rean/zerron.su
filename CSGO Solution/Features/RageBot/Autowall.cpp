#include "Autowall.hpp"
#include "../Tools/Tools.hpp"

float penetration::scale(C_BasePlayer* player, float damage, float armor_ratio, int hitgroup) {
	bool  has_heavy_armor;
	int   armor;
	float heavy_ratio, bonus_ratio, ratio, new_damage;

	static auto is_armored = [](C_BasePlayer* player, int armor, int hitgroup) {
		// the player has no armor.
		if (armor <= 0)
			return false;

		// if the hitgroup is head and the player has a helment, return true.
		// otherwise only return true if the hitgroup is not generic / legs / gear.
		if (hitgroup == HITGROUP_HEAD && player->m_bHasHelmet())
			return true;

		else if (hitgroup >= HITGROUP_CHEST && hitgroup <= HITGROUP_RIGHTARM)
			return true;

		return false;
	};

	// check if the player has heavy armor, this is only really used in operation stuff.
	has_heavy_armor = player->m_bHasHeavyArmor();

	// scale damage based on hitgroup.
	switch (hitgroup) {
	case HITGROUP_HEAD:
		if (has_heavy_armor)
			damage = (damage * 4.f) * 0.5f;
		else
			damage *= 4.f;
		break;

	case HITGROUP_STOMACH:
		damage *= 1.25f;
		break;

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		damage *= 0.75f;
		break;

	default:
		break;
	}

	// grab amount of player armor.
	armor = player->m_ArmourValue();

	// check if the ent is armored and scale damage based on armor.
	if (is_armored(player, armor, hitgroup)) {
		heavy_ratio = 1.f;
		bonus_ratio = 0.5f;
		ratio = armor_ratio * 0.5f;

		// player has heavy armor.
		if (has_heavy_armor) {
			// calculate ratio values.
			bonus_ratio = 0.33f;
			ratio = armor_ratio * 0.25f;
			heavy_ratio = 0.33f;

			// calculate new damage.
			new_damage = (damage * ratio) * 0.85f;
		}

		// no heavy armor, do normal damage calculation.
		else
			new_damage = damage * ratio;

		if (((damage - new_damage) * (heavy_ratio * bonus_ratio)) > armor)
			new_damage = damage - (armor / bonus_ratio);

		damage = new_damage;
	}

	return std::floor(damage);
}
void ClipTraceToPlayers(Vector vecStart, Vector vecEnd, uint32_t mask, ITraceFilter* filter, CGameTrace* Trace, float range)
{
	Ray_t Ray;
	Ray.Init(vecStart, vecEnd);

	CGameTrace NewTrace;
	std::array < uintptr_t, 5 > aFilter
		=
	{
		*(std::uintptr_t*)(g_Globals.m_AddressList.m_TraceFilterSkipTwoEntities),
		NULL,
		NULL,
		NULL,
		(std::uintptr_t)(Trace->hit_entity),
	};

	CTraceFilter* pTraceFilter = (CTraceFilter*)(aFilter.data());
	for (int iPlayerID = 1; iPlayerID <= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients; iPlayerID++)
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(iPlayerID);
		if (!pPlayer || !pPlayer->IsPlayer() || !pPlayer->IsAlive() || pPlayer->IsDormant() || pPlayer->m_iTeamNum() == g_Globals.m_LocalPlayer->m_iTeamNum())
			continue;

		if (pTraceFilter && !pTraceFilter->ShouldHitEntity(pPlayer, NULL))
			continue;

		float_t flRange = Math::DistanceToRay(pPlayer->WorldSpaceCenter(), vecStart, vecEnd);
		if (flRange < 0.0f || flRange > 60.0f)
			continue;

		g_Globals.m_Interfaces.m_EngineTrace->ClipRayToEntity(Ray, mask, pPlayer, &NewTrace);
		if (NewTrace.fraction > Trace->fraction)
			std::memcpy(Trace, &NewTrace, sizeof(CGameTrace));
	}
}

bool IsBreakableEntity(C_BasePlayer* pPlayer)
{
	int32_t m_TakeDamage = *(int32_t*)((uintptr_t)(pPlayer)+0x280);
	if (m_TakeDamage != 2)
		return false;

	int32_t m_CollisionGroup = *(int32_t*)((uintptr_t)(pPlayer)+0x474);
	if (m_CollisionGroup != 17 && m_CollisionGroup != 6 && m_CollisionGroup > 0)
		return false;

	int32_t m_ClassID = pPlayer->GetClientClass()->m_ClassID;
	if (m_ClassID == ClassId_CBreakableSurface ||
		m_ClassID == ClassId_CDynamicProp ||
		m_ClassID == ClassId_CBaseDoor ||
		m_ClassID == ClassId_CBasePropDoor)
		return true;

	return ((bool(__thiscall*)(LPVOID))(g_Globals.m_AddressList.m_IsBreakableEntity))(pPlayer);
}
bool penetration::TraceToExit(const Vector& start, const Vector& dir, Vector& out, CGameTrace* enter_trace, CGameTrace* exit_trace) {
	static CTraceFilter filter{};

	float  dist{};
	Vector new_end;
	int    contents, first_contents{};

	// max pen distance is 90 units.
	while (dist <= 90.f) {
		// step forward a bit.
		dist += 4.f;

		// set out pos.
		out = start + (dir * dist);

		if (!first_contents)
			first_contents = g_Globals.m_Interfaces.m_EngineTrace->GetPointContents(out, MASK_SHOT, nullptr);

		contents = g_Globals.m_Interfaces.m_EngineTrace->GetPointContents(out, MASK_SHOT, nullptr);

		if ((contents & MASK_SHOT_HULL) && (!(contents & CONTENTS_HITBOX) || (contents == first_contents)))
			continue;

		// move end pos a bit for tracing.
		new_end = out - (dir * 4.f);

		Ray_t ray1;
		ray1.Init(out, new_end);

		// do first trace.
		g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray1, MASK_SHOT, nullptr, exit_trace);

		// note - dex; this is some new stuff added sometime around late 2017 ( 10.31.2017 update? ).
		ClipTraceToPlayers(out, new_end, MASK_SHOT, nullptr, exit_trace, -60.f);

		// we hit an ent's hitbox, do another trace.
		if (exit_trace->startsolid && (exit_trace->surface.flags & SURF_HITBOX)) {
			
			Ray_t ray2;
			ray2.Init(out, start);

			std::array < uintptr_t, 5 > aSkipTwoEntities
				=
			{
				*(std::uintptr_t*)(g_Globals.m_AddressList.m_TraceFilterSkipTwoEntities),
				NULL,
				NULL,
				NULL,
				(std::uintptr_t)(exit_trace->hit_entity),
			};

			g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray2, MASK_SHOT_HULL, (CTraceFilter*)(aSkipTwoEntities.data()), exit_trace);

			if (exit_trace->DidHit() && !exit_trace->startsolid) {
				out = exit_trace->endpos;
				return true;
			}

			continue;
		}

		if (!exit_trace->DidHit() || exit_trace->startsolid) {
			if (((C_BaseEntity*)(enter_trace->hit_entity))->IsBreakableEntity()) {
				*exit_trace = *enter_trace;
				exit_trace->endpos = start + dir;
				return true;
			}

			continue;
		}

		if ((exit_trace->surface.flags & SURF_NODRAW)) {
			// note - dex; ok, when this happens the game seems to not ignore world?
			if (((C_BaseEntity*)(enter_trace->hit_entity))->IsBreakableEntity() && ((C_BaseEntity*)(exit_trace->hit_entity))->IsBreakableEntity()) {
				out = exit_trace->endpos;
				return true;
			}

			if (!(enter_trace->surface.flags & SURF_NODRAW))
				continue;
		}

		if (exit_trace->plane.normal.Dot(dir) <= 1.f) {
			out -= (dir * (exit_trace->fraction * 4.f));
			return true;
		}
	}

	return false;
}

void penetration::ClipTraceToPlayer(const Vector& start, const Vector& end, uint32_t mask, CGameTrace* tr, C_BasePlayer* player, float min) {
	Vector     pos, to, dir, on_ray;
	float      len, range_along, range;
	Ray_t        ray;
	CGameTrace new_trace;

	// reference: https://github.com/alliedmodders/hl2sdk/blob/3957adff10fe20d38a62fa8c018340bf2618742b/game/shared/util_shared.h#L381

	// set some local vars.
	pos = player->m_vecOrigin() + ((player->m_vecMins() + player->m_vecMaxs()) * 0.5f);
	to = pos - start;
	dir = start - end;
	len = dir.Normalize();
	range_along = dir.Dot(to);

	// off start point.
	if (range_along < 0.f)
		range = -(to).Length();

	// off end point.
	else if (range_along > len)
		range = -(pos - end).Length();

	// within ray bounds.
	else {
		on_ray = start + (dir * range_along);
		range = (pos - on_ray).Length();
	}

	if ( /*min <= range &&*/ range <= 60.f) {
		// clip to player.
		Ray_t ray3;
		ray3.Init(start, end);
		g_Globals.m_Interfaces.m_EngineTrace->ClipRayToEntity(ray3, mask, player, &new_trace);

		if (tr->fraction > new_trace.fraction)
			*tr = new_trace;
	}
}
bool IsValidHitgroup(int index) {
	if ((index >= HITGROUP_HEAD && index <= HITGROUP_RIGHTLEG) || index == HITGROUP_GEAR)
		return true;

	return false;
}
bool penetration::run(PenetrationInput_t* in, PenetrationOutput_t* out) {
	static CTraceFilter filter{};

	int			  pen{ 4 }, enter_material, exit_material;
	float		  damage, penetration, penetration_mod, player_damage, remaining, trace_len{}, total_pen_mod, damage_mod, modifier, damage_lost;
	surfacedata_t* enter_surface, * exit_surface;
	bool		  nodraw, grate;
	Vector		  start, dir, end, pen_end;
	CGameTrace	  trace, exit_trace;
	C_BaseCombatWeapon* weapon;
	C_CSWeaponData* weapon_info;

	// if we are tracing from our local player perspective.
	if (in->m_from == g_Globals.m_LocalPlayer) {
		weapon = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get();
		
		weapon_info = weapon->GetWeaponData();
		
		start = in->m_start;
	}

	// not local player.
	else {
		weapon = in->m_from->m_hActiveWeapon().Get();
		if (!weapon)
			return false;

		// get weapon info.
		weapon_info = weapon->GetWeaponData();
		if (!weapon_info)
			return false;

		// set trace start.

		start = in->m_from->GetShootPosition();
	}

	// get some weapon data.
	damage = (float)weapon_info->m_iDamage;
	penetration = weapon_info->m_flPenetration;

	// used later in calculations.
	penetration_mod = max(0.f, (3.f / penetration) * 1.25f);

	// get direction to end point.
	dir = (in->m_pos - start).Normalized();

	// setup trace filter for later.
	filter.pSkip = (in->m_from);
	// filter.SetPassEntity2( nullptr );

	while (damage > 0.f) {
		// calculating remaining len.
		remaining = weapon_info->m_flRange - trace_len;

		// set trace end.
		end = start + (dir * remaining);

		Ray_t ray4;
		ray4.Init(start, end);

		g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray4, MASK_SHOT | CONTENTS_HITBOX, (ITraceFilter*)&filter, &trace);
		
		// we didn't hit anything.
		if (trace.fraction == 1.f)
			return false;

		// check for player hitboxes extending outside their collision bounds.
		// if no target is passed we clip the trace to a specific player, otherwise we clip the trace to any player.
		if (in->m_target)
			ClipTraceToPlayer(start, end + (dir * 40.f), MASK_SHOT | CONTENTS_HITBOX, &trace, in->m_target, -60.f);
		else
			ClipTraceToPlayers(start, end + (dir * 40.f), MASK_SHOT | CONTENTS_HITBOX, (ITraceFilter*)&filter, &trace, -60.f);

		// calculate damage based on the distance the bullet traveled.
		trace_len += trace.fraction * remaining;
		damage *= std::pow(weapon_info->m_flRangeModifier, trace_len / 500.f);

		// if a target was passed.
		if (in->m_target) {

			// validate that we hit the target we aimed for.
			if (trace.hit_entity && trace.hit_entity == in->m_target && IsValidHitgroup(trace.hitgroup)) {
				int group = (weapon->m_iItemDefinitionIndex() == WEAPON_TASER) ? HITGROUP_GENERIC : trace.hitgroup;

				// scale damage based on the hitgroup we hit.
				player_damage = scale(in->m_target, damage, weapon_info->m_flArmorRatio, group);

				// set result data for when we hit a player.
				out->m_pen = pen != 4;
				out->m_hitgroup = group;
				out->m_damage = player_damage;
				out->m_target = in->m_target;

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}
		}

		// no target was passed, check for any player hit or just get final damage done.
		else {
			out->m_pen = pen != 4;
			auto player = reinterpret_cast<C_BasePlayer*>(trace.hit_entity);

			// todo - dex; team checks / other checks / etc.
			if (player && player->EntIndex() && player->EntIndex() > 0 && player->EntIndex() < 65 && IsValidHitgroup(trace.hitgroup)) {
				int group = (weapon->m_iItemDefinitionIndex() == WEAPON_TASER) ? HITGROUP_GENERIC : trace.hitgroup;

				player_damage = scale(player, damage, weapon_info->m_flArmorRatio, group);

				// set result data for when we hit a player.
				out->m_hitgroup = group;
				out->m_damage = player_damage;
				out->m_target = player;

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}

			// if we've reached here then we didn't hit a player yet, set damage and hitgroup.
			out->m_damage = damage;
		}

		// don't run pen code if it's not wanted.
		if (!in->m_can_pen)
			return false;

		// get surface at entry point.
		enter_surface = g_Globals.m_Interfaces.m_PropPhysics->GetSurfaceData(trace.surface.surfaceProps);

		// this happens when we're too far away from a surface and can penetrate walls or the surface's pen modifier is too low.
		if ((trace_len > 3000.f && penetration) || enter_surface->game.flPenetrationModifier < 0.1f)
			return false;

		// store data about surface flags / contents.
		nodraw = (trace.surface.flags & SURF_NODRAW);
		grate = (trace.contents & CONTENTS_GRATE);

		// get material at entry point.
		enter_material = enter_surface->game.material;

		// note - dex; some extra stuff the game does.
		if (!pen && !nodraw && !grate && enter_material != CHAR_TEX_GRATE && enter_material != CHAR_TEX_GLASS)
			return false;

		// no more pen.
		if (penetration <= 0.f || pen <= 0)
			return false;

		// try to penetrate object.
		if (!TraceToExit(trace.endpos, dir, pen_end, &trace, &exit_trace)) {
			if (!(g_Globals.m_Interfaces.m_EngineTrace->GetPointContents(pen_end, MASK_SHOT_HULL) & MASK_SHOT_HULL))
				return false;
		}

		// get surface / material at exit point.
		exit_surface = g_Globals.m_Interfaces.m_PropPhysics->GetSurfaceData(exit_trace.surface.surfaceProps);
		exit_material = exit_surface->game.material;

		// todo - dex; check for CHAR_TEX_FLESH and ff_damage_bullet_penetration / ff_damage_reduction_bullets convars?
		//             also need to check !isbasecombatweapon too.
		if (enter_material == CHAR_TEX_GRATE || enter_material == CHAR_TEX_GLASS) {
			total_pen_mod = 3.f;
			damage_mod = 0.05f;
		}

		else if (nodraw || grate) {
			total_pen_mod = 1.f;
			damage_mod = 0.16f;
		}

		else {
			total_pen_mod = (enter_surface->game.flPenetrationModifier + exit_surface->game.flPenetrationModifier) * 0.5f;
			damage_mod = 0.16f;
		}

		// thin metals, wood and plastic get a penetration bonus.
		if (enter_material == exit_material) {
			if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
				total_pen_mod = 3.f;

			else if (exit_material == CHAR_TEX_PLASTIC)
				total_pen_mod = 2.f;
		}

		// set some local vars.
		trace_len = (exit_trace.endpos - trace.endpos).LengthSqr();
		modifier = fmaxf(1.f / total_pen_mod, 0.f);

		// this calculates how much damage we've lost depending on thickness of the wall, our penetration, damage, and the modifiers set earlier
		damage_lost = fmaxf(
			((modifier * trace_len) / 24.f)
			+ ((damage * damage_mod)
				+ (fmaxf(3.75 / penetration, 0.f) * 3.f * modifier)), 0.f);

		// subtract from damage.
		damage -= max(0.f, damage_lost);
		if (damage < 1.f)
			return false;

		// set new start pos for successive trace.
		start = exit_trace.endpos;

		// decrement pen.
		--pen;
	}

	return false;
}