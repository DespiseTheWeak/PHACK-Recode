#include "../includes.h"
#include "../UTILS/interfaces.h"
#include "../SDK/IEngine.h"
#include "../SDK/CUserCmd.h"
#include "../SDK/ConVar.h"
#include "../FEATURES/trash.h"
#include "../SDK/CGlobalVars.h"
#include "../SDK/IVDebugOverlay.h"
#include "../SDK/CClientEntityList.h"
#include <string.h>
// Не знаю зачем я вывел их отдельно, наверное потому что могу.
void trash::ModulateWorld2()
{
	static bool nightmode_performed = false, nightmode_lastsetting;

	if (!INTERFACES::Engine->IsConnected() || !INTERFACES::Engine->IsInGame())
	{
		if (nightmode_performed)
			nightmode_performed = false;
		return;
	}

	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	if (!local_player)
	{
		if (nightmode_performed)
			nightmode_performed = false;
		return;
	}

	if (nightmode_lastsetting != SETTINGS::settings.night_bool)
	{
		nightmode_lastsetting = SETTINGS::settings.night_bool;
		nightmode_performed = false;
	}

	if (!nightmode_performed)
	{
		static SDK::ConVar* r_DrawSpecificStaticProp = INTERFACES::cvar->FindVar("r_DrawSpecificStaticProp");
		r_DrawSpecificStaticProp->nFlags &= ~FCVAR_CHEAT;
		r_DrawSpecificStaticProp->SetValue(0);

		static SDK::ConVar* sv_skyname = INTERFACES::cvar->FindVar("sv_skyname");
		sv_skyname->nFlags &= ~FCVAR_CHEAT;

		for (SDK::MaterialHandle_t i = INTERFACES::MaterialSystem->FirstMaterial(); i != INTERFACES::MaterialSystem->InvalidMaterial(); i = INTERFACES::MaterialSystem->NextMaterial(i))
		{
			SDK::IMaterial *pMaterial = INTERFACES::MaterialSystem->GetMaterial(i);

			if (!pMaterial)
				continue;

			if (strstr(pMaterial->GetTextureGroupName(), "World"))
			{
				if (SETTINGS::settings.night_bool)
					pMaterial->ColorModulate(0.08, 0.08, 0.05);
				else
					pMaterial->ColorModulate(1, 1, 1);

				if (SETTINGS::settings.night_bool)
				{
					sv_skyname->SetValue("sky_csgo_night02");
					pMaterial->SetMaterialVarFlag(SDK::MATERIAL_VAR_TRANSLUCENT, false);
					pMaterial->ColorModulate(0.05, 0.05, 0.05);
				}
				else
				{
					sv_skyname->SetValue("vertigoblue_hdr");
					pMaterial->ColorModulate(1.00, 1.00, 1.00);
				}
			}
			else if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
			{
				if (SETTINGS::settings.night_bool)
					pMaterial->ColorModulate(0.28, 0.28, 0.28);
				else
					pMaterial->ColorModulate(1, 1, 1);
			}
		}
		nightmode_performed = true;
	}
}

void trash::Skycolorchanger2()
{
	if (SETTINGS::settings.sky_color)
	{
		static SDK::ConVar* r_DrawSpecificStaticProp = INTERFACES::cvar->FindVar("r_DrawSpecificStaticProp");
		r_DrawSpecificStaticProp->nFlags &= ~FCVAR_CHEAT;
		static SDK::ConVar* sv_skyname = INTERFACES::cvar->FindVar("sv_skyname");
		sv_skyname->nFlags &= ~FCVAR_CHEAT;
		for (SDK::MaterialHandle_t i = INTERFACES::MaterialSystem->FirstMaterial(); i != INTERFACES::MaterialSystem->InvalidMaterial(); i = INTERFACES::MaterialSystem->NextMaterial(i))
		{
			SDK::IMaterial *pMaterial = INTERFACES::MaterialSystem->GetMaterial(i);
			if (!pMaterial)
				continue;
			if (strstr(pMaterial->GetTextureGroupName(), "SkyBox")) {

				if (SETTINGS::settings.sky_color)
				{
					pMaterial->ColorModulate(SETTINGS::settings.sky_ccolor[0], SETTINGS::settings.sky_ccolor[1], SETTINGS::settings.sky_ccolor[2]);
				}
			}
		}
	}
	else
	{
		static SDK::ConVar* r_DrawSpecificStaticProp = INTERFACES::cvar->FindVar("r_DrawSpecificStaticProp");
		r_DrawSpecificStaticProp->nFlags &= ~FCVAR_CHEAT;
		static SDK::ConVar* sv_skyname = INTERFACES::cvar->FindVar("sv_skyname");
		sv_skyname->nFlags &= ~FCVAR_CHEAT;
		for (SDK::MaterialHandle_t i = INTERFACES::MaterialSystem->FirstMaterial(); i != INTERFACES::MaterialSystem->InvalidMaterial(); i = INTERFACES::MaterialSystem->NextMaterial(i))
		{
			SDK::IMaterial *pMaterial = INTERFACES::MaterialSystem->GetMaterial(i);
			if (!pMaterial)
				continue;
			if (strstr(pMaterial->GetTextureGroupName(), "SkyBox")) {

				if (!SETTINGS::settings.sky_color)
				{
					pMaterial->ColorModulate(1, 1, 1);
				}
			}
		}

	}
}
void trash::Worldcolorchanger2()
{
	if (SETTINGS::settings.worldcolorchanger)
	{
		for (SDK::MaterialHandle_t i = INTERFACES::MaterialSystem->FirstMaterial(); i != INTERFACES::MaterialSystem->InvalidMaterial(); i = INTERFACES::MaterialSystem->NextMaterial(i))
		{
			SDK::IMaterial *pMaterial = INTERFACES::MaterialSystem->GetMaterial(i);
			if (!pMaterial)
				continue;
			const char* group = pMaterial->GetTextureGroupName();
			const char* name = pMaterial->GetMaterialName();

			if (strstr(group, "World textures"))
			{
				pMaterial->ColorModulate(SETTINGS::settings.wcolorchanger[0], SETTINGS::settings.wcolorchanger[1], SETTINGS::settings.wcolorchanger[2]);
			}
		}
	}
	else
	{
		for (SDK::MaterialHandle_t i = INTERFACES::MaterialSystem->FirstMaterial(); i != INTERFACES::MaterialSystem->InvalidMaterial(); i = INTERFACES::MaterialSystem->NextMaterial(i))
		{
			SDK::IMaterial *pMaterial = INTERFACES::MaterialSystem->GetMaterial(i);

			if (!pMaterial)
				continue;
			const char* group = pMaterial->GetTextureGroupName();
			const char* name = pMaterial->GetMaterialName();
			if (strstr(group, "World textures"))
			{

				pMaterial->ColorModulate(1, 1, 1);
			}
		}
	}
}
trash* Gettergay = new trash();