#pragma once
namespace SDK
{
	class CUserCmd;
	class CBaseEntity;
}

class CAntiAim
{
public:
	void freestand4(SDK::CUserCmd * cmd);
	void fake_duck(SDK::CUserCmd * cmd);
	void freestand3(SDK::CUserCmd * cmd);
	void do_antiaim(SDK::CUserCmd* cmd);
	void lbybreaker(SDK::CUserCmd * cmd);
	void minimal(SDK::CUserCmd * cmd);
	void fix_movement(SDK::CUserCmd* cmd);
	Vector fix_movement(SDK::CUserCmd* cmd, SDK::CUserCmd orignal);
private:
	void backwards(SDK::CUserCmd* cmd);
	void lowerbody_pysen(SDK::CUserCmd * cmd);
	void ZAnti(SDK::CUserCmd * cmd);
	void legit(SDK::CUserCmd* cmd);
	void sidespin(SDK::CUserCmd * cmd);
	void freestand(SDK::CUserCmd * cmd);
	void sideways(SDK::CUserCmd* cmd);
	void lowerbody(SDK::CUserCmd* cmd);
	void backjitter(SDK::CUserCmd* cmd);
};

extern CAntiAim* antiaim;