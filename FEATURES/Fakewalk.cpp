#include "../includes.h"
#include "../UTILS/interfaces.h"
#include "../SDK/IEngine.h"
#include "../SDK/CUserCmd.h"
#include "../SDK/CBaseEntity.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/CTrace.h"
#include "../SDK/CBaseWeapon.h"
#include "../SDK/CGlobalVars.h"
#include "../SDK/NetChannel.h"
#include "../SDK/CBaseAnimState.h"
#include "../SDK/ConVar.h"
#include "../FEATURES/AutoWall.h"
#include "../FEATURES/Fakewalk.h"
#include "../FEATURES/Aimbot.h"

#include <time.h>
#include <iostream>
float fastsqrt(float val) {
	int tmp = *(int *)&val;
	tmp -= 1 << 23;
	tmp = tmp >> 1;
	tmp += 1 << 29;
	return *(float *)&tmp;
}
void CFakewalk::slow_govno(SDK::CUserCmd* cmd ,float get_speed)
{
	if (get_speed <= 0.f)
		return;

	float min_speed = (float)(fastsqrt(sqrt(cmd->move.x) + sqrt(cmd->move.y) + sqrt(cmd->move.z)));
	if (min_speed <= 0.f)
		return;

	if (GetAsyncKeyState(VK_SHIFT))
		get_speed *= 2.94117647f;

	if (min_speed <= get_speed)
		return;

	float kys = get_speed / min_speed;

	cmd->move.x *= kys;
	cmd->move.y *= kys;
	cmd->move.z *= kys;
}
void CFakewalk::do_fakewalk(SDK::CUserCmd* cmd)
{
	if (GetAsyncKeyState(VK_SHIFT))
	{
		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		if (!local_player || local_player->GetHealth() <= 0) return;

		auto net_channel = INTERFACES::Engine->GetNetChannel();
		if (!net_channel) return;

		auto animstate = local_player->GetAnimState();
		if (!animstate) return;

		fake_walk = true;

		if (fabs(local_update - INTERFACES::Globals->curtime) <= 0.1)
		{
			cmd->move.x = 450;
			aimbot->rotate_movement(UTILS::CalcAngle(Vector(0, 0, 0), local_player->GetVelocity()).y + 180.f, cmd);
		}

		choked = choked > 7 ? 0 : choked + 1;
		cmd->move.x = choked < 2 || choked > 5 ? 0 : cmd->move.x;
		cmd->move.y = choked < 2 || choked > 5 ? 0 : cmd->move.y;
	}
	else
		fake_walk = false;
}
void CFakewalk::slow_mo(SDK::CUserCmd *cmd)
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	if (SETTINGS::settings.glitch_bool)
	{
		if (GetAsyncKeyState(VK_SHIFT))
		{

			static int iChoked = -1;
			iChoked++;

			static bool slowmo;
			slowmo = !slowmo;


			const auto lag = 90;


			if (iChoked < lag)
			{

				GLOBAL::should_send_packet = false;
				if (slowmo)
				{
					cmd->tick_count = INT_MAX;
					cmd->command_number += INT_MAX + cmd->tick_count % 2 ? 1 : 0;
					cmd->buttons |= IN_FORWARD;
					cmd->buttons |= IN_LEFT;
					cmd->buttons |= IN_BACK;
					cmd->move.x = cmd->move.y = 0.f;
				}
				else
				{
					GLOBAL::should_send_packet = true;
					iChoked = -1;
					INTERFACES::Globals->frametime *= (local_player->GetVelocity().Length2D()) / 1.2;
					cmd->buttons |= IN_FORWARD;
				}

			}
			else
			{
				if (!GLOBAL::should_send_packet)
				{

					if (slowmo)
					{
						cmd->tick_count = INT_MAX;
						cmd->command_number += INT_MAX + cmd->tick_count % 2 ? 1 : 0;
						cmd->buttons |= IN_FORWARD;
						cmd->buttons |= IN_LEFT;
						cmd->buttons |= IN_BACK;
						cmd->move.x = cmd->move.y = 0.f;
					}

				}
				else
				{

					if (slowmo)
					{
						GLOBAL::should_send_packet = true;
						iChoked = -1;
						cmd->tick_count = INT_MAX;
						INTERFACES::Globals->frametime *= (local_player->GetVelocity().Length2D()) / 1.25;
						cmd->buttons |= IN_FORWARD;
					}

				}
			}
		}
	}
}
CFakewalk* slidebitch = new CFakewalk();