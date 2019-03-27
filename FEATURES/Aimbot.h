#pragma once

namespace SDK
{
	class CUserCmd;
	class CBaseEntity;
	class CBaseWeapon;
	struct mstudiobbox_t;
}

struct AimbotData_t
{
	AimbotData_t(SDK::CBaseEntity* player, const int& idx)
	{
		this->pPlayer = player;
		this->index = idx;
	}
	SDK::CBaseEntity*	pPlayer;
	int					index;
};

class CAimbot
{
public:
	//void shoot_enemy(SDK::CUserCmd* cmd);
	bool good_backtrack_tick(int tick);
	void run_aimbot(SDK::CUserCmd * cmd);
	void run_aimbot(SDK::CUserCmd * cmd, SDK::CBaseEntity * pEntity);
	//void run_aimbot(SDK::CUserCmd * cmd);
	//float FovToPlayer(Vector ViewOffSet, Vector View, SDK::CBaseEntity * entity);
	void SelectTarget();
	void lby_backtrack(SDK::CUserCmd * pCmd, SDK::CBaseEntity * pLocal, SDK::CBaseEntity * pEntity);
	//void shoot_enemy(SDK::CUserCmd * cmd, SDK::CBaseEntity * pEntity);
	void shoot_enemy(SDK::CUserCmd * cmd);
	float accepted_inaccuracy(SDK::CBaseWeapon * weapon);
	bool can_shoot(SDK::CUserCmd * cmd);
	bool IsWeaponKnife();
	int zeus_hitbox(SDK::CBaseEntity * entity);
	void autozeus(SDK::CUserCmd * cmd);
	void autoknife(SDK::CUserCmd * cmd);
	void fix_recoil(SDK::CUserCmd* cmd);
	void rotate_movement(float yaw, SDK::CUserCmd * cmd);
	std::vector<Vector> GetMultiplePointsForHitbox(SDK::CBaseEntity * local, SDK::CBaseEntity * entity, int iHitbox, VMatrix BoneMatrix[]);
	Vector get_hitbox_pos(SDK::CBaseEntity* entity, int hitbox_id);
	int scan_hitbox(SDK::CBaseEntity * entity);
	int smart_baim(SDK::CBaseEntity * entity);
	SDK::mstudiobbox_t* get_hitbox(SDK::CBaseEntity* entity, int hitbox_index);
	Vector multipoint(SDK::CBaseEntity* entity, int option);
	Vector point(SDK::CBaseEntity* entity, int option);
	bool slow_walkk(SDK::CUserCmd * cmd);
	int knife_hitbox(SDK::CBaseEntity * entity);
	void auto_revolver(SDK::CUserCmd* cmd);
	void autoscale();
	void automindmg();
	void autohitchance();

	std::vector<AimbotData_t>	Entities;
private:
	//std::vector<Vector> GetMultiplePointsForHitbox(SDK::CBaseEntity * local, SDK::CBaseEntity * entity, int iHitbox, VMatrix BoneMatrix[128]);
	//std::vector<Vector> GetMultiplePointsForHitbox(SDK::CBaseEntity * local, SDK::CBaseEntity * entity, int iHitbox, matrix3x4_t BoneMatrix[]);
	Vector smart_baimpoint(SDK::CBaseEntity * entity);
	Vector scan_hitpoint(SDK::CBaseEntity * entity);
	int get_damage(Vector position);
};

extern CAimbot* aimbot;