#include "../includes.h"
#include "../UTILS/interfaces.h"
#include "../SDK/IEngine.h"
#include "../SDK/CUserCmd.h"
#include "../SDK/CBaseEntity.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/CBaseAnimState.h"
#include "../SDK/CGlobalVars.h"
#include "../SDK/CTrace.h"
#include "../SDK/CBaseWeapon.h"
#include "../FEATURES/Aimbot.h"
#include "../FEATURES/AntiAim.h"
#include "../FEATURES/AutoWall.h"
#define clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))
/*
тут блять мем на меме, спасиба sivm0l(у) за подсказку в десинках оаоаоа
*/
static bool left = false;
static bool right = false;
static bool backwards2 = false;
static bool flip = false;
float randnum(float Min, float Max)
{
	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}
bool breaking_lby;
bool next_lby_update(const float yaw_to_break, SDK::CUserCmd* cmd)
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return false;

	static float next_lby_update_time = 0;
	float curtime = INTERFACES::Globals->curtime;
	local_update = next_lby_update_time;

	auto animstate = local_player->GetAnimState();

	if (!animstate)
		return false;

	if (!(local_player->GetFlags() & FL_ONGROUND))
		return false;

	if (animstate->speed_2d > 0.1f && !fake_walk)
		next_lby_update_time = curtime + 0.22f;

	if (next_lby_update_time < curtime)
	{
		next_lby_update_time = curtime + 1.1f;
		return true;
	}

	return false;
}
float fov_player(Vector ViewOffSet, Vector View, SDK::CBaseEntity* entity, int hitbox)
{
	const float MaxDegrees = 180.0f;
	Vector Angles = View, Origin = ViewOffSet;
	Vector Delta(0, 0, 0), Forward(0, 0, 0);
	Vector AimPos = aimbot->get_hitbox_pos(entity, hitbox);

	MATH::AngleVectors(Angles, &Forward);
	MATH::VectorSubtract(AimPos, Origin, Delta);
	MATH::NormalizeNum(Delta, Delta);

	float DotProduct = Forward.Dot(Delta);
	return (acos(DotProduct) * (MaxDegrees / M_PI));
}
void AngleVectorsp(const Vector angles, Vector& forward, Vector& right, Vector& up)
{
	float angle;
	static float sp, sy, cp, cy;

	angle = angles[0] * (M_PI / 180.f);
	sp = sin(angle);
	cp = cos(angle);

	angle = angles[1] * (M_PI / 180.f);
	sy = sin(angle);
	cy = cos(angle);


	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;


	static float sr, cr;

	angle = angles[2] * (M_PI / 180.f);
	sr = sin(angle);
	cr = cos(angle);


	right[0] = -1 * sr * sp * cy + -1 * cr * -sy;
	right[1] = -1 * sr * sp * sy + -1 * cr *cy;
	right[2] = -1 * sr * cp;

	up[0] = cr * sp *cy + -sr * -sy;
	up[1] = cr * sp *sy + -sr * cy;
	up[2] = cr * cp;


}

int closest_to_crosshair()
{
	int index = -1;
	float lowest_fov = INT_MAX;

	SDK::CBaseEntity* local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return -1;

	Vector local_position = local_player->GetVecOrigin() + local_player->GetViewOffset();

	Vector angles;
	INTERFACES::Engine->GetViewAngles(angles);

	for (int i = 1; i <= INTERFACES::Globals->maxclients; i++)
	{
		SDK::CBaseEntity *entity = INTERFACES::ClientEntityList->GetClientEntity(i);

		if (!entity || entity->GetHealth() <= 0 || entity->GetTeam() == local_player->GetTeam() || entity->GetIsDormant() || entity == local_player)
			continue;

		float fov = fov_player(local_position, angles, entity, 0);

		if (fov < lowest_fov)
		{
			lowest_fov = fov;
			index = i;
		}
	}

	return index;
}
float get_max_desync_delta() {
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	uintptr_t animstate = uintptr_t(local_player->GetAnimState());


	float duckammount = *(float *)(animstate + 0xA4);
	float speedfraction = max(0, min(*reinterpret_cast<float*>(animstate + 0xF8), 1));

	float speedfactor = max(0, min(1, *reinterpret_cast<float*> (animstate + 0xFC)));

	float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
	float unk2 = unk1 + 1.1f;
	float unk3;

	if (duckammount > 0) {

		unk2 += ((duckammount * speedfactor) * (0.5f - unk2));

	}
	else
		unk2 += ((duckammount * speedfactor) * (0.5f - 0.58f));

	unk3 = *(float *)(animstate + 0x334) * unk2;

	return unk3;

}
float detectdatwallbitch(float yaw) //actual freestanding part
{
	float Back, Right, Left;

	Vector src3D, dst3D, forward, right, up, src, dst;
	SDK::trace_t tr;
	SDK::Ray_t ray, ray2, ray3, ray4, ray5;
	SDK::CTraceFilter filter;

	Vector viewangles;
	INTERFACES::Engine->GetViewAngles(viewangles);

	viewangles.x = 0;

	AngleVectorsp(viewangles, forward, right, up);
	int index = closest_to_crosshair();
	auto entity = INTERFACES::ClientEntityList->GetClientEntity(index);
	auto local = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	filter.pSkip1 = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	src3D = local->GetEyePosition();
	dst3D = src3D + (forward * 384);

	ray.Init(src3D, dst3D);

	INTERFACES::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	Back = (tr.end - tr.start).Length();

	ray2.Init(src3D + right * 35, dst3D + right * 35);

	INTERFACES::Trace->TraceRay(ray2, MASK_SHOT, &filter, &tr);

	Right = (tr.end - tr.start).Length();

	ray3.Init(src3D - right * 35, dst3D - right * 35);

	INTERFACES::Trace->TraceRay(ray3, MASK_SHOT, &filter, &tr);

	Left = (tr.end - tr.start).Length();

	if (Back < Right && Back < Left && Left == Right && !(entity == nullptr)) {
		return (yaw); 
	}
	if (Back > Right && Back > Left && !(entity == nullptr)) {
		return (yaw - 180); 
	}
	else if (Right > Left && Right > Back && !(entity == nullptr)) {
		return (yaw + 90); 
	}
	else if (Right > Left && Right == Back && !(entity == nullptr)) {
		return (yaw + 135); 
	}
	else if (Left > Right && Left > Back && !(entity == nullptr)) {
		return (yaw - 90); 
	}
	else if (Left > Right && Left == Back && !(entity == nullptr)) {
		return (yaw - 135); 
	}
	else { 
		if (entity == nullptr) 
			return (yaw - 180); //backward 0.o
		else //if the entity isn't null
			return ((UTILS::CalcAngle(local->GetVecOrigin(), entity->GetVecOrigin()).y) + 180); //else do backward at target antiaim
	}

	return 0;
}
void CAntiAim::fake_duck(SDK::CUserCmd* cmd) {
	if (SETTINGS::settings.fakeduck)
	{
		if (GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.fakeducsk))) {
			static bool counter = false;
			static int counte = 0;
			if (counte == 9) {
				counte = 0;
				counter = !counter;
			}
			counte++;
			if (counter) {
				cmd->buttons |= IN_DUCK;
				GLOBAL::should_send_packet = true;
			}
			else {
				cmd->buttons &= ~IN_DUCK;
				GLOBAL::should_send_packet = false;
			}
		}
	}
}
void CAntiAim::lbybreaker(SDK::CUserCmd* cmd)
{
	static bool flip;
	if (SETTINGS::settings.flip_bool)
		flip = true;
	else
		flip = false;

	if (GLOBAL::should_send_packet)
		if (flip)
			if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd)) {
				//INTERFACES::cvar->ConsoleColorPrintf(CColor::RED, "[PHACK] ");
				GLOBAL::Msg("lby update : %i    \n", local_update);
				cmd->viewangles.y += SETTINGS::settings.delta_val + get_max_desync_delta();
			}
			else
				cmd->viewangles.y -= 90 - get_max_desync_delta();
		else
			if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd)) {
				//INTERFACES::cvar->ConsoleColorPrintf(CColor::RED, "[PHACK] ");
				GLOBAL::Msg("lby update : %i    \n", local_update);
				cmd->viewangles.y -= SETTINGS::settings.delta_val - get_max_desync_delta();
			}
			else
				cmd->viewangles.y += 90 + get_max_desync_delta();
}
void CAntiAim::minimal(SDK::CUserCmd* cmd)
{
	//IClientEntity* plocalminimal = hackManager.pLocal();
	SDK::CBaseEntity *plocalminimal = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	static float Timer = 0;
	Timer++;
	if (!plocalminimal->IsAlive())
		Timer = 0;

	if (plocalminimal->GetFlags() & FL_ONGROUND)
	{
		if (Timer > 1 && Timer < 50)
		{
			cmd->viewangles.x = 1080;
		}
		else
			cmd->viewangles.x = 87;
	}
	else
	{
		Timer = 0;
		cmd->viewangles.x = 87;
	}
}
void CAntiAim::backwards(SDK::CUserCmd* cmd)
{
	if (GLOBAL::should_send_packet)
		cmd->viewangles.y += 179.000000; //cya fake angle checks
	else
		cmd->viewangles.y += 180.000000;
}
void CAntiAim::lowerbody_pysen(SDK::CUserCmd* cmd)
{
	static float last_real;
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd)) {
		cmd->viewangles.y = local_player->GetLowerBodyYaw() + 1809;
		cmd->viewangles.z = 1050.f;
	}


	if (SETTINGS::settings.flip_bool)
	{
		if (GLOBAL::should_send_packet) {
			if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
				cmd->viewangles.y = last_real - SETTINGS::settings.delta_val;
			cmd->viewangles.y = local_player->GetLowerBodyYaw() + 1814;
			cmd->viewangles.z = 1050.f;
		}
		else
			cmd->viewangles.y -= 36078;
		last_real = cmd->viewangles.y;

	}
	else
	{
		if (GLOBAL::should_send_packet)
		{
			if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
				cmd->viewangles.y = last_real - SETTINGS::settings.delta_val;
			cmd->viewangles.y = local_player->GetLowerBodyYaw() + 1814;
			cmd->viewangles.z = 1050.f;
		}
		else {
			cmd->viewangles.y += 36078;
			last_real = cmd->viewangles.y;

		}

	}
}
void CAntiAim::ZAnti(SDK::CUserCmd* cmd)
{
	constexpr auto maxRange = 90.0f;

	// where you want your head to go
	constexpr auto angleAdditive = 180.0f;

	// to make it equal on both sides / by 2
	switch (cmd->command_number % 3)
	{
	case 0: cmd->viewangles.y += get_max_desync_delta() - maxRange / 2.f + std::fmodf(INTERFACES::Globals->curtime * 60, maxRange);
	case 1: cmd->viewangles.y -= get_max_desync_delta() - maxRange / 2.f + std::fmodf(INTERFACES::Globals->curtime * 60, maxRange);
	}
}
void CAntiAim::lowerbody(SDK::CUserCmd* cmd)
{
	static float last_real;
	auto nci = INTERFACES::Engine->GetNetChannelInfo();

	if (SETTINGS::settings.flip_bool)
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += randnum(-180, 180);
		else
		{
			if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd)) //else if
			{
				cmd->viewangles.y = last_real + SETTINGS::settings.delta_val;
			}
			else
			{
				cmd->viewangles.y -= 90;
				last_real = cmd->viewangles.y;
			}
		}
	}
	else
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += randnum(-180, 180);
		else
		{
			if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
			{
				cmd->viewangles.y = last_real - SETTINGS::settings.delta_val;
			}
			else
			{
				cmd->viewangles.y += 90;
				last_real = cmd->viewangles.y;
			}
		}
	}
}
float RandomFloat(float min, float max)
{
	static auto ranFloat = reinterpret_cast<float(*)(float, float)>(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	if (ranFloat)
		return ranFloat(min, max);
	else
		return 0.f;
}
void CAntiAim::freestand3(SDK::CUserCmd* cmd)
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	static float last_real;
	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	Vector besthead;

	auto leyepos = local_player->GetVecOrigin() + local_player->GetViewOffset();
	auto headpos = aimbot->get_hitbox_pos(local_player, 0);
	auto origin = local_player->GetAbsOrigin();

	auto checkWallThickness = [&](SDK::CBaseEntity* pPlayer, Vector newhead) -> float
	{
		Vector endpos1, endpos2;
		Vector eyepos = pPlayer->GetVecOrigin() + pPlayer->GetViewOffset();

		SDK::Ray_t ray;
		ray.Init(newhead, eyepos);

		SDK::CTraceFilterSkipTwoEntities filter(pPlayer, local_player);

		SDK::trace_t trace1, trace2;
		INTERFACES::Trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace1);

		if (trace1.DidHit())
			endpos1 = trace1.end;
		else
			return 0.f;

		ray.Init(eyepos, newhead);
		INTERFACES::Trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace2);

		if (trace2.DidHit())
			endpos2 = trace2.end;

		float add = newhead.Dist(eyepos) - leyepos.Dist(eyepos) + 3.f;
		return endpos1.Dist(endpos2) + add / 3;
	};

	int index = closest_to_crosshair();

	static SDK::CBaseEntity* entity;

	if (index != -1)
		entity = INTERFACES::ClientEntityList->GetClientEntity(index);

	float step = (2 * M_PI) / 18.f; // One PI = half a circle ( for stacker cause low iq :sunglasses: ), 28

	float radius = fabs(Vector(headpos - origin).Length2D());

	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
		{
			Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);

			float totalthickness = 0.f;

			no_active = false;

			totalthickness += checkWallThickness(entity, newhead);

			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				bestrotation = rotation;
				besthead = newhead;
			}
		}
	}

	if (!GLOBAL::should_send_packet)
	{
		if (next_lby_update(cmd->viewangles.y, cmd))
		{
			cmd->viewangles.y = last_real + SETTINGS::settings.delta_val;
		}
		else
		{
			if (no_active)
			{
				cmd->viewangles.y += 179.000000;
			}
			else
				cmd->viewangles.y = RAD2DEG(bestrotation);

			last_real = cmd->viewangles.y;
		}
	}
}
void autoDirection(SDK::CUserCmd* cmd)
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	static float last_real;
	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	Vector besthead;

	auto leyepos = local_player->GetVecOrigin() + local_player->GetViewOffset();
	auto headpos = aimbot->get_hitbox_pos(local_player, 0);
	auto origin = local_player->GetAbsOrigin();

	auto checkWallThickness = [&](SDK::CBaseEntity* pPlayer, Vector newhead) -> float
	{
		Vector endpos1, endpos2;
		Vector eyepos = pPlayer->GetVecOrigin() + pPlayer->GetViewOffset();

		SDK::Ray_t ray;
		ray.Init(newhead, eyepos);

		SDK::CTraceFilterSkipTwoEntities filter(pPlayer, local_player);

		SDK::trace_t trace1, trace2;
		INTERFACES::Trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace1);

		if (trace1.DidHit())
			endpos1 = trace1.end;
		else
			return 0.f;

		ray.Init(eyepos, newhead);
		INTERFACES::Trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace2);

		if (trace2.DidHit())
			endpos2 = trace2.end;

		float add = newhead.DistTo(eyepos) - leyepos.DistTo(eyepos) + 3.f;
		return endpos1.DistTo(endpos2) + add / 3;
	};

	int index = closest_to_crosshair();
	auto entity = INTERFACES::ClientEntityList->GetClientEntity(index);

	float step = (2 * M_PI) / 18.f;
	float radius = fabs(Vector(headpos - origin).Length2D());

	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
		{
			Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);

			float totalthickness = 0.f;

			no_active = false;

			totalthickness += checkWallThickness(entity, newhead);

			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				bestrotation = rotation;
				besthead = newhead;
			}
		}
	}
	if (no_active)
		cmd->viewangles.y += 180.f;
	else
		cmd->viewangles.y = RAD2DEG(bestrotation);

	last_real = cmd->viewangles.y;
}
void CAntiAim::do_antiaim(SDK::CUserCmd* cmd)
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return;
	if (local_player->GetHealth() <= 0) return;
	//float_t cmd->viewangles.yaw = cmd->viewangles.yaw;
	auto weapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));
	if (!weapon) return; auto grenade = (SDK::CBaseCSGrenade*)weapon;
	if (local_player->GetMoveType() == SDK::MOVETYPE_LADDER) return;
	if (cmd->buttons & IN_USE) return;
	//if (weapon->is_knife())return;
	if (cmd->buttons & IN_ATTACK && aimbot->can_shoot(cmd)) return;
	if (weapon->get_full_info()->m_WeaponType == 9) return;
	if (!SETTINGS::settings.aa_bool) return;
	if (GLOBAL::DisableAA) return;
	if (weapon->is_grenade()) return;

	auto animstate = local_player->GetAnimState();

	if (!animstate)
		return;

	if (animstate->m_bInHitGroundAnimation)
	{
		if (animstate->m_flHeadHeightOrOffsetFromHittingGroundAnimation)
		{
			cmd->viewangles.x = 0.0f;
		}
	}

	static float last_real; int local_flags = local_player->GetFlags();
	if ((local_player->GetVelocity().Length2D() < 80) && !(cmd->buttons & IN_JUMP))
	{

		switch (SETTINGS::settings.aa_pitch_type)
		{
		case 0: break;
		case 1: cmd->viewangles.x = 89.f; break;
		case 2: cmd->viewangles.x = -179.f; break;
		case 3: cmd->viewangles.x = 179.f; break;
		case 4: cmd->viewangles.x = 1080.f; break;
		case 5: cmd->viewangles.x = randnum(-180, 1080); break;
		case 6: cmd->viewangles.x = randnum(-180, 1080) + get_max_desync_delta(); break;
		case 7: minimal(cmd); break;
			//case 8:g_LocalPlayer->m_hActiveWeapon().Get()->IsSniper() ? (g_LocalPlayer->m_hActiveWeapon().Get()->m_zoomLevel() != 0 ? 87.f : 85.f) : 88.99f;
		}
		if (GLOBAL::should_send_packet)
		{
			switch (SETTINGS::settings.aa_fake_type)
			{
			case 0: break;
			case 1:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 2:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += randnum(-180, 180) + get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= randnum(-180, 180) + get_max_desync_delta(); break;
				}
			}break;
			case 3:
			{
				int susi = rand() % 2;
				switch (susi)
				{
				case 0:cmd->viewangles.y += get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 4:
			{
				int susi = rand() % 5;
				switch (susi)
				{
				case 0: cmd->viewangles.y -= get_max_desync_delta(); break;
				case 1:cmd->viewangles.y += get_max_desync_delta(); break;
				case 2: cmd->viewangles.y -= get_max_desync_delta(); break;
				case 3: cmd->viewangles.y -= get_max_desync_delta(); break;
				case 4:cmd->viewangles.y += get_max_desync_delta(); break;
				case 5: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 5: {
				static int y2 = -SETTINGS::settings.spinanglefake;
				int speed = SETTINGS::settings.spinspeedfake;
				y2 += speed; if (y2 >= SETTINGS::settings.spinanglefake) y2 = -SETTINGS::settings.spinanglefake;
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += y2 + get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= y2 + get_max_desync_delta(); break;
				}
				//cmd->viewangles.y = y2 + SETTINGS::settings.aa_fakeadditive_val;
			} break;
			case 6: lbybreaker(cmd);break;
			case 7:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += 90 + get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= 90 - get_max_desync_delta(); break;
				}
			}break;
			case 8:ZAnti(cmd);break;
			case 9:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += detectdatwallbitch(90 + get_max_desync_delta()); break;
				case 1: cmd->viewangles.y -= detectdatwallbitch(90 - get_max_desync_delta()); break;
				}
			}break;
			case 10:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += detectdatwallbitch(get_max_desync_delta()); break;
				case 1: cmd->viewangles.y -= detectdatwallbitch(get_max_desync_delta()); break;
				}
			}break;
				/*case 1: cmd->viewangles.y += 180 + ((rand() % 15) - (15 * 0.5f)) + SETTINGS::settings.aa_fakeadditive_val; break;
				case 2: cmd->viewangles.y += randnum(-180, 180) + SETTINGS::settings.aa_fakeadditive_val; break;
				case 3: cmd->viewangles.y += 0 + SETTINGS::settings.aa_fakeadditive_val; break;
				case 4: cmd->viewangles.y = GLOBAL::real_angles.y + 180.f + SETTINGS::settings.aa_fakeadditive_val; break;
				case 5: {
				static int y2 = -SETTINGS::settings.spinanglefake;
				int speed = SETTINGS::settings.spinspeedfake;
				y2 += speed; if (y2 >= SETTINGS::settings.spinanglefake) y2 = -SETTINGS::settings.spinanglefake;
				cmd->viewangles.y = y2 + SETTINGS::settings.aa_fakeadditive_val;
				} break;
				}*/
			}
		}
		else
		{
			switch (SETTINGS::settings.aa_real_type)
			{
			case 0: break;
			case 1: backwards(cmd);break;
			case 2: SETTINGS::settings.flip_bool ? cmd->viewangles.y -= 90 + SETTINGS::settings.aa_realadditive_val : cmd->viewangles.y += 90 + SETTINGS::settings.aa_realadditive_val; break;
			case 3: cmd->viewangles.y += 180 + ((rand() % 15) - (15 * 0.5f)) + SETTINGS::settings.aa_realadditive_val; break;
			case 4:lowerbody(cmd);break;
			case 5: {
				if (SETTINGS::settings.flip_bool)
				{
					if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd))
					{
						cmd->viewangles.y = last_real + SETTINGS::settings.delta_val;
					}
					else
					{
						cmd->viewangles.y -= 90 + SETTINGS::settings.aa_realadditive_val;
						last_real = cmd->viewangles.y;
					}
				}
				else
				{
					if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
					{
						cmd->viewangles.y = last_real - SETTINGS::settings.delta_val;
					}
					else
					{
						cmd->viewangles.y += 90 + SETTINGS::settings.aa_realadditive_val;
						last_real = cmd->viewangles.y;
					}
				}
			}break;
			case 6: {
				static int y2 = -SETTINGS::settings.spinangle;
				int speed = SETTINGS::settings.spinspeed;
				y2 += speed; if (y2 >= SETTINGS::settings.spinangle) y2 = -SETTINGS::settings.spinangle;
				SETTINGS::settings.flip_bool ? cmd->viewangles.y -= y2 + 90 + SETTINGS::settings.aa_realadditive_val : cmd->viewangles.y += y2 + 90 + SETTINGS::settings.aa_realadditive_val;
			} break;
			case 7: autoDirection(cmd);break;
			case 8:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 9:lowerbody_pysen(cmd);break;
			case 10:freestand3(cmd);break;
			case 11:
			{
				if (GetAsyncKeyState(VK_LEFT)) { left = true; right = false;  backwards2 = false; }
				else if (GetAsyncKeyState(VK_RIGHT)) { left = false; right = true; backwards2 = false; }
				else if (GetAsyncKeyState(VK_DOWN)) { left = false; right = false; backwards2 = true; }

				if (left)
					cmd->viewangles.y + (90.f + randnum(-25.5f, 25.5f));

				else if (right)
					cmd->viewangles.y - (90.f + randnum(-25.5f, 25.5f));

				else if (backwards2)
					cmd->viewangles.y + 180.0f + randnum(-25.5f, 25.5f);
				//break;
			}break;
			}
		}
	}
	else if ((local_player->GetVelocity().Length2D() > 80) && (!(cmd->buttons & IN_JUMP) && (local_flags & FL_ONGROUND)))
	{
		switch (SETTINGS::settings.aa_pitch1_type)
		{
		case 0: break;
		case 1: cmd->viewangles.x = 89.f; break;
		case 2: cmd->viewangles.x = -179.f; break;
		case 3: cmd->viewangles.x = 179.f; break;
		case 4: cmd->viewangles.x = 1080.f; break;
		case 5: cmd->viewangles.x = randnum(-180, 1080); break;
		case 6: cmd->viewangles.x = randnum(-180, 1080) + get_max_desync_delta(); break;
		case 7: minimal(cmd); break;
		}
		if (GLOBAL::should_send_packet)
		{
			switch (SETTINGS::settings.aa_fake1_type)
			{
			case 0: break;
			case 1:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 2:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += randnum(-180, 180) + get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= randnum(-180, 180) + get_max_desync_delta(); break;
				}
			}break;
			case 3:
			{
				int susi = rand() % 2;
				switch (susi)
				{
				case 0:cmd->viewangles.y += get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 4:
			{
				int susi = rand() % 5;
				switch (susi)
				{
				case 0: cmd->viewangles.y -= get_max_desync_delta(); break;
				case 1:cmd->viewangles.y += get_max_desync_delta(); break;
				case 2: cmd->viewangles.y -= get_max_desync_delta(); break;
				case 3: cmd->viewangles.y -= get_max_desync_delta(); break;
				case 4:cmd->viewangles.y += get_max_desync_delta(); break;
				case 5: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 5: {
				static int y2 = -SETTINGS::settings.spinanglefake;
				int speed = SETTINGS::settings.spinspeedfake;
				y2 += speed; if (y2 >= SETTINGS::settings.spinanglefake) y2 = -SETTINGS::settings.spinanglefake;
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += y2 + get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= y2 + get_max_desync_delta(); break;
				}
				//cmd->viewangles.y = y2 + SETTINGS::settings.aa_fakeadditive_val;
			} break;
			case 6: lbybreaker(cmd);break;
			case 7:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += 90 + get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= 90 - get_max_desync_delta(); break;
				}
			}break;
			case 8:ZAnti(cmd);break;
			case 9:switch (cmd->command_number % 2)
			{
			case 0:cmd->viewangles.y + 80.f + get_max_desync_delta(); break;
			case 1:	cmd->viewangles.y - 80.f - get_max_desync_delta(); break;
			}break;
			}
		}
		else
		{
			switch (SETTINGS::settings.aa_real1_type)
			{
			case 0: break;
			case 1: backwards(cmd);break;
			case 2: flip ? cmd->viewangles.y -= 90 + SETTINGS::settings.aa_realadditive1_val : cmd->viewangles.y += 90 + SETTINGS::settings.aa_realadditive1_val; break;
			case 3: cmd->viewangles.y += 180 + ((rand() % 15) - (15 * 0.5f)) + SETTINGS::settings.aa_realadditive1_val; break;
			case 4: lowerbody(cmd);break;
			case 5: {
				if (SETTINGS::settings.flip_bool)
				{
					if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd))
					{
						cmd->viewangles.y = last_real + SETTINGS::settings.delta_val;
					}
					else
					{
						cmd->viewangles.y -= 90 + SETTINGS::settings.aa_realadditive_val;
						last_real = cmd->viewangles.y;
					}
				}
				else
				{
					if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
					{
						cmd->viewangles.y = last_real - SETTINGS::settings.delta_val;
					}
					else
					{
						cmd->viewangles.y += 90 + SETTINGS::settings.aa_realadditive_val;
						last_real = cmd->viewangles.y;
					}
				}
			}break;
			case 6: {
				static int y2 = -SETTINGS::settings.spinangle1;
				int speed = SETTINGS::settings.spinspeed1;
				y2 += speed; if (y2 >= SETTINGS::settings.spinangle1) y2 = -SETTINGS::settings.spinangle1;
				SETTINGS::settings.flip_bool ? cmd->viewangles.y -= y2 + 90 + SETTINGS::settings.aa_realadditive1_val : cmd->viewangles.y += y2 + 90 + SETTINGS::settings.aa_realadditive1_val;
			} break;
			case 7: autoDirection(cmd);break;
			case 8:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 9:lowerbody_pysen(cmd);break;
			case 10:freestand3(cmd);break;
			case 11:
			{
				if (GetAsyncKeyState(VK_LEFT)) { left = true; right = false;  backwards2 = false; }
				else if (GetAsyncKeyState(VK_RIGHT)) { left = false; right = true; backwards2 = false; }
				else if (GetAsyncKeyState(VK_DOWN)) { left = false; right = false; backwards2 = true; }

				if (left)
					cmd->viewangles.y + (90.f + randnum(-25.5f, 25.5f));

				else if (right)
					cmd->viewangles.y - (90.f + randnum(-25.5f, 25.5f));

				else if (backwards2)
					cmd->viewangles.y + 180.0f + randnum(-25.5f, 25.5f);
				break;
			}break;
			}
		}
	}
	else
	{
		switch (SETTINGS::settings.aa_pitch2_type)
		{
		case 0: break;
		case 1: cmd->viewangles.x = 89.f; break;
		case 2: cmd->viewangles.x = -179.f; break;
		case 3: cmd->viewangles.x = 179.f; break;
		case 4: cmd->viewangles.x = 1080.f; break;
		case 5: cmd->viewangles.x = randnum(-180, 1080); break;
		case 6: cmd->viewangles.x = randnum(-180, 1080) + get_max_desync_delta(); break;
		case 7: minimal(cmd); break;
		}
		if (GLOBAL::should_send_packet)
		{
			switch (SETTINGS::settings.aa_fake2_type)
			{
			case 0: break;
			case 1:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 2:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += randnum(-180, 180) + get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= randnum(-180, 180) + get_max_desync_delta(); break;
				}
			}break;
			case 3:
			{
				int susi = rand() % 2;
				switch (susi)
				{
				case 0:cmd->viewangles.y += get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 4:
			{
				int susi = rand() % 5;
				switch (susi)
				{
				case 0: cmd->viewangles.y -= get_max_desync_delta(); break;
				case 1:cmd->viewangles.y += get_max_desync_delta(); break;
				case 2: cmd->viewangles.y -= get_max_desync_delta(); break;
				case 3: cmd->viewangles.y -= get_max_desync_delta(); break;
				case 4:cmd->viewangles.y += get_max_desync_delta(); break;
				case 5: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 5: {
				static int y2 = -SETTINGS::settings.spinanglefake;
				int speed = SETTINGS::settings.spinspeedfake;
				y2 += speed; if (y2 >= SETTINGS::settings.spinanglefake) y2 = -SETTINGS::settings.spinanglefake;
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += y2 + get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= y2 + get_max_desync_delta(); break;
				}
				//cmd->viewangles.y = y2 + SETTINGS::settings.aa_fakeadditive_val;
			} break;
			case 6: lbybreaker(cmd);break;
			case 7:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += 90 + get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= 90 - get_max_desync_delta(); break;
				}
			}break;
			case 8:ZAnti(cmd);break;
			case 9:switch (cmd->command_number % 2)
			{
			case 0:cmd->viewangles.y + 80.f + get_max_desync_delta(); break;
			case 1:	cmd->viewangles.y - 80.f - get_max_desync_delta(); break;
			}break;
			}
		}
		else
		{
			switch (SETTINGS::settings.aa_real2_type)
			{
			case 0: break;
			case 1: backwards(cmd);break;
			case 2: SETTINGS::settings.flip_bool ? cmd->viewangles.y -= 90 + SETTINGS::settings.aa_realadditive2_val : cmd->viewangles.y += 90 + SETTINGS::settings.aa_realadditive2_val; break;
			case 3: cmd->viewangles.y += 180 + ((rand() % 15) - (15 * 0.5f)) + SETTINGS::settings.aa_realadditive2_val; break;
			case 4: lowerbody(cmd);break;
			case 5: {
				if (SETTINGS::settings.flip_bool)
				{
					if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd))
					{
						cmd->viewangles.y = last_real + SETTINGS::settings.delta_val;
					}
					else
					{
						cmd->viewangles.y -= 90 + SETTINGS::settings.aa_realadditive_val;
						last_real = cmd->viewangles.y;
					}
				}
				else
				{
					if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
					{
						cmd->viewangles.y = last_real - SETTINGS::settings.delta_val;
					}
					else
					{
						cmd->viewangles.y += 90 + SETTINGS::settings.aa_realadditive_val;
						last_real = cmd->viewangles.y;
					}
				}
			}break;
			case 6: {
				static int y2 = -SETTINGS::settings.spinangle2;
				int speed = SETTINGS::settings.spinspeed2;
				y2 += speed; if (y2 >= SETTINGS::settings.spinangle2) y2 = -SETTINGS::settings.spinangle2;
				SETTINGS::settings.flip_bool ? cmd->viewangles.y -= y2 + 90 + SETTINGS::settings.aa_realadditive2_val : cmd->viewangles.y += y2 + 90 + SETTINGS::settings.aa_realadditive2_val;
			} break;
			case 7: autoDirection(cmd);break;
			case 8:
			{
				switch (cmd->command_number % 2)
				{
				case 0:cmd->viewangles.y += get_max_desync_delta(); break;
				case 1: cmd->viewangles.y -= get_max_desync_delta(); break;
				}
			}break;
			case 9:lowerbody_pysen(cmd);break;
			case 10:freestand3(cmd);break;
			case 11:
			{
				if (GetAsyncKeyState(VK_LEFT)) { left = true; right = false;  backwards2 = false; }
				else if (GetAsyncKeyState(VK_RIGHT)) { left = false; right = true; backwards2 = false; }
				else if (GetAsyncKeyState(VK_DOWN)) { left = false; right = false; backwards2 = true; }

				if (left)

					cmd->viewangles.y + (90.f + randnum(-25.5f, 25.5f));

				else if (right)

					cmd->viewangles.y - (90.f + randnum(-25.5f, 25.5f));


				else if (backwards2)

					cmd->viewangles.y + 180.0f + randnum(-25.5f, 25.5f);
				break;
				//break;
			}break;
			}
		}
	}
}
void CAntiAim::fix_movement(SDK::CUserCmd* cmd)
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	if (SETTINGS::settings.aa_pitch > 1 || SETTINGS::settings.aa_pitch > 1 || SETTINGS::settings.aa_pitch > 1)
	{
		cmd->move.x = clamp(cmd->move.x, -450.f, 450.f);
		cmd->move.y = clamp(cmd->move.y, -450.f, 450.f);
		cmd->move.z = clamp(cmd->move.z, -320.f, 320.f);

		cmd->viewangles.x = clamp(cmd->viewangles.x, -89.f, 89.f);
	}

	Vector real_viewangles;
	INTERFACES::Engine->GetViewAngles(real_viewangles);

	Vector vecMove(cmd->move.x, cmd->move.y, cmd->move.z);
	float speed = sqrt(vecMove.x * vecMove.x + vecMove.y * vecMove.y);

	Vector angMove;
	MATH::VectorAngles(vecMove, angMove);

	float yaw = DEG2RAD(cmd->viewangles.y - real_viewangles.y + angMove.y);

	cmd->move.x = cos(yaw) * speed;
	cmd->move.y = sin(yaw) * speed;

	cmd->viewangles = MATH::NormalizeAngle(cmd->viewangles);

	if (cmd->viewangles.x < -89.f || cmd->viewangles.x > 89.f) cmd->move.x *= -1;
}

Vector CAntiAim::fix_movement(SDK::CUserCmd* cmd, SDK::CUserCmd originalCMD)
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (!local_player) return Vector(0, 0, 0);

	Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

	auto viewangles = cmd->viewangles;
	auto movedata = originalCMD.move;
	viewangles.Normalize();

	if (!(local_player->GetFlags() & FL_ONGROUND) && viewangles.z != 0.f)
		movedata.y = 0.f;

	MATH::AngleVectors2(originalCMD.viewangles, &wish_forward, &wish_right, &wish_up);
	MATH::AngleVectors2(viewangles, &cmd_forward, &cmd_right, &cmd_up);

	auto v8 = sqrt(wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y), v10 = sqrt(wish_right.x * wish_right.x + wish_right.y * wish_right.y), v12 = sqrt(wish_up.z * wish_up.z);

	Vector wish_forward_norm(1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f),
		wish_right_norm(1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f),
		wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up.z);

	auto v14 = sqrt(cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y), v16 = sqrt(cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y), v18 = sqrt(cmd_up.z * cmd_up.z);

	Vector cmd_forward_norm(1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f),
		cmd_right_norm(1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f),
		cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up.z);

	auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

	Vector correct_movement;
	correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25
		+ (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28)
		+ (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27);
	correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25
		+ (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28)
		+ (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27);
	correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25
		+ (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28)
		+ (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27);

	correct_movement.x = clamp(correct_movement.x, -450.f, 450.f);
	correct_movement.y = clamp(correct_movement.y, -450.f, 450.f);
	correct_movement.z = clamp(correct_movement.z, -320.f, 320.f);

	return correct_movement;
}

CAntiAim* antiaim = new CAntiAim();