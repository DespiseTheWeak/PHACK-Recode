#pragma once

namespace SDK
{
	class CUserCmd;
	class CBaseEntity;
}
class CVisuals
{
public:
	struct damage_indicator_t {
		int dmg;
		bool initializes;
		float earse_time;
		float last_update;
		SDK::CBaseEntity * player;
		Vector Position;
		char* hitboxmeme;
	};
	struct ESPBox
	{
		int x, y, w, h;
	};
	std::vector<damage_indicator_t> dmg_indicator;
	void Draw();
	void HeadEsp(SDK::CBaseEntity * entity);
	void SpecList();
	void DrawZeusRange(SDK::CBaseEntity* entity);
	void DrawDamageIndicator();
	void ClientDraw();
	void DrawInaccuracy();
	void LagCompHitbox(SDK::CBaseEntity * entity, int index);
	void DrawBulletBeams();
	void AntiAimLines();
	void set_hitmarker_time(float time);
	void ModulateSky();
	void DrawName(SDK::CBaseEntity * entity, CColor color, int index, Vector pos, Vector top);
	void LogEvents();
	void phack3();
	void fatality();
	void skeet();
	void moneybot();
	void phack2();
	void watermark();
	void DrawHealth(SDK::CBaseEntity * entity, CVisuals::ESPBox size);
	bool GetBox(SDK::CBaseEntity * pEntity, CVisuals::ESPBox & result);
private:
	void misc_visuals();
	void penetration_reticle();
	void DrawBox(CVisuals::ESPBox size, CColor color, SDK::CBaseEntity * pEntity);
	void DrawBoneESP(SDK::CBaseEntity * entity, CColor color);
	void DrawSkeleton(SDK::CBaseEntity * entity, CColor color);
	void DrawWeapon(SDK::CBaseEntity * entity, CColor color, int index, Vector pos, Vector top);
	void DrawDropped(SDK::CBaseEntity * entity);
	float resolve_distance(Vector src, Vector dest);
	void DrawDistance(SDK::CBaseEntity * entity, CColor color, Vector pos, Vector top);
	void DrawInfo(SDK::CBaseEntity * entity, CColor color, CColor alt, Vector pos, Vector top);
	void DrawFovArrows(SDK::CBaseEntity* entity, CColor color);
	void DrawCrosshair();
	void DrawIndicator();
	void DrawHitmarker();
	void DrawBorderLines();
public:
	std::vector<std::pair<int, float>>				Entities;
	std::deque<UTILS::BulletImpact_t>				Impacts;
};

extern CVisuals* visuals;
extern CVisuals::ESPBox Box;