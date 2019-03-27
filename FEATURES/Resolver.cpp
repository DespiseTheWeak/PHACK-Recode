#include "../includes.h"
#include "../UTILS/interfaces.h"
#include "../SDK/IEngine.h"
#include "../SDK/CUserCmd.h"
#include "../SDK/CBaseEntity.h"
#include "../SDK/CClientEntityList.h"
#include "../UTILS/render.h"
#include "../SDK/CTrace.h"
#include "../SDK/CBaseWeapon.h"
#include "../SDK/CGlobalVars.h"
#include "../SDK/ConVar.h"
#include "../SDK/AnimLayer.h"
#include "../UTILS/qangle.h"
#include "../FEATURES/Aimbot.h"
#include "../FEATURES/Resolver.h"
#include "../SDK/AnimLayer.h"
#include "../SDK/RecvData.h"
bool UseFreestandAngle[65];
float FreestandAngle[65];
// ты блять даун нахуй сюда залез, пиздуй нахуй тупой пастер мать ебал
void CResolver::record(SDK::CBaseEntity* entity, float new_yaw)
{
	if (entity->GetVelocity().Length2D() > 36) return;

	auto c_baseweapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(entity->GetActiveWeaponIndex()));
	if (!c_baseweapon) return;

	auto &info = player_info[entity->GetIndex()];
	if (entity->GetActiveWeaponIndex() && info.last_ammo < c_baseweapon->GetLoadedAmmo()) {
		info.last_ammo = c_baseweapon->GetLoadedAmmo();
		return;
	}

	info.unresolved_yaw.insert(info.unresolved_yaw.begin(), new_yaw);
	if (info.unresolved_yaw.size() > 20) info.unresolved_yaw.pop_back();
	if (info.unresolved_yaw.size() < 2) return;

	auto average_unresolved_yaw = 0;
	for (auto val : info.unresolved_yaw)
		average_unresolved_yaw += val;
	average_unresolved_yaw /= info.unresolved_yaw.size();

	int delta = average_unresolved_yaw - entity->GetLowerBodyYaw();
	auto big_math_delta = abs((((delta + 180) % 360 + 360) % 360 - 180));

	info.lby_deltas.insert(info.lby_deltas.begin(), big_math_delta);
	if (info.lby_deltas.size() > 10) {
		info.lby_deltas.pop_back();
	}
}
bool find_layer(SDK::CBaseEntity* entity, int act, SDK::CAnimationLayer *set)
{
	for (int i = 0; i < 13; i++)
	{
		SDK::CAnimationLayer layer = entity->GetAnimOverlay(i);
		const int activity = entity->GetSequenceActivity(layer.m_nSequence);
		if (activity == act) {
			*set = layer;
			return true;
		}
	}
	return false;
}
Vector CalcAngle(const Vector& vecSource, const Vector& vecDestination)
{
	Vector qAngles;
	Vector delta = Vector((vecSource[0] - vecDestination[0]), (vecSource[1] - vecDestination[1]), (vecSource[2] - vecDestination[2]));
	float hyp = sqrtf(delta[0] * delta[0] + delta[1] * delta[1]);
	qAngles[0] = (float)(atan(delta[2] / hyp) * (180.0f / M_PI));
	qAngles[1] = (float)(atan(delta[1] / delta[0]) * (180.0f / M_PI));
	qAngles[2] = 0.f;
	if (delta[0] >= 0.f)
		qAngles[1] += 180.f;

	return qAngles;
}
void CResolver::AnimationFix(SDK::CBaseEntity* entity)
{
	//who needs structs or classes not me lol
	static float oldSimtime[65];
	static float storedSimtime[65];
	static float ShotTime[65];
	static float SideTime[65][3];
	static int LastDesyncSide[65];
	static bool Delaying[65];
	static AnimationLayer StoredLayers[64][15];
	static C_AnimState * StoredAnimState[65];
	static float StoredPosParams[65][24];
	static Vector oldEyeAngles[65];
	static float oldGoalfeetYaw[65];
	float* PosParams = (float*)((uintptr_t)entity + 0x2774);
	bool update = false;
	auto shot = GLOBAL::shoot;
	GLOBAL::shoot = false;

	static bool jittering[65];

	auto* AnimState = entity->AnimStat2e();
	if (!AnimState || !entity->AnimOverlays() || !PosParams)
		return;
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;
	auto RemapVal = [](float val, float A, float B, float C, float D) -> float
	{
		if (A == B)
			return val >= B ? D : C;
		return C + (D - C) * (val - A) / (B - A);
	};

	if (storedSimtime[entity->GetIndex()] != entity->GetSimulationTime())
	{
		jittering[entity->GetIndex()] = false;
		entity->ClientAnimations(true);
		entity->UpdateClientSideAnimation();

		memcpy(StoredPosParams[entity->GetIndex()], PosParams, sizeof(float) * 24);
		memcpy(StoredLayers[entity->GetIndex()], entity->AnimOverlays(), (sizeof(AnimationLayer) * entity->NumOverlays()));

		oldGoalfeetYaw[entity->GetIndex()] = AnimState->m_flGoalFeetYaw;

	/*	if (entity->GetActiveWeapon() && !entity->IsKnifeorNade())
		{
			if (ShotTime[entity->GetIndex()] != entity->GetActiveWeapon()->GetLastShotTime())
			{
				shot = true;
				ShotTime[entity->GetIndex()] = pEnt->GetActiveWeapon()->GetLastShotTime();
			}
			else
				shot = false;
		}
		else
		{
			shot = false;
			ShotTime[entity->GetIndex()] = 0.f;
		}*/

		float angToLocal = MATH::NormalizeYaw(CalcAngle(local_player->GetVecOrigin(), entity->GetVecOrigin()).y);

		float Back = MATH::NormalizeYaw(angToLocal);
		float DesyncFix = 0;
		float Resim = MATH::NormalizeYaw((0.24f / (entity->GetSimulationTime() - oldSimtime[entity->GetIndex()]))*(oldEyeAngles[entity->GetIndex()].y - entity->GetEyeAngles().y));

		if (Resim > 58.f)
			Resim = 58.f;
		if (Resim < -58.f)
			Resim = -58.f;

		if (entity->GetVelocity().Length2D() > 0.5f && !shot)
		{
			float Delta = MATH::NormalizeYaw(MATH::NormalizeYaw(CalcAngle(Vector(0, 0, 0), entity->GetVelocity()).y) - MATH::NormalizeYaw(MATH::NormalizeYaw(AnimState->m_flGoalFeetYaw + RemapVal(PosParams[11], 0, 1, -60, 60)) + Resim));

			int CurrentSide = 0;

			if (Delta < 0)
			{
				CurrentSide = 1;
				SideTime[entity->GetIndex()][1] = INTERFACES::Globals->curtime;
			}
			else if (Delta > 0)
			{
				CurrentSide = 2;
				SideTime[entity->GetIndex()][2] = INTERFACES::Globals->curtime;
			}

			if (LastDesyncSide[entity->GetIndex()] == 1)
			{
				Resim += (58.f - Resim);
				DesyncFix += (58.f - Resim);
			}
			if (LastDesyncSide[entity->GetIndex()] == 2)
			{
				Resim += (-58.f - Resim);
				DesyncFix += (-58.f - Resim);
			}

			if (LastDesyncSide[entity->GetIndex()] != CurrentSide)
			{
				Delaying[entity->GetIndex()] = true;

				if (.5f < (INTERFACES::Globals->curtime - SideTime[entity->GetIndex()][LastDesyncSide[entity->GetIndex()]]))
				{
					LastDesyncSide[entity->GetIndex()] = CurrentSide;
					Delaying[entity->GetIndex()] = false;
				}
			}

			if (!Delaying[entity->GetIndex()])
				LastDesyncSide[entity->GetIndex()] = CurrentSide;
		}
		else if (!shot)
		{
			float Brute = UseFreestandAngle[entity->GetIndex()] ? MATH::NormalizeYaw(Back + FreestandAngle[entity->GetIndex()]) : entity->GetLowerBodyYaw();

			float Delta = MATH::NormalizeYaw(MATH::NormalizeYaw(Brute - MATH::NormalizeYaw(MATH::NormalizeYaw(AnimState->m_flGoalFeetYaw + RemapVal(PosParams[11], 0, 1, -60, 60))) + Resim));

			if (Delta > 58.f)
				Delta = 58.f;
			if (Delta < -58.f)
				Delta = -58.f;

			Resim += Delta;
			DesyncFix += Delta;

			if (Resim > 58.f)
				Resim = 58.f;
			if (Resim < -58.f)
				Resim = -58.f;
		}

		float Equalized = MATH::NormalizeYaw(MATH::NormalizeYaw(AnimState->m_flGoalFeetYaw + RemapVal(PosParams[11], 0, 1, -60, 60)) + Resim);

		float JitterDelta = fabs(MATH::NormalizeYaw(oldEyeAngles[entity->GetIndex()].y - entity->GetEyeAngles().y));

		if (JitterDelta >= 70.f && !shot)
			jittering[entity->GetIndex()] = true;

		if (entity != local_player && entity->GetTeam() != local_player->GetTeam() && (entity->GetFlags() & FL_ONGROUND))
		{
			if (jittering[entity->GetIndex()])
				AnimState->m_flGoalFeetYaw = MATH::NormalizeYaw(entity->GetEyeAngles().y + DesyncFix);
			else
				AnimState->m_flGoalFeetYaw = Equalized;

			entity->SetLowerBodyYaw(AnimState->m_flGoalFeetYaw);
		}

		StoredAnimState[entity->GetIndex()] = AnimState;

		oldEyeAngles[entity->GetIndex()] = entity->GetEyeAngles();

		oldSimtime[entity->GetIndex()] = storedSimtime[entity->GetIndex()];

		storedSimtime[entity->GetIndex()] = entity->GetSimulationTime();

		update = true;
	}

	entity->ClientAnimations(false);

	if (entity != local_player && entity->GetTeam() != local_player->GetTeam() && (entity->GetFlags() & FL_ONGROUND))
		entity->SetLowerBodyYaw(AnimState->m_flGoalFeetYaw);

	AnimState = StoredAnimState[entity->GetIndex()];

	memcpy((void*)PosParams, &StoredPosParams[entity->GetIndex()], (sizeof(float) * 24));
	memcpy(entity->AnimOverlays(), StoredLayers[entity->GetIndex()], (sizeof(AnimationLayer) * entity->NumOverlays()));

	if (entity != local_player && entity->GetTeam() != local_player->GetTeam() && (entity->GetFlags() & FL_ONGROUND) && jittering[entity->GetIndex()])
		entity->SetAbsAngles(Vector(0, entity->GetEyeAngles().y, 0));
	else
		entity->SetAbsAngles(Vector(0, oldGoalfeetYaw[entity->GetIndex()], 0));

	*reinterpret_cast<int*>(uintptr_t(entity) + 0xA30) = INTERFACES::Globals->framecount;
	*reinterpret_cast<int*>(uintptr_t(entity) + 0xA28) = 0;
}

void CResolver::resolve(SDK::CBaseEntity* entity)
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!entity) return;
	if (!local_player) return;

	bool is_local_player = entity == local_player;
	bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;
	if (is_local_player) return;
	if (is_teammate) return;
	if (entity->GetHealth() <= 0) return;
	if (local_player->GetHealth() <= 0) return;

	if ((SETTINGS::settings.overridemethod == 1 && GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.overridekey))) || (SETTINGS::settings.overridemethod == 0 && SETTINGS::settings.overridething))
	{
		Vector viewangles; INTERFACES::Engine->GetViewAngles(viewangles);
		auto at_target_yaw = UTILS::CalcAngle(entity->GetVecOrigin(), local_player->GetVecOrigin()).y;

		auto delta = MATH::NormalizeYaw(viewangles.y - at_target_yaw);
		auto rightDelta = Vector(entity->GetEyeAngles().x, at_target_yaw + 90, entity->GetEyeAngles().z);
		auto leftDelta = Vector(entity->GetEyeAngles().x, at_target_yaw - 90, entity->GetEyeAngles().z);

		if (delta > 0)
			entity->SetEyeAngles(rightDelta);
		else
			entity->SetEyeAngles(leftDelta);
		return;
	}

	auto &info = player_info[entity->GetIndex()];
	float fl_lby = entity->GetLowerBodyYaw();

	info.lby = Vector(entity->GetEyeAngles().x, entity->GetLowerBodyYaw(), 0.f);
	info.inverse = Vector(entity->GetEyeAngles().x, entity->GetLowerBodyYaw() + 180.f, 0.f);
	info.last_lby = Vector(entity->GetEyeAngles().x, info.last_moving_lby, 0.f);
	info.inverse_left = Vector(entity->GetEyeAngles().x, entity->GetLowerBodyYaw() + 115.f, 0.f);
	info.inverse_right = Vector(entity->GetEyeAngles().x, entity->GetLowerBodyYaw() - 115.f, 0.f);

	info.back = Vector(entity->GetEyeAngles().x, UTILS::CalcAngle(entity->GetVecOrigin(), local_player->GetVecOrigin()).y + 180.f, 0.f);
	info.right = Vector(entity->GetEyeAngles().x, UTILS::CalcAngle(entity->GetVecOrigin(), local_player->GetVecOrigin()).y + 70.f, 0.f);
	info.left = Vector(entity->GetEyeAngles().x, UTILS::CalcAngle(entity->GetVecOrigin(), local_player->GetVecOrigin()).y - 70.f, 0.f);

	info.backtrack = Vector(entity->GetEyeAngles().x, lby_to_back[entity->GetIndex()], 0.f);

	shots_missed[entity->GetIndex()] = shots_fired[entity->GetIndex()] - shots_hit[entity->GetIndex()];

	/*if (SETTINGS::settings.fakefix_bool) info.is_moving = entity->GetVelocity().Length2D() > 0.1 && entity->GetFlags() & FL_ONGROUND && !info.could_be_slowmo;
	else info.is_moving = entity->GetVelocity().Length2D() > 0.1 && entity->GetFlags() & FL_ONGROUND;*/
	if (SETTINGS::settings.fakefix_bool)
		info.is_moving = entity->GetVelocity().Length2D() > 0.1 && entity->GetFlags() & FL_ONGROUND && !info.could_be_slowmo;
	else
		info.is_standing = entity->GetVelocity().Length2D() > 0.0 && entity->GetFlags() & FL_ONGROUND && !info.could_be_slowmo;
	info.is_jumping = !entity->GetFlags() & FL_ONGROUND;
	info.could_be_slowmo = entity->GetVelocity().Length2D() > 6 && entity->GetVelocity().Length2D() < 36 && !info.is_crouching;
	info.is_crouching = entity->GetFlags() & FL_DUCKING;
	update_time[entity->GetIndex()] = info.next_lby_update_time;

	static float old_simtime[65];
	if (entity->GetSimTime() != old_simtime[entity->GetIndex()])
	{
		using_fake_angles[entity->GetIndex()] = entity->GetSimTime() - old_simtime[entity->GetIndex()] == INTERFACES::Globals->interval_per_tick;
		old_simtime[entity->GetIndex()] = entity->GetSimTime();
	}

	if (!using_fake_angles[entity->GetIndex()])
	{
		AnimationFix(entity);
		if (backtrack_tick[entity->GetIndex()])
		{
			resolve_type[entity->GetIndex()] = 7;
			entity->SetEyeAngles(info.backtrack);
		}
		else if (info.is_moving)
		{
			entity->SetEyeAngles(info.lby);
			info.last_moving_lby = entity->GetLowerBodyYaw();
			info.stored_missed = shots_missed[entity->GetIndex()];
			resolve_type[entity->GetIndex()] = 2;
		}
		if (shots_missed[entity->GetIndex()] > info.stored_missed)
		{
			resolve_type[entity->GetIndex()] = 4;
			switch (shots_missed[entity->GetIndex()] % 1)
			{
			case 0: {entity->SetEyeAngles(info.lby);
				info.last_moving_lby = entity->GetLowerBodyYaw(); } break;
			case 1: {entity->SetEyeAngles(info.lby);
				info.last_moving_lby = entity->GetLowerBodyYaw(); } break;
		/*	case 0: entity->SetEyeAngles(info.inverse); break;
				//case 1: entity->SetEyeAngles(info.left); break;
			case 1: entity->SetEyeAngles(info.back); break;
				//case 3: entity->SetEyeAngles(info.right); break;
			case 2: entity->SetEyeAngles(info.backtrack); break;
			case 3: entity->SetEyeAngles(info.lby); break;
			case 4: entity->SetEyeAngles(info.last_lby); break;*/
			}
		}
		/*	else if (info.stored_lby != entity->GetLowerBodyYaw())
			{
				entity->SetEyeAngles(info.lby);
				info.stored_lby = entity->GetLowerBodyYaw();
				resolve_type[entity->GetIndex()] = 2;
			}
			else if (info.is_moving)
			{
				entity->SetEyeAngles(info.lby);
				info.last_moving_lby = entity->GetLowerBodyYaw();
				info.stored_missed = shots_missed[entity->GetIndex()];
				resolve_type[entity->GetIndex()] = 2;
			}
			else if (GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.desresolver)))
			{
				entity->SetEyeAngles(info.inverse);
				info.last_moving_lby = entity->GetLowerBodyYaw();
				info.stored_missed = shots_missed[entity->GetIndex()];
				resolve_type[entity->GetIndex()] = 2;
			}
			else
			{
				if (shots_missed[entity->GetIndex()] > info.stored_missed)
				{
					resolve_type[entity->GetIndex()] = 4;
					switch (shots_missed[entity->GetIndex()] % 4)
					{
					case 0: entity->SetEyeAngles(info.inverse); break;
						//case 1: entity->SetEyeAngles(info.left); break;
					case 1: entity->SetEyeAngles(info.back); break;
						//case 3: entity->SetEyeAngles(info.right); break;
					case 2: entity->SetEyeAngles(info.backtrack); break;
					case 3: entity->SetEyeAngles(info.lby); break;
					case 4: entity->SetEyeAngles(info.last_lby); break;
					}
				}
				else
				{
					resolve_type[entity->GetIndex()] = 2;
					entity->SetEyeAngles(info.last_lby);
				}
			}
		}
		else
		{
			entity->SetEyeAngles(info.lby);
			resolve_type[entity->GetIndex()] = 1;

		}
		/*SDK::CAnimationLayer layer = entity->GetAnimOverlay(0);
		if (entity->GetSimTime() != info.stored_simtime)
		{
			info.stored_simtime = entity->GetSimTime();
			info.prev_layer = info.backup_layer;
			SDK::CAnimationLayer dummy;
			info.backup_layer = find_layer(entity, 979, &dummy) ? dummy : layer;
		}

		SDK::CAnimationLayer prev = info.prev_layer;
		auto server_time = local_player->GetTickBase() * INTERFACES::Globals->interval_per_tick; //i have a global dedicated to curtime but am using this because lemon is gay

		if (info.is_moving && !info.could_be_slowmo)
		{
			entity->SetEyeAngles(info.lby);
			info.last_moving_lby = entity->GetLowerBodyYaw();
			info.stored_missed = shots_missed[entity->GetIndex()];
			info.last_move_time = server_time;
			info.reset_state = true;
			resolve_type[entity->GetIndex()] = 1;
		}
		else
		{
			if (info.stored_lby != entity->GetLowerBodyYaw())
			{
				entity->SetEyeAngles(info.lby);
				info.stored_lby = entity->GetLowerBodyYaw();
				info.next_lby_update_time = entity->GetSimTime() + 1.1;
				resolve_type[entity->GetIndex()] = 7;
			}
			else if (server_time - info.last_move_time < 0.1 && info.reset_state)
			{
				info.pre_anim_lby = entity->GetLowerBodyYaw();
				info.reset_state = false;
				info.breaking_lby = false;
				std::cout << "reset and lby break is false!" << std::endl;
			}
		}
		auto previous_is_valid = entity->GetSequenceActivity(prev.m_nSequence) == 979;

		if (info.unresolved_yaw.size() < 2 || info.lby_deltas.size() < 2)
			return;

		auto average_unresolved_yaw = 0;
		for (auto val : info.unresolved_yaw)
			average_unresolved_yaw += val;
		average_unresolved_yaw /= info.unresolved_yaw.size();

		auto average_lby_delta = 0;
		for (auto val : info.lby_deltas)
			average_lby_delta += val;
		average_lby_delta /= info.lby_deltas.size();

		int deltaxd = average_unresolved_yaw - entity->GetLowerBodyYaw();
		auto current_lby_delta = abs((((deltaxd + 180) % 360 + 360) % 360 - 180));

		int update_delta = info.pre_anim_lby - entity->GetLowerBodyYaw();
		auto lby_update_delta = abs((((update_delta + 180) % 360 + 360) % 360 - 180));

		if (find_layer(entity, 979, &layer)
			&& previous_is_valid
			&& (layer.m_flCycle != prev.m_flCycle
				|| layer.m_flWeight == 1.f
				|| server_time - info.last_move_time < 1.4
				&& !info.breaking_lby
				&& layer.m_flCycle >= 0.01
				&& lby_update_delta > 75))
		{
			if (server_time - info.last_move_time < 1.4)
			{
				info.breaking_lby = true;
				std::cout << "breaking lby" << std::endl;
			}
			entity->SetEyeAngles(info.inverse);
			resolve_type[entity->GetIndex()] = 6;
		}
		else
		{
			if (info.breaking_lby)
			{
				if (current_lby_delta > 130 && average_lby_delta > 130) {
					entity->SetEyeAngles(info.lby);
					resolve_type[entity->GetIndex()] = 7;
				}
				else {
					if (info.next_lby_update_time < entity->GetSimTime())
					{
						entity->SetEyeAngles(info.lby);
						info.next_lby_update_time = entity->GetSimTime() + 1.1;
						resolve_type[entity->GetIndex()] = 3;
					}
					else if (info.is_moving)
					{
						resolve_type[entity->GetIndex()] = 5;
						switch (shots_missed[entity->GetIndex()] % 2)
						{
						case 0: entity->SetEyeAngles(info.last_lby); break;
						case 1: entity->SetEyeAngles(info.inverse); break;
						}
					}
					else
					{
						if (shots_missed[entity->GetIndex()] > info.stored_missed)
						{
							resolve_type[entity->GetIndex()] = 4;
							switch (shots_missed[entity->GetIndex()] % 3)
							{
							case 0: entity->SetEyeAngles(info.back); break;
							case 1: entity->SetEyeAngles(info.inverse_left); break;
							case 2: entity->SetEyeAngles(info.last_lby); break;
							case 3: entity->SetEyeAngles(info.inverse_right); break;
							case 4: entity->SetEyeAngles(info.backtrack); break;
							case 5: entity->SetEyeAngles(info.lby); break;
							case 6: entity->SetEyeAngles(info.back); break;

							}
						}
						else
						{
							resolve_type[entity->GetIndex()] = 2;
							entity->SetEyeAngles(info.last_lby);
						}
					}
				}
			}
			else
			{
				entity->SetEyeAngles(info.lby);
				resolve_type[entity->GetIndex()] = 7;
			}
		}
		if (info.stored_lby != fl_lby)
		{
			entity->SetEyeAngles(info.lby);
			info.stored_lby = fl_lby;
			info.next_lby_update_time = entity->GetSimTime() + 1.1;
			resolve_type[entity->GetIndex()] = 1;
		}
		else if (info.next_lby_update_time < entity->GetSimTime())
		{
			entity->SetEyeAngles(info.lby);
			info.next_lby_update_time = entity->GetSimTime() + 1.1;
			resolve_type[entity->GetIndex()] = 3;
		}
		else if (info.is_moving && !info.could_be_slowmo)
		{
			entity->SetEyeAngles(info.lby);
			info.last_moving_lby = fl_lby;
			info.stored_missed = shots_missed[entity->GetIndex()];
			INTERFACES::Globals->curtime;
			resolve_type[entity->GetIndex()] = 1;
		}
		else
		{
			if (info.breaking_lby)
			{
				if (info.is_moving && !info.is_crouching)
				{
					resolve_type[entity->GetIndex()] = 5;
					switch (shots_missed[entity->GetIndex()] % 2)
					{
					case 0: entity->SetEyeAngles(info.last_lby); break;
					case 1: entity->SetEyeAngles(info.inverse); break;
					}
				}
				else {
					if (shots_missed[entity->GetIndex()] > info.stored_missed)
					{
						resolve_type[entity->GetIndex()] = 4;
						switch (shots_missed[entity->GetIndex()] % 4)
						{
						case 0: entity->SetEyeAngles(info.inverse); break;
							//case 1: entity->SetEyeAngles(info.right); break;
						case 1: entity->SetEyeAngles(info.lby); break;
						case 2: entity->SetEyeAngles(info.back); break;
						}
					}
					else
					{
						resolve_type[entity->GetIndex()] = 2;
						entity->SetEyeAngles(info.last_lby);
					}
				}
			}
			else
			{
				entity->SetEyeAngles(info.lby);
				resolve_type[entity->GetIndex()] = 1;

			}
		}*/
	}
}

CResolver* resolver = new CResolver();