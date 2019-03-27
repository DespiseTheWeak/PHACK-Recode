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
	void Drawmodels();
	void CustomModels(SDK::CBaseEntity* entity);
	void Draw();
	void HeadEsp(SDK::CBaseEntity * entity);
	void SpecList();
	void DrawZeusRange(SDK::CBaseEntity* entity);
	void DrawDamageIndicator();
	void AsusProps();
	void DoFSN();
	void ClientDraw();
	//void apply_clantag();
	void DrawInaccuracy();
	void BombH(SDK::CBaseEntity * bomb);
	void LagCompHitbox(SDK::CBaseEntity * entity, int index);
	void DrawCapsuleOverlay(SDK::CBaseEntity * entity, int index);
	void DrawBulletBeams();
	void ModulateWorld();
	void Skycolorchanger();
	void AntiAimLines();
	void ModulateSky();
	void Worldcolorchanger();
	void DrawName(SDK::CBaseEntity * entity, CColor color, int index, Vector pos, Vector top);
	//void DrawName(SDK::CBaseEntity * entity, CColor color, int index, Vector pos, Vector top, IDirect3DDevice9 * pDevice);
	void set_hitmarker_time( float time );
	void LogEvents();
//	void Clantag();
	void phack3();
	void fatality();
	void skeet();
	void moneybot();
	void phack2();
	void watermark();
	void DrawHealth(SDK::CBaseEntity * entity, CVisuals::ESPBox size);
	ESPBox CVisuals::GetBOXX(SDK::CBaseEntity* pEntity);
	bool GetBox(SDK::CBaseEntity * pEntity, CVisuals::ESPBox & result);
private:
	void misc_visuals();
	void penetration_reticle();
	void DrawBox(CVisuals::ESPBox size, CColor color, SDK::CBaseEntity * pEntity);
	//void DrawBox(SDK::CBaseEntity* entity, CColor color, Vector pos, Vector top);
	//void DrawName(SDK::CBaseEntity* entity, CColor color, int index, Vector pos, Vector top);
	void DrawForwardtrack(SDK::CBaseEntity * entity);
	void DrawBoneESP(SDK::CBaseEntity * entity, CColor color);
	void DrawSkeleton(SDK::CBaseEntity * entity, CColor color);
	//void LagCompHitbox(SDK::CBaseEntity * entity);
	void DrawWeapon(SDK::CBaseEntity * entity, CColor color, int index, Vector pos, Vector top);
	
//	void DrawHealth(SDK::CBaseEntity * entity, CColor color, CColor dormant, Vector pos, Vector top);
//	void BombPlanted(SDK::CBaseEntity * entity);
	//void BacktrackHit(SDK::CBaseEntity * entity, int index);
	void DrawDropped(SDK::CBaseEntity * entity);
	void DrawAmmo(SDK::CBaseEntity * entity, CColor color, CColor dormant, Vector pos, Vector top);
	float resolve_distance(Vector src, Vector dest);
	void DrawDistance(SDK::CBaseEntity * entity, CColor color, Vector pos, Vector top);
	void DrawInfo(SDK::CBaseEntity * entity, CColor color, CColor alt, Vector pos, Vector top);
	void DrawFovArrows(SDK::CBaseEntity* entity, CColor color);
	void DrawInaccuracy1();
	void DrawCrosshair();
	void DrawIndicator();
	void DrawHitmarker();
	void DrawBorderLines();
public:
	std::vector<std::pair<int, float>>				Entities;
	std::deque<UTILS::BulletImpact_t>				Impacts;
};

extern CVisuals* visuals;