#pragma once

namespace SDK
{
	class CUserCmd;
	class CBaseEntity;
}

class CFakewalk
{
public:
	int choked;
	void slow_govno(SDK::CUserCmd * cmd, float get_speed);
	void do_fakewalk(SDK::CUserCmd * cmd);
	void slow_mo(SDK::CUserCmd * cmd);
};

extern CFakewalk* slidebitch;