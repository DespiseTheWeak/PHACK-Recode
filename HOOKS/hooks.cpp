#include "..\includes.h"
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_internal.h"
#include "../ImGui/dx9/imgui_impl_dx9.h"
#include "hooks.h"
#include "../UTILS/interfaces.h"
#include "../UTILS/offsets.h"
#include "../UTILS/NetvarHookManager.h"
#include "../UTILS/render.h"
#include "../SDK/NetChannel.h"
#include "../FEATURES/NewEventLog.h"
#include "../SDK/CInput.h"
#include "../SDK/IClient.h"
#include "../SDK/CPanel.h"
#include "../SDK/ConVar.h"
#include "../SDK/CGlowObjectManager.h"
#include "../SDK/IEngine.h"
#include "../SDK/CTrace.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/RecvData.h"
#include "../SDK/CBaseAnimState.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/ModelRender.h"
#include "../SDK/RenderView.h"
#include "../SDK/CTrace.h"
#include "../SDK/CViewSetup.h"
#include "../SDK/CGlobalVars.h"
#include "../AWHitmarkers.h"
#include "../SDK/CPrediction.h"
#include "../FEATURES/Movement.h"
#include "../FEATURES/NewVisuals.h"
#include "../FEATURES/Chams.h"
#include "../FEATURES/AntiAim.h"
#include "../FEATURES/Aimbot.h"
#include "../FEATURES/Resolver.h"
#include "../FEATURES/Backtracking.h"
#include "../FEATURES/FakeWalk.h"
#include "../FEATURES/FakeLag.h"
#include "../FEATURES/EnginePred.h"
#include "../FEATURES/Trash.h"
#include "../FEATURES/EventListener.h"
#include "../FEATURES/GrenadePrediction.h"
#include "../FEATURES/Legitbot.h"
#include "../SDK/shitstate.h"
//#include "../FEATURES/Flashlight.h"
#include "../FEATURES/GloveChanger.h"
#include "../FEATURES/SkinChanger.h"
#include "../FEATURES/fakelatency.h"
#include "../shit.h"
#include <intrin.h>
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
namespace INIT
{
	HMODULE Dll;
	HWND Window;
	WNDPROC OldWindow;
}
static bool menu_open = false;
static bool d3d_init = false;
bool PressedKeys[256] = {};
const char* merixids[] =
{
	"1","2","3","4","5","6", "7", "8", "9",
	"Q","W","E","R","T","Y","U","I","O","P",
	"A","S","D","F","G","H","J","K","L",
	"Z","X","C","V","B","N","M",".","\\","|", "/","}","{","[","]",
	"<",">","?","'"
};
static char ConfigNamexd[64] = { 0 };
namespace ImGui
{

	static auto vector_getterxd = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	IMGUI_API bool ComboBoxArrayxd(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getterxd,
			static_cast<void*>(&values), values.size());
	}
}
template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(UTILS::FindSignature("client_panorama.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(UTILS::FindSignature("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}
typedef void(__cdecl* MsgFn)(const char* msg, va_list);
void ConMsg(const char* msg, ...) {

	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything

	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg"); //This gets the address of export "Msg" in the dll "tier0.dll". The static keyword means it's only called once and then isn't called again (but the variable is still there)
	char buffer[989];
	va_list list; //Normal varargs stuff http://stackoverflow.com/questions/10482960/varargs-to-printf-all-arguments
	va_start(list, msg);
	vsprintf(buffer, msg, list);
	va_end(list);
	fn(buffer, list); //Calls the function, we got the address above.

	if (!INTERFACES::cvar->FindVar("developer")->GetInt())
		INTERFACES::cvar->FindVar("developer")->SetValue(true);

	if (!INTERFACES::cvar->FindVar("con_filter_enable")->GetInt())
		INTERFACES::cvar->FindVar("con_filter_enable")->SetValue(2);
}
inline float FastSqrt1(float x)
{
	unsigned int i = *(unsigned int*)&x;
	i += 127 << 23;
	i >>= 1;
	return *(float*)&i;
}
#define square1( x ) ( x * x )
void SlowWalk(SDK::CUserCmd* get_cmd, float get_speed)
{
	if (get_speed <= 0.f)
		return;

	float min_speed = (float)(FastSqrt1(square1(get_cmd->move.x) + square1(get_cmd->move.y) + square1(get_cmd->move.z)));
	if (min_speed <= 0.f)
		return;

	if (get_cmd->buttons & IN_DUCK)
		get_speed *= 2.94117647f;

	if (min_speed <= get_speed)
		return;

	float speed = get_speed / min_speed;

	get_cmd->move.x *= speed;
	get_cmd->move.y *= speed;
	get_cmd->move.z *= speed;
}

void AnimFix(SDK::CBaseEntity* entity)
{

	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	bool is_local_player = entity == local_player;
	bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

	if (is_local_player)
		return;



	struct clientanimating_t
	{
		SDK::CBaseAnimating *pAnimating;
		unsigned int	flags;
		clientanimating_t(SDK::CBaseAnimating *_pAnim, unsigned int _flags) : pAnimating(_pAnim), flags(_flags) {}
	};


	clientanimating_t *animating = nullptr;
	int animflags;

	const unsigned int FCLIENTANIM_SEQUENCE_CYCLE = 0x00000001;


	SDK::CAnimationLayer AnimLayer[15];

	int cnt = 15;
	for (int i = 0; i < cnt; i++)
	{
		AnimLayer[i] = entity->GetAnimOverlay(i);
	}

	float flPoseParameter[24];
	float* pose = (float*)((uintptr_t)entity + 0x2764);
	memcpy(&flPoseParameter, pose, sizeof(float) * 24);

	Vector TargetEyeAngles = *entity->GetEyeAnglesPointer();

	bool bForceAnimationUpdate = entity->GetEyeAnglesPointer()->x != TargetEyeAngles.x || entity->GetEyeAnglesPointer()->y != TargetEyeAngles.y;

	if (bForceAnimationUpdate)
	{
		//Update animations and pose parameters
		clientanimating_t *animating = nullptr;
		int animflags;

		//Make sure game is allowed to client side animate. Probably unnecessary
		for (unsigned int i = 0; i < INTERFACES::g_ClientSideAnimationList->count; i++)
		{
			clientanimating_t *tanimating = (clientanimating_t*)INTERFACES::g_ClientSideAnimationList->Retrieve(i, sizeof(clientanimating_t));
			SDK::CBaseEntity *pAnimEntity = (SDK::CBaseEntity*)tanimating->pAnimating;
			if (pAnimEntity == entity)
			{
				animating = tanimating;
				animflags = tanimating->flags;
				tanimating->flags |= FCLIENTANIM_SEQUENCE_CYCLE;
				break;
			}
		}

		//Update animations/poses
		entity->UpdateClientSideAnimation();

		//Restore anim flags
		if (animating)
			animating->flags = animflags;
	}
	for (unsigned int i = 0; i < INTERFACES::g_ClientSideAnimationList->count; i++)
	{
		clientanimating_t *animating = (clientanimating_t*)INTERFACES::g_ClientSideAnimationList->Retrieve(i, sizeof(clientanimating_t));
		SDK::CBaseEntity *Entity = (SDK::CBaseEntity*)animating->pAnimating;
		if (Entity != local_player && !Entity->GetIsDormant() && Entity->GetHealth() > 0)
		{

			int TickReceivedNetUpdate[65];

			TickReceivedNetUpdate[entity->GetIndex()] = INTERFACES::Globals->tickcount;

			bool HadClientAnimSequenceCycle[65];

			int ClientSideAnimationFlags[65];
			bool IsBreakingLagCompensation[65];
			IsBreakingLagCompensation[entity->GetIndex()] = !Entity->GetIsDormant() && entity->GetVecOrigin().LengthSqr() > (64.0f * 64.0f);

			unsigned int flags = animating->flags;
			ClientSideAnimationFlags[entity->GetIndex()] = flags;
			HadClientAnimSequenceCycle[entity->GetIndex()] = (flags & FCLIENTANIM_SEQUENCE_CYCLE);
			if (HadClientAnimSequenceCycle[entity->GetIndex()])
			{
				if (IsBreakingLagCompensation[entity->GetIndex()] && INTERFACES::Globals->tickcount != TickReceivedNetUpdate[entity->GetIndex()])
				{
					Entity->UpdateClientSideAnimation();
					//Store the new animations
					Entity->CopyPoseParameters(flPoseParameter);
					Entity->CopyAnimLayers(AnimLayer);
				}
			}
		}
	}
	if (is_local_player) {

		for (unsigned int i = 0; i < INTERFACES::g_ClientSideAnimationList->count; i++)
		{
			clientanimating_t *animating = (clientanimating_t*)INTERFACES::g_ClientSideAnimationList->Retrieve(i, sizeof(clientanimating_t));
			SDK::CBaseEntity *Entity = (SDK::CBaseEntity*)animating->pAnimating;
			if (Entity != local_player && !Entity->GetIsDormant() && Entity->GetHealth() > 0)
			{
				bool HadClientAnimSequenceCycle[65];

				int ClientSideAnimationFlags[65];

				unsigned int flags = animating->flags;
				ClientSideAnimationFlags[entity->GetIndex()] = flags;
				HadClientAnimSequenceCycle[entity->GetIndex()] = (flags & FCLIENTANIM_SEQUENCE_CYCLE);

				if (HadClientAnimSequenceCycle[entity->GetIndex()])
				{
					animating->flags |= FCLIENTANIM_SEQUENCE_CYCLE;
				}
			}
		}
	}
}
ImFont* bigmenu_font;
ImFont* menu_font;
ImFont* smallmenu_font;
ImFont* menu_fon;
//--- Other Globally Used Variables ---///
static bool tick = false;
static int ground_tick;
Vector vecAimPunch, vecViewPunch;
Vector* pAimPunch = nullptr;
Vector* pViewPunch = nullptr;

//--- Declare Signatures and Patterns Here ---///
static auto CAM_THINK = UTILS::FindSignature("client_panorama.dll", "85 C0 75 30 38 86");
static auto linegoesthrusmoke = UTILS::FindPattern("client_panorama.dll", (PBYTE)"\x55\x8B\xEC\x83\xEC\x08\x8B\x15\x00\x00\x00\x00\x0F\x57\xC0", "xxxxxxxx????xxx");

//--- Tick Counting ---//
void ground_ticks()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	if (local_player->GetFlags() & FL_ONGROUND)
		ground_tick++;
	else
		ground_tick = 0;
}

namespace HOOKS
{
	CreateMoveFn original_create_move;
	PaintTraverseFn original_paint_traverse;
	PaintFn original_paint;
	FrameStageNotifyFn original_frame_stage_notify;
	DrawModelExecuteFn original_draw_model_execute;
	SceneEndFn original_scene_end;
	TraceRayFn original_trace_ray;
	OverrideViewFn original_override_view;
	RenderViewFn original_render_view;
	SvCheatsGetBoolFn original_get_bool;
	GetViewmodelFOVFn original_viewmodel_fov;

	vfunc_hook fireevent;
	vfunc_hook directz;
	SendDatagramFn original_send_datagram = 0;
	VMT::VMTHookManager iclient_hook_manager;
	VMT::VMTHookManager panel_hook_manager;
	VMT::VMTHookManager paint_hook_manager;
	VMT::VMTHookManager model_render_hook_manager;
	VMT::VMTHookManager scene_end_hook_manager;
	VMT::VMTHookManager render_view_hook_manager;
	VMT::VMTHookManager trace_hook_manager;
	VMT::VMTHookManager net_channel_hook_manager;
	VMT::VMTHookManager override_view_hook_manager;
	VMT::VMTHookManager input_table_manager;
	VMT::VMTHookManager get_bool_manager;
	std::string sPanel = ("FocusOverlayPanel");
	template<class T, class U>
	T fine(T in, U low, U high)
	{
		if (in <= low)
			return low;

		if (in >= high)
			return high;

		return in;
	}
	int __fastcall hkSendDatagram(SDK::NetChannel* netchan, void* edx, void* datagram) {

		if (!INTERFACES::Engine->IsInGame() || !INTERFACES::Engine->IsConnected() || datagram || !SETTINGS::settings.fakelatency_enabled)
			return original_send_datagram(netchan, datagram);

		g_FakeLatency->m_netchan = netchan;

		auto instate = netchan->m_nInReliableState;
		auto in_sequencenr = netchan->m_nInSequenceNr;

		auto lag_s = SETTINGS::settings.fakelatency_amount / 1000.f;
		auto lag_delta = lag_s - INTERFACES::Engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);

		g_FakeLatency->AddLatency(netchan, lag_delta);

		int ret = original_send_datagram(netchan, datagram);

		netchan->m_nInReliableState = instate;
		netchan->m_nInSequenceNr = in_sequencenr;

		return ret;
	}
	bool __stdcall HookedCreateMove(float sample_input_frametime, SDK::CUserCmd* cmd)
	{
		if (!cmd || cmd->command_number == 0)
			return false;

		uintptr_t* FPointer; __asm { MOV FPointer, EBP }
		byte* SendPacket = (byte*)(*FPointer - 0x1C);
		if (!SendPacket) return false;

		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		if (!local_player) return false;

		GLOBAL::should_send_packet = *SendPacket;
		GLOBAL::originalCMD = *cmd;
		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			GrenadePrediction::instance().Tick(cmd->buttons);
			//if (SETTINGS::settings.aim_type == 0)
				//slidebitch->do_fakewalk(cmd);

			if (!GetAsyncKeyState(0x56))
				fakelag->do_fakelag(cmd);
			if (SETTINGS::settings.fastduck)
			{
				cmd->buttons |= IN_BULLRUSH;
			}
			if (SETTINGS::settings.fakeduck) antiaim->fake_duck(cmd);
			if (SETTINGS::settings.bhop_bool) movement->bunnyhop(cmd);
			if (SETTINGS::settings.duck_bool) movement->duckinair(cmd);
			if (SETTINGS::settings.quickstops) movement->quick_stop(cmd);
			if (SETTINGS::settings.clangtaspp) visuals->moneybot();
			if (SETTINGS::settings.clangtasp) visuals->phack3();
			if (SETTINGS::settings.jopakamaza) aimbot->slow_walkk(cmd);
			if (SETTINGS::settings.astro)
			{
				if (GLOBAL::originalCMD.move.x > 0)
				{
					cmd->buttons |= IN_BACK;
					cmd->buttons &= ~IN_FORWARD;
				}

				if (GLOBAL::originalCMD.move.x < 0)
				{
					cmd->buttons |= IN_FORWARD;
					cmd->buttons &= ~IN_BACK;
				}

				if (GLOBAL::originalCMD.move.y < 0)
				{
					cmd->buttons |= IN_MOVERIGHT;
					cmd->buttons &= ~IN_MOVELEFT;
				}

				if (GLOBAL::originalCMD.move.y > 0)
				{
					cmd->buttons |= IN_MOVELEFT;
					cmd->buttons &= ~IN_MOVERIGHT;
				}
			}
			if (SETTINGS::settings.fakewalk)
			{
				if (GetAsyncKeyState(VK_SHIFT))
				{
					static int choked = 0;
					choked = choked > 7 ? 0 : choked + 1;
					GLOBAL::originalCMD.move.x = choked < 2 || choked > SETTINGS::settings.fakewalkspeed ? 0 : GLOBAL::originalCMD.move.x;
					GLOBAL::originalCMD.move.y = choked < 2 || choked > SETTINGS::settings.fakewalkspeed ? 0 : GLOBAL::originalCMD.move.y;
					GLOBAL::should_send_packet = choked < 1;
				}
			}

			if (SETTINGS::settings.clangtasf) visuals->fatality();
			if (SETTINGS::settings.clangtass) visuals->skeet();
			if (SETTINGS::settings.jopakamaza && GetAsyncKeyState(VK_SHIFT))
				SlowWalk(cmd, SETTINGS::settings.slow_scar);
			if (SETTINGS::settings.clangtaspp2) visuals->phack2();
			if (SETTINGS::settings.glitch_bool)
				slidebitch->slow_mo(cmd);
			//if (SETTINGS::settings.misc_clantag) visuals->Clantag();
			prediction->run_prediction(cmd);
			{
				if (SETTINGS::settings.strafe_bool) movement->autostrafer(cmd);

				if (SETTINGS::settings.aim_type == 0 && SETTINGS::settings.aim_bool)
				{
					if (SETTINGS::settings.fake_bool3)
					{
						if (local_player->IsAlive())
						{
							for (int i = 1; i < INTERFACES::Globals->maxclients; i++)
							{
								auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);
								if (!entity || entity == local_player || entity->GetClientClass()->m_ClassID != !entity->IsAlive()) continue;



								float simtime_delta = entity->GetSimTime() - *reinterpret_cast<float*>(uintptr_t(entity) + OFFSETS::m_flSimulationTime) + 0x4;

								int choked_ticks = HOOKS::fine(TIME_TO_TICKS(simtime_delta), 1, 15);
								Vector lastOrig;

								if (lastOrig.Length() != entity->GetVecOrigin().Length())
									lastOrig = entity->GetVecOrigin();

								float delta_distance = (entity->GetVecOrigin() - lastOrig).LengthSqr();
								if (delta_distance > 4096.f)
								{
									Vector velocity_per_tick = entity->GetVelocity() * INTERFACES::Globals->interval_per_tick;
									auto new_origin = entity->GetVecOrigin() + (velocity_per_tick * choked_ticks);
									entity->SetAbsOrigin(new_origin);
								}
							}
						}
					}
					aimbot->run_aimbot(cmd);
					aimbot->autozeus(cmd);
					aimbot->auto_revolver(cmd);
					//aimbot->autoknife(cmd);
					backtracking->backtrack_player(cmd);
					backtracking->run_legit(cmd);
				}

				if (SETTINGS::settings.aim_type == 1 && SETTINGS::settings.aim_bool)
				{
					if (SETTINGS::settings.legittrigger_bool)
						legitbot->triggerbot(cmd);
					backtracking->run_legit(cmd);
				}

				if (SETTINGS::settings.aa_bool)
				{
					antiaim->do_antiaim(cmd);
					antiaim->fix_movement(cmd);
					ground_ticks();
				}
			}
			prediction->end_prediction(cmd);

			if (!GLOBAL::should_send_packet)
				GLOBAL::real_angles = cmd->viewangles;
			else
			{
				GLOBAL::FakePosition = local_player->GetAbsOrigin();
				GLOBAL::fake_angles = cmd->viewangles;
			}

		}
		*SendPacket = GLOBAL::should_send_packet;
		cmd->move = antiaim->fix_movement(cmd, GLOBAL::originalCMD);
		if (SETTINGS::settings.aa_pitch < 2 || SETTINGS::settings.aa_pitch1_type < 2 || SETTINGS::settings.aa_pitch2_type < 2)
			UTILS::ClampLemon(cmd->viewangles);
		return false;
	}

	void __stdcall HookedPaintTraverse(int VGUIPanel, bool ForceRepaint, bool AllowForce)
	{
		std::string panel_name = INTERFACES::Panel->GetName(VGUIPanel);
		static SDK::ConVar* rag = INTERFACES::cvar->FindVar("phys_pushscale");
		rag->nFlags &= ~FCVAR_CHEAT;

		if (SETTINGS::settings.ragdol)
		{
			rag->SetValue(-900);
		}
		else
			rag->SetValue(100);
		if (panel_name == "HudZoom" && SETTINGS::settings.scope_bool) return;
		if (panel_name == "FocusOverlayPanel")
		{
			if (FONTS::ShouldReloadFonts())
				FONTS::InitFonts();

			if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
			{
				
				GrenadePrediction::instance().Paint();
				FEATURES::MISC::in_game_logger.Do();
			//	auto matpostprocess = INTERFACES::cvar->FindVar("mat_postprocess_enable");
			//	matpostprocess->fnChangeCallback = 0;
			//	matpostprocess->SetValue(SETTINGS::settings.matpostprocessenable);

				//visuals->ModulateWorld();
				//if (SETTINGS::settings.night_bool2) visuals->ModulateWorld();
			//	visuals->DrawDamageIndicator();
				//visuals->SpecList();
				SDK::ConVar* mat_fullbright = INTERFACES::cvar->FindVar("mat_fullbright");

				if (SETTINGS::settings.full_bright)
					mat_fullbright->SetValue(true);
				else
					mat_fullbright->SetValue(false);
				if (SETTINGS::settings.esp_bool)
				{
					visuals->Draw();
					visuals->ClientDraw();
				

				}
					if (SETTINGS::settings.hitmarker22)
					{
						pHitmarker->Paint();
					}
					if (SETTINGS::settings.dafakfkads)
					{
						if (SETTINGS::settings.thirdspoof) {
							auto adffaa = INTERFACES::cvar->FindVar("thirdperson");
							adffaa->SetValue(1);
						}
					}
			}

			/*MENU::PPGUI_PP_GUI::Begin();
			MENU::Do();
			MENU::PPGUI_PP_GUI::End();

			UTILS::INPUT::input_handler.Update();*/



			//	visuals->LogEvents();
		}

		original_paint_traverse(INTERFACES::Panel, VGUIPanel, ForceRepaint, AllowForce);

		const char* pszPanelName = INTERFACES::Panel->GetName(VGUIPanel);

		if (!strstr(pszPanelName, sPanel.data()))
			return;


		INTERFACES::Panel->SetMouseInputEnabled(VGUIPanel, menu_open);
	}
	void __fastcall HookedFrameStageNotify(void* ecx, void* edx, int stage)
	{
		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		if (!local_player) return;
		Vector vecAimPunch, vecViewPunch;
		Vector* pAimPunch = nullptr; Vector* pViewPunch = nullptr;
		//visuals->ModulateSky();
		//visuals->DoFSN();
		switch (stage)
		{
		case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
			if (SETTINGS::settings.fakelatency_enabled)
				g_FakeLatency->UpdateIncomingSequences(FRAME_NET_UPDATE_POSTDATAUPDATE_START);
			if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
			{
				GloveChanger();
				xdSkinchanger();
				for (int i = 1; i <= 65; i++)
				{
					auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);
					if (!entity) continue;

					bool is_local_player = entity == local_player;
					bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

					if (is_local_player) continue;
					if (is_teammate) continue;
					if (entity->GetHealth() <= 0) continue;
					if (entity->GetIsDormant()) continue;
					for (int i = 1; i < INTERFACES::Globals->maxclients; i++)
					{
						auto pentity = INTERFACES::ClientEntityList->GetClientEntity(i);
						if (pentity == nullptr)
							continue;
						pentity->GetEyeAngles() = MATH::NormalizeAngle(pentity->GetEyeAngles());
					}
					if (SETTINGS::settings.aim_type == 0 && SETTINGS::settings.resolve_bool)
						resolver->resolve(entity);
				}
			} break;
		case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
			break;
		case FRAME_RENDER_START:
			if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
			{
				if (in_tp)
				{
					SDK::CBaseAnimState* animstate = local_player->GetAnimState();

					if (!animstate)
						return;

					if (animstate->m_bInHitGroundAnimation && animstate->m_flHeadHeightOrOffsetFromHittingGroundAnimation)
						*reinterpret_cast<Vector*>(reinterpret_cast<DWORD>(local_player) + 0x31D8) = Vector(-10, GLOBAL::angles.y, 0.f);
					else
						*reinterpret_cast<Vector*>(reinterpret_cast<DWORD>(local_player) + 0x31D8) = Vector(GLOBAL::angles.x, GLOBAL::angles.y, 0.f);

					INTERFACES::pPrediction->SetLocalViewAngles(GLOBAL::real_angles);
					if (GLOBAL::is_fakewalking) {
						local_player->UpdateClientSideAnimation(); //update our client side animation
						local_player->GetAnimState()->m_flUnknownFraction = 0.f; //replace leg shuffling with leg sliding
					}
					//else
						//FixThemAnim(local_player); //do that premium anim fix m8
						//g_csgo::pPrediction->SetLocalViewAngles(global::real_angles);

				}
				for (int i = 1; i <= 65; i++)
				{
					auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);
					if (!entity) continue;
					if (entity == local_player) continue;

					*(int*)((uintptr_t)entity + 0xA30) = INTERFACES::Globals->framecount;
					*(int*)((uintptr_t)entity + 0xA28) = 0;
				}
			} break;

		case FRAME_NET_UPDATE_START:
			if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
			{
				if (SETTINGS::settings.beam_bool)
					visuals->DrawBulletBeams();
			} break;
		case FRAME_NET_UPDATE_END:
			if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
			{
				for (int i = 1; i < 65; i++)
				{
					auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);

					if (!entity)
						continue;

					if (!local_player)
						continue;

					bool is_local_player = entity == local_player;
					bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

					if (is_local_player)
						continue;

					if (is_teammate)
						continue;

					if (entity->GetHealth() <= 0)
						continue;

					if (SETTINGS::settings.aim_type == 0)
						backtracking->DisableInterpolation(entity);

					auto Animationss = entity->GetAnimState();
					if (Animationss->m_iLastClientSideAnimationUpdateFramecount)
						Animationss->m_iLastClientSideAnimationUpdateFramecount--;

					if (Animationss->m_flLastClientSideAnimationUpdateTime)
						Animationss->m_flLastClientSideAnimationUpdateTime -= INTERFACES::Globals->interval_per_tick;


					AnimFix(entity);
				}
			}
			break;
		}
		original_frame_stage_notify(ecx, stage);
	}
	Vector2D RotatePoint(Vector EntityPos, Vector LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, float zoom, bool* viewCheck, bool angleInRadians = false)
	{
		float r_1, r_2;
		float x_1, y_1;
		r_1 = -(EntityPos.y - LocalPlayerPos.y);
		r_2 = EntityPos.x - LocalPlayerPos.x;
		float Yaw = angle - 90.0f;
		float yawToRadian = Yaw * (float)(M_PI / 180.0F);
		x_1 = (float)(r_2 * (float)cos((double)(yawToRadian)) - r_1 * sin((double)(yawToRadian))) / 20;
		y_1 = (float)(r_2 * (float)sin((double)(yawToRadian)) + r_1 * cos((double)(yawToRadian))) / 20;
		*viewCheck = y_1 < 0;
		x_1 *= zoom;
		y_1 *= zoom;
		int sizX = sizeX / 2;
		int sizY = sizeY / 2;
		x_1 += sizX;
		y_1 += sizY;
		if (x_1 < 5)
			x_1 = 5;
		if (x_1 > sizeX - 5)
			x_1 = sizeX - 5;
		if (y_1 < 5)
			y_1 = 5;
		if (y_1 > sizeY - 5)
			y_1 = sizeY - 5;
		x_1 += posX;
		y_1 += posY;
		return Vector2D(x_1, y_1);
	}
	bool EntityIsInvalid(SDK::CBaseEntity* Entity)
	{
		if (!Entity)
			return true;
		if (Entity->GetHealth() <= 0)
			return true;
		if (Entity->GetIsDormant())
			return true;
		return false;
	}
	void DrawRadar()
	{
		if (SETTINGS::settings.Radargui.Enabled)
		{
			ImGuiStyle& style = ImGui::GetStyle();
			ImVec2 oldPadding = style.WindowPadding;
			float oldAlpha = style.Colors[ImGuiCol_WindowBg].w;
			style.WindowPadding = ImVec2(0, 0);
			style.Colors[ImGuiCol_WindowBg].w = 1.f;
			if (ImGui::Begin("Radar", &SETTINGS::settings.Radargui.Enabled, ImVec2(200, 200), 0.4F, ImGuiWindowFlags_NoTitleBar |/*ImGuiWindowFlags_NoResize | */ImGuiWindowFlags_NoCollapse))
			{
				ImVec2 siz = ImGui::GetWindowSize();
				ImVec2 pos = ImGui::GetWindowPos();
				ImDrawList* DrawList = ImGui::GetWindowDrawList();
				DrawList->AddRect(ImVec2(pos.x - 6, pos.y - 6), ImVec2(pos.x + siz.x + 6, pos.y + siz.y + 6), CColor::Black().GetD3DColor(), 0.0F, -1, 1.5f);
				ImDrawList* windowDrawList = ImGui::GetWindowDrawList();
				windowDrawList->AddLine(ImVec2(pos.x + (siz.x / 2), pos.y + 0), ImVec2(pos.x + (siz.x / 2), pos.y + siz.y), CColor::Black().GetD3DColor(), 1.5f);
				windowDrawList->AddLine(ImVec2(pos.x + 0, pos.y + (siz.y / 2)), ImVec2(pos.x + siz.x, pos.y + (siz.y / 2)), CColor::Black().GetD3DColor(), 1.5f);
				if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
				{
					auto LocalPlayer = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
					if (LocalPlayer)
					{
						Vector LocalPos = LocalPlayer->GetEyePosition();
						Vector ang;
						INTERFACES::Engine->GetViewAngles(ang);
						for (int i = 0; i < INTERFACES::Engine->GetMaxClients(); i++) {
							SDK::CBaseEntity* pBaseEntity = INTERFACES::ClientEntityList->GetClientEntity(i);
							SDK::player_info_t pInfo;
							INTERFACES::Engine->GetPlayerInfo(i, &pInfo);
							if (EntityIsInvalid(pBaseEntity))
								continue;
							SDK::CBaseEntity* observerTarget = INTERFACES::ClientEntityList->GetClientEntityFromHandle(LocalPlayer->GetObserverTargetHandle());
							bool bIsEnemy = (LocalPlayer->GetTeam() != pBaseEntity->GetTeam() || pBaseEntity == observerTarget || pBaseEntity == LocalPlayer) ? true : false;
							if (SETTINGS::settings.Radargui.OnlyEnemy && !bIsEnemy)
								continue;
							bool viewCheck = false;
							Vector2D EntityPos = RotatePoint(pBaseEntity->GetVecOrigin(), LocalPos, pos.x, pos.y, siz.x, siz.y, ang.y, SETTINGS::settings.Radargui.Range, &viewCheck);
							//ImU32 clr = (bIsEnemy ? (isVisibled ? Color::LightGreen() : Color::Blue()) : Color::White()).GetU32();
							ImU32 clr = (bIsEnemy ? CColor::Red() : CColor::LightBlue()).GetD3DColor();
							static bool drawname = true;
							if (pBaseEntity == observerTarget || pBaseEntity == LocalPlayer)
							{
								clr = CColor::White().GetD3DColor();
								drawname = false;
							}
							else
								drawname = true;
							int s = 2;
							windowDrawList->AddRect(ImVec2(EntityPos.x - s, EntityPos.y - s),
								ImVec2(EntityPos.x + s, EntityPos.y + s),
								clr);
							//		RECT TextSize = D::GetTextSize(F::ESP, pInfo.name);
							//	if (drawname && Vars.Misc.Radar.Nicks)
							//		windowDrawList->AddText(ImVec2(EntityPos.x - (TextSize.left / 2), EntityPos.y - s), CColor::White().GetU32(), pInfo.name);
						}
					}
				}
			}
			ImGui::End();
			style.WindowPadding = oldPadding;
			style.Colors[ImGuiCol_WindowBg].w = oldAlpha;
		}

	}
	void __fastcall HookedDrawModelExecute(void* ecx, void* edx, SDK::IMatRenderContext* context, const SDK::DrawModelState_t& state, const SDK::ModelRenderInfo_t& render_info, matrix3x4_t* matrix)
	{
		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			auto local_player = static_cast<SDK::CBaseEntity*>(INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer()));
			std::string ModelName = INTERFACES::ModelInfo->GetModelName(render_info.pModel);
			if (in_tp && render_info.entity_index == local_player->GetIndex() && local_player->GetIsScoped())
				INTERFACES::RenderView->SetBlend(SETTINGS::settings.transparency_amnt);

			if (ModelName.find("v_sleeve") != std::string::npos)
			{
				SDK::IMaterial* material = INTERFACES::MaterialSystem->FindMaterial(ModelName.c_str(), TEXTURE_GROUP_MODEL);
				if (!material) return;
				material->SetMaterialVarFlag(SDK::MATERIAL_VAR_NO_DRAW, true);
				INTERFACES::ModelRender->ForcedMaterialOverride(material);
			}
			if (SETTINGS::settings.asuswalls)
			{
				INTERFACES::RenderView->SetBlend(1 - (SETTINGS::settings.asuswallsvalue / 100));
			}
			switch (SETTINGS::settings.flexmode)
			{
			case 0: {} break;
			case 1: {
				auto as = INTERFACES::cvar->FindVar("cl_phys_timescale");
				as->SetValue(20);
		         	}break;
			case 2: {auto as3 = INTERFACES::cvar->FindVar("cl_phys_timescale");
				as3->SetValue(0.1);}break;

			}
			switch (SETTINGS::settings.fps_mod)
			{
			case 0: {
				auto xd80 = INTERFACES::cvar->FindVar("r_showenvcubemap");
				xd80->SetValue(0);} break;
			case 1: {
				auto vs = INTERFACES::cvar->FindVar("cl_interp");
				vs->SetValue(0);
				auto da = INTERFACES::cvar->FindVar("cl_interp_ratio");
				da->SetValue(1);
				auto da2 = INTERFACES::cvar->FindVar("cl_disable_ragdolls");
				da2->SetValue(1);
				auto da3 = INTERFACES::cvar->FindVar("dsp_slow_cpu");
				da3->SetValue(1);
				auto da5 = INTERFACES::cvar->FindVar("mat_disable_bloom");
				da5->SetValue(1);
				auto da6 = INTERFACES::cvar->FindVar("r_drawparticles");
				da6->SetValue(0);
				auto da7 = INTERFACES::cvar->FindVar("func_break_max_pieces");
				da7->SetValue(0);
				auto da8 = INTERFACES::cvar->FindVar("muzzleflash_light");
				da8->SetValue(0);
				auto da9 = INTERFACES::cvar->FindVar("r_eyemove");
				da9->SetValue(0);
				auto da10 = INTERFACES::cvar->FindVar("r_eyegloss");
				da10->SetValue(0);
				auto da11 = INTERFACES::cvar->FindVar("mat_queue_mode");
				da11->SetValue(2);
				auto xd80 = INTERFACES::cvar->FindVar("r_showenvcubemap");
				xd80->SetValue(0);
			}break;
			case 2: {
				auto xd = INTERFACES::cvar->FindVar("r_drawparticles");
				xd->SetValue(0);
				auto xd2 = INTERFACES::cvar->FindVar("cl_interp");
				xd2->SetValue(0);
				auto xd3 = INTERFACES::cvar->FindVar("dsp_slow_cpu");
				xd3->SetValue(1);
				auto xd4 = INTERFACES::cvar->FindVar("mat_disable_bloom");
				xd4->SetValue(1);
				auto xd5 = INTERFACES::cvar->FindVar("r_showenvcubemap");
				xd5->SetValue(1);
			}break;
			}
		}
		original_draw_model_execute(ecx, context, state, render_info, matrix);
		INTERFACES::RenderView->SetBlend(1.f);
	}
	void __fastcall HookedSceneEnd(void* ecx, void* edx)
	{
		original_scene_end(ecx);
		static SDK::IMaterial* ignorez = chams->CreateMaterialBasic(true, true, false);
		static SDK::IMaterial* notignorez = chams->CreateMaterialBasic(false, true, false);
		static SDK::IMaterial* ignorez_metallic = chams->CreateMaterialMetallic(true, true, false);
		static SDK::IMaterial* notignorez_metallic = chams->CreateMaterialMetallic(false, true, false);

		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
			if (!local_player) return;
			CColor color = CColor(SETTINGS::settings.glow_col[0] * 255, SETTINGS::settings.glow_col[1] * 255, SETTINGS::settings.glow_col[2] * 255, SETTINGS::settings.glow_col[3] * 255), colorTeam = CColor(SETTINGS::settings.teamglow_color[0] * 255, SETTINGS::settings.teamglow_color[1] * 255, SETTINGS::settings.teamglow_color[2] * 255, SETTINGS::settings.teamglow_color[3] * 255), colorlocal = CColor(SETTINGS::settings.glowlocal_col[0] * 255, SETTINGS::settings.glowlocal_col[1] * 255, SETTINGS::settings.glowlocal_col[2] * 255, SETTINGS::settings.glowlocal_col[3] * 255);
			for (int i = 1; i < 65; i++)
			{
				if (SETTINGS::settings.fakechams)
				{
					auto pLocal = reinterpret_cast<SDK::CBaseEntity*>(INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer()));
					if (pLocal && pLocal->IsAlive())
					{
						static SDK::IMaterial* mat = chams->CreateMaterialBasic(false, true, false);
						if (mat)
						{
							Vector OrigAng;
							OrigAng = pLocal->GetEyeAngles();
							pLocal->SetAngle2(Vector(0, GLOBAL::fake_angles.y, 0));
							bool LbyColor = false;
							mat->ColorModulate(CColor(SETTINGS::settings.fakechamscol[0] * 255, SETTINGS::settings.fakechamscol[1] * 255, SETTINGS::settings.fakechamscol[2] * 255, SETTINGS::settings.fakechamscol[3] * 255));
							INTERFACES::ModelRender->ForcedMaterialOverride(mat);
							pLocal->DrawModel(0x1, 150);
							INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
							pLocal->SetAngle2(OrigAng);
						}
					}
				}

				auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);

				if (!entity) continue;
				if (!local_player) continue;

				bool is_local_player = entity == local_player;
				bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;
				auto ignorezmaterial = SETTINGS::settings.chamstype == 0 ? ignorez_metallic : ignorez;
				auto notignorezmaterial = SETTINGS::settings.chamstype == 0 ? notignorez_metallic : notignorez;
				if (is_local_player)
				{
					switch (SETTINGS::settings.localchams)
					{
					case 0: continue; break;
					case 1:
						local_player->SetAbsOrigin(GLOBAL::FakePosition);
						local_player->DrawModel(0x1, 255);
						local_player->SetAbsOrigin(local_player->GetAbsOrigin());
						continue; break;
					case 2:
						notignorezmaterial->ColorModulate(CColor(SETTINGS::settings.localchams_col[0] * 255, SETTINGS::settings.localchams_col[1] * 255, SETTINGS::settings.localchams_col[2] * 255, SETTINGS::settings.localchams_col[3] * 255));
						INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
						local_player->DrawModel(0x1, 255);
						INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
						continue;  break;
					case 3:
						notignorezmaterial->ColorModulate(CColor(SETTINGS::settings.localchams_col[0] * 255, SETTINGS::settings.localchams_col[1] * 255, SETTINGS::settings.localchams_col[2] * 255, SETTINGS::settings.localchams_col[3] * 255));
						INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
						local_player->SetAbsOrigin(GLOBAL::FakePosition);
						local_player->DrawModel(0x1, 255);
						local_player->SetAbsOrigin(local_player->GetAbsOrigin());
						INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
						continue; break;
					case 4:
						static float rainbow;
						rainbow += 0.005f;
						if (rainbow > 1.f)
							rainbow = 0.f;
						notignorezmaterial->ColorModulate(CColor::FromHSB(rainbow, 1.f, 1.f));
						INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
						local_player->DrawModel(0x1, 255);
						INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
						continue;  break;
					}
				}

				if (entity->GetHealth() <= 0) continue;
				if (entity->GetIsDormant())	continue;
				if (entity->GetClientClass()->m_ClassID != 38) continue;

				if (is_teammate)
				{
					if (entity && SETTINGS::settings.chamsteam == 2)
					{
						ignorezmaterial->ColorModulate(CColor(SETTINGS::settings.teaminvis_color[0] * 255, SETTINGS::settings.teaminvis_color[1] * 255, SETTINGS::settings.teaminvis_color[2] * 255, SETTINGS::settings.teaminvis_color[3] * 255));
						INTERFACES::ModelRender->ForcedMaterialOverride(ignorezmaterial);
						entity->DrawModel(0x1, 255);

						notignorezmaterial->ColorModulate(CColor(SETTINGS::settings.teamvis_color[0] * 255, SETTINGS::settings.teamvis_color[1] * 255, SETTINGS::settings.teamvis_color[2] * 255, SETTINGS::settings.teamvis_color[3] * 255));
						INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
						entity->DrawModel(0x1, 255);

						INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
					}
					else if (entity && SETTINGS::settings.chamsteam == 1)
					{
						notignorezmaterial->ColorModulate(CColor(SETTINGS::settings.teamvis_color[0] * 255, SETTINGS::settings.teamvis_color[1] * 255, SETTINGS::settings.teamvis_color[2] * 255, SETTINGS::settings.teamvis_color[3] * 255));
						INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
						entity->DrawModel(0x1, 255);

						INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
					} continue;
				}
				else if (is_teammate && SETTINGS::settings.chamsteam)
					continue;
				if (entity && SETTINGS::settings.chams_type == 2)
				{
					ignorezmaterial->ColorModulate(CColor(SETTINGS::settings.imodel_col[0] * 255, SETTINGS::settings.imodel_col[1] * 255, SETTINGS::settings.imodel_col[2] * 255, SETTINGS::settings.imodel_col[3] * 255));
					INTERFACES::ModelRender->ForcedMaterialOverride(ignorezmaterial);
					entity->DrawModel(0x1, 255);

					notignorezmaterial->ColorModulate(CColor(SETTINGS::settings.vmodel_col[0] * 255, SETTINGS::settings.vmodel_col[1] * 255, SETTINGS::settings.vmodel_col[2] * 255, SETTINGS::settings.vmodel_col[3] * 255));
					INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
					entity->DrawModel(0x1, 255);

					INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
				
				}
				else if (entity && SETTINGS::settings.chams_type == 1)
				{
					notignorezmaterial->ColorModulate(CColor(SETTINGS::settings.vmodel_col[0] * 255, SETTINGS::settings.vmodel_col[1] * 255, SETTINGS::settings.vmodel_col[2] * 255, SETTINGS::settings.vmodel_col[3] * 255));
					INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
					entity->DrawModel(0x1, 255);

					INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);

				}
				else if (entity && SETTINGS::settings.backtrack_show)
				{
					SETTINGS::settings.chams_type = 0;
					Vector oldOrigin = entity->GetVecOrigin();
					QAngle oldAngs = entity->GetAbsAnglesQang();
					if (local_player->GetHealth() > 0)
					{
						for (int i = 1; i < 11; i++)
						{
							auto record = headPositions[entity->GetIndex()][i];
							entity->SetAbsOriginal(record.origin);
							entity->SetAbsAnglesVec(record.angs);
							Vector temp = entity->GetVecOrigin();
							ignorezmaterial->ColorModulate(CColor(SETTINGS::settings.backtrack_col[0] * 255, SETTINGS::settings.backtrack_col[1] * 255, SETTINGS::settings.backtrack_col[2] * 255, SETTINGS::settings.backtrack_col[3] * 255));
							INTERFACES::ModelRender->ForcedMaterialOverride(ignorezmaterial);
							INTERFACES::RenderView->SetBlend(0.7f);
							entity->DrawModel(0x1, 255);
							//RENDER::DrawLine(prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color);
						}
					}
					entity->SetAbsAnglesQang(oldAngs);
					entity->SetAbsOriginal(oldOrigin);

					static float kys[3] = { 0.f, 0.f, 0.f };
					kys[0] = 210 / 255.f;
					kys[1] = 210 / 255.f;
					kys[2] = 210 / 255.f;
					INTERFACES::RenderView->SetBlend(0.7);
					ignorezmaterial->ColorModulate(CColor(SETTINGS::settings.imodel_col[0] * 255, SETTINGS::settings.imodel_col[1] * 255, SETTINGS::settings.imodel_col[2] * 255, SETTINGS::settings.imodel_col[3] * 255));
					INTERFACES::ModelRender->ForcedMaterialOverride(ignorezmaterial);
					entity->DrawModel(0x1, 255);
					INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
					//}

				}
			}

			for (auto i = 0; i < INTERFACES::GlowObjManager->GetSize(); i++)
			{
				auto &glowObject = INTERFACES::GlowObjManager->m_GlowObjectDefinitions[i];
				auto entity = reinterpret_cast<SDK::CBaseEntity*>(glowObject.m_pEntity);

				if (!entity) continue;
				if (!local_player) continue;

				if (glowObject.IsUnused()) continue;

				bool is_local_player = entity == local_player;
				bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

				if (is_local_player && in_tp && SETTINGS::settings.glowlocal)
				{
					glowObject.m_nGlowStyle = SETTINGS::settings.glowstylelocal;
					glowObject.m_flRed = colorlocal.RGBA[0] / 255.0f;
					glowObject.m_flGreen = colorlocal.RGBA[1] / 255.0f;
					glowObject.m_flBlue = colorlocal.RGBA[2] / 255.0f;
					glowObject.m_flAlpha = colorlocal.RGBA[3] / 255.0f;
					glowObject.m_bRenderWhenOccluded = true;
					glowObject.m_bRenderWhenUnoccluded = false;
					continue;
				}
				else if (!SETTINGS::settings.glowlocal && is_local_player)
					continue;

				if (entity->GetHealth() <= 0) continue;
				if (entity->GetIsDormant())	continue;
				if (entity->GetClientClass()->m_ClassID != 38) continue;

				
				if (is_teammate && SETTINGS::settings.glowteam)
				{
					glowObject.m_nGlowStyle = SETTINGS::settings.glowstyle; //0;
					glowObject.m_flRed = colorTeam.RGBA[0] / 255.0f;
					glowObject.m_flGreen = colorTeam.RGBA[1] / 255.0f;
					glowObject.m_flBlue = colorTeam.RGBA[2] / 255.0f;
					glowObject.m_flAlpha = colorTeam.RGBA[3] / 255.0f;
					glowObject.m_bRenderWhenOccluded = true;
					glowObject.m_bRenderWhenUnoccluded = false;
					continue;
				}
				else if (is_teammate && !SETTINGS::settings.glowteam)
					continue;

				if (SETTINGS::settings.glowenable)
				{
					glowObject.m_nGlowStyle = SETTINGS::settings.glowstyle;//0;
					glowObject.m_flRed = color.RGBA[0] / 255.0f;
					glowObject.m_flGreen = color.RGBA[1] / 255.0f;
					glowObject.m_flBlue = color.RGBA[2] / 255.0f;
					glowObject.m_flAlpha = color.RGBA[3] / 255.0f;
					glowObject.m_bRenderWhenOccluded = true;
					glowObject.m_bRenderWhenUnoccluded = false;
				}
			}

			if (SETTINGS::settings.smoke_bool)
			{
				std::vector<const char*> vistasmoke_wireframe = { "particle/vistasmokev1/vistasmokev1_smokegrenade" };

				std::vector<const char*> vistasmoke_nodraw =
				{
					"particle/vistasmokev1/vistasmokev1_fire",
					"particle/vistasmokev1/vistasmokev1_emods",
					"particle/vistasmokev1/vistasmokev1_emods_impactdust",
				};

				for (auto mat_s : vistasmoke_wireframe)
				{
					SDK::IMaterial* mat = INTERFACES::MaterialSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
					mat->SetMaterialVarFlag(SDK::MATERIAL_VAR_WIREFRAME, true); //wireframe
				}

				for (auto mat_n : vistasmoke_nodraw)
				{
					SDK::IMaterial* mat = INTERFACES::MaterialSystem->FindMaterial(mat_n, TEXTURE_GROUP_OTHER);
					mat->SetMaterialVarFlag(SDK::MATERIAL_VAR_NO_DRAW, true);
				}

				static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);
				*(int*)(smokecout) = 0;
			}
		}
	}
	void __fastcall HookedOverrideView(void* ecx, void* edx, SDK::CViewSetup* pSetup)
	{
		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		if (!local_player) return;

		auto animstate = local_player->GetAnimState();
		if (!animstate) return;

		if (GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.thirdperson_int)) & 1)
			in_tp = !in_tp;

		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			if (GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.zoomkey)))
			{
				SETTINGS::settings.fov_val = 30;
			}
			else
			{
				SETTINGS::settings.fov_val = 90;
			}
			GrenadePrediction::instance().View(pSetup);
			auto GetCorrectDistance = [&local_player](float ideal_distance) -> float //lambda
			{
				Vector inverse_angles;
				INTERFACES::Engine->GetViewAngles(inverse_angles);

				inverse_angles.x *= -1.f, inverse_angles.y += 180.f;

				Vector direction;
				
				MATH::AngleVectors(inverse_angles, &direction);

				SDK::CTraceWorldOnly filter;
				SDK::trace_t trace;
				SDK::Ray_t ray;

				ray.Init(local_player->GetVecOrigin() + local_player->GetViewOffset(), (local_player->GetVecOrigin() + local_player->GetViewOffset()) + (direction * (ideal_distance + 5.f)));
				INTERFACES::Trace->TraceRay(ray, MASK_ALL, &filter, &trace);

				return ideal_distance * trace.flFraction;
			};

			if (SETTINGS::settings.tp_bool && in_tp) {

				if (local_player->GetHealth() <= 0)
					local_player->SetObserverMode(5);

				if (!INTERFACES::Input->m_fCameraInThirdPerson)
				{
					INTERFACES::Input->m_fCameraInThirdPerson = true;

					SDK::CBaseAnimState* animstate = local_player->GetAnimState();

					if (!animstate)
						return;

					if (animstate->m_bInHitGroundAnimation && animstate->m_flHeadHeightOrOffsetFromHittingGroundAnimation) {
						INTERFACES::Input->m_vecCameraOffset = Vector(-10, GLOBAL::angles.y, GetCorrectDistance(100));
					}
					else {
						INTERFACES::Input->m_vecCameraOffset = Vector(GLOBAL::angles.x, GLOBAL::angles.y, GetCorrectDistance(100));
					}
					Vector camForward;

					MATH::AngleVectors(Vector(INTERFACES::Input->m_vecCameraOffset.x, INTERFACES::Input->m_vecCameraOffset.y, 0), &camForward);
				}
			}
			else {
				INTERFACES::Input->m_fCameraInThirdPerson = false;
				INTERFACES::Input->m_vecCameraOffset = Vector(GLOBAL::angles.x, GLOBAL::angles.y, 0);
			}
			auto zoomsensration = INTERFACES::cvar->FindVar("zoom_sensitivity_ratio_mouse");
			if (SETTINGS::settings.fixscopesens)
				zoomsensration->SetValue("0");
			else
				zoomsensration->SetValue("1");

			if (SETTINGS::settings.aim_type == 0)
			{
				if (!local_player->GetIsScoped())
					pSetup->fov = SETTINGS::settings.fov_val;
				else if (local_player->GetIsScoped() && SETTINGS::settings.removescoping)
					pSetup->fov = SETTINGS::settings.fov_val;
			}
			else if (!(SETTINGS::settings.aim_type == 0) && !local_player->GetIsScoped())
				pSetup->fov = 90;
		}
		original_override_view(ecx, pSetup);
	}
	void __fastcall HookedTraceRay(void *thisptr, void*, const SDK::Ray_t &ray, unsigned int fMask, SDK::ITraceFilter *pTraceFilter, SDK::trace_t *pTrace)
	{
		original_trace_ray(thisptr, ray, fMask, pTraceFilter, pTrace);
		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
			pTrace->surface.flags |= SURF_SKY;
	}
	bool __fastcall HookedGetBool(void* pConVar, void* edx)
	{
		if ((uintptr_t)_ReturnAddress() == CAM_THINK)
			return true;

		return original_get_bool(pConVar);
	}
	float __fastcall GetViewmodelFOV()
	{
		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			float player_fov = original_viewmodel_fov();

			if (SETTINGS::settings.esp_bool)
				player_fov = SETTINGS::settings.viewfov_val;

			return player_fov;
		}
	}






	void OpenMenu()
	{
		static bool is_down = false;
		static bool is_clicked = false;
		if (GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.menu)))
		{
			is_clicked = false;
			is_down = true;
		}
		if (GetAsyncKeyState(VK_INSERT))
		{
			is_clicked = false;
			is_down = true;
		}
		else if (!GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.menu)) && is_down)
		{
			is_clicked = true;
			is_down = false;
		}
		else if (!GetAsyncKeyState(VK_INSERT) && is_down)
		{
			is_clicked = true;
			is_down = false;
		}
		else
		{
			is_clicked = false;
			is_down = false;
		}

		if (is_clicked)
		{
			menu_open = !menu_open;

		}
	}

	struct hud_weapons_t {
		std::int32_t* get_weapon_count() {
			return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
		}
	};
	void KnifeApplyCallbk()
	{

		static auto clear_hud_weapon_icon_fn =
			reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(
				UTILS::FindSignature("client_panorama.dll", "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C 89 5D FC"));

		auto element = FindHudElement<std::uintptr_t*>("CCSGO_HudWeaponSelection");

		auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xA0);
		if (hud_weapons == nullptr)
			return;

		if (!*hud_weapons->get_weapon_count())
			return;

		for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
			i = clear_hud_weapon_icon_fn(hud_weapons, i);

		static SDK::ConVar* Meme = INTERFACES::cvar->FindVar("cl_fullupdate");
		Meme->nFlags &= ~FCVAR_CHEAT;
		INTERFACES::Engine->ClientCmd_Unrestricted("cl_fullupdate");

	}
	SDK::CBaseWeapon* xd(SDK::CBaseEntity* xz)
	{
		if (!INTERFACES::Engine->IsConnected())
			return nullptr;
		if (!xz->IsAlive())
			return nullptr;

		HANDLE weaponData = *(HANDLE*)((DWORD)xz + OFFSETS::m_hActiveWeapon);
		return (SDK::CBaseWeapon*)INTERFACES::ClientEntityList->GetClientEntityFromHandle(weaponData);
	}

	short SafeWeaponID()
	{
		SDK::CBaseEntity* local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		if (!(local_player))
			return 0;

		SDK::CBaseWeapon* WeaponC = xd(local_player);

		if (!(WeaponC))
			return 0;

		return WeaponC->GetItemDefenitionIndex();
	}

	LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg) {
		case WM_LBUTTONDOWN:
			PressedKeys[VK_LBUTTON] = true;
			break;
		case WM_LBUTTONUP:
			PressedKeys[VK_LBUTTON] = false;
			break;
		case WM_RBUTTONDOWN:
			PressedKeys[VK_RBUTTON] = true;
			break;
		case WM_RBUTTONUP:
			PressedKeys[VK_RBUTTON] = false;
			break;
		case WM_MBUTTONDOWN:
			PressedKeys[VK_MBUTTON] = true;
			break;
		case WM_MBUTTONUP:
			PressedKeys[VK_MBUTTON] = false;
			break;
		case WM_XBUTTONDOWN:
		{
			UINT button = GET_XBUTTON_WPARAM(wParam);
			if (button == XBUTTON1)
			{
				PressedKeys[VK_XBUTTON1] = true;
			}
			else if (button == XBUTTON2)
			{
				PressedKeys[VK_XBUTTON2] = true;
			}
			break;
		}
		case WM_XBUTTONUP:
		{
			UINT button = GET_XBUTTON_WPARAM(wParam);
			if (button == XBUTTON1)
			{
				PressedKeys[VK_XBUTTON1] = false;
			}
			else if (button == XBUTTON2)
			{
				PressedKeys[VK_XBUTTON2] = false;
			}
			break;
		}
		case WM_KEYDOWN:
			PressedKeys[wParam] = true;
			break;
		case WM_KEYUP:
			PressedKeys[wParam] = false;
			break;
		default: break;
		}

		OpenMenu();

		if (d3d_init && menu_open && ImGui_ImplDX9_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		return CallWindowProc(INIT::OldWindow, hWnd, uMsg, wParam, lParam);
	}
	void purple()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(0.87f, 0.85f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.87f, 0.85f, 0.92f, 0.58f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.12f, 0.16f, 0.71f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.27f, 0.20f, 0.39f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.87f, 0.85f, 0.92f, 0.30f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.27f, 0.20f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.68f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.46f, 0.27f, 0.80f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.34f, 0.19f, 0.63f, 0.45f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.34f, 0.19f, 0.63f, 0.35f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.34f, 0.19f, 0.63f, 0.78f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.27f, 0.20f, 0.39f, 0.57f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.27f, 0.20f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.19f, 0.63f, 0.31f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.34f, 0.19f, 0.63f, 0.80f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.19f, 0.63f, 0.24f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.34f, 0.19f, 0.63f, 0.44f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.86f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.34f, 0.19f, 0.63f, 0.76f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.86f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.87f, 0.85f, 0.92f, 0.32f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.87f, 0.85f, 0.92f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.87f, 0.85f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.34f, 0.19f, 0.63f, 0.20f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.87f, 0.85f, 0.92f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.87f, 0.85f, 0.92f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.87f, 0.85f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.87f, 0.85f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.87f, 0.85f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.34f, 0.19f, 0.63f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void DefaultSheme1()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
		style.Colors[ImGuiCol_WindowBg] = ImColor(15, 15, 15, 255);
		style.Colors[ImGuiCol_ChildWindowBg] = ImColor(15, 15, 15, 255);
		style.Colors[ImGuiCol_Border] = ImColor(15, 15, 15, 255);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15f, 0.60f, 0.78f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.70f, 0.02f, 0.60f, 0.22f);
	}

	void RedSheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.58f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.71f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.92f, 0.18f, 0.29f, 0.37f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.92f, 0.18f, 0.29f, 0.75f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.15f, 0.00f, 0.00f, 0.35f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.59f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.92f, 0.18f, 0.29f, 0.63f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.78f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.92f, 0.18f, 0.29f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.22f, 0.27f, 0.73f);
	}

	void darkblue()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
	}

	void MidNightSheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(0.85f, 0.89f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.85f, 0.89f, 0.92f, 0.58f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.20f, 0.30f, 0.39f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.85f, 0.89f, 0.92f, 0.30f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.30f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.32f, 0.60f, 0.92f, 0.68f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.19f, 0.43f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.77f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.77f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.77f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.08f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.47f, 1.00f, 0.31f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.10f, 0.38f, 0.62f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.13f, 0.44f, 0.72f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.00f, 0.00f, 0.77f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.77f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.77f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.05f, 0.27f, 0.48f, 0.59f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.42f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.05f, 0.27f, 0.48f, 0.59f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 0.24f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.42f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.42f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.85f, 0.89f, 0.92f, 0.32f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.85f, 0.89f, 0.92f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.85f, 0.89f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.19f, 0.43f, 0.63f, 0.20f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.19f, 0.43f, 0.63f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.19f, 0.43f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.85f, 0.89f, 0.92f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.85f, 0.89f, 0.92f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.85f, 0.89f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.85f, 0.89f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.19f, 0.43f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.85f, 0.89f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.19f, 0.43f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.19f, 0.43f, 0.63f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void NightSheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.85f, 0.87f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.85f, 0.87f, 0.92f, 0.58f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.14f, 0.19f, 0.36f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.85f, 0.87f, 0.92f, 0.30f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.19f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.19f, 0.49f, 0.68f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.10f, 0.19f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.19f, 0.49f, 0.45f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.19f, 0.49f, 0.35f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.19f, 0.49f, 0.78f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.19f, 0.36f, 0.57f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.14f, 0.19f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.10f, 0.19f, 0.49f, 0.31f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.10f, 0.19f, 0.49f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.10f, 0.19f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.16f, 0.86f, 0.90f, 0.80f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.10f, 0.19f, 0.49f, 0.24f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.07f, 0.26f, 0.53f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.19f, 0.49f, 0.44f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.10f, 0.19f, 0.49f, 0.86f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.10f, 0.19f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.19f, 0.49f, 0.76f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.10f, 0.19f, 0.49f, 0.86f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.10f, 0.19f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.85f, 0.87f, 0.92f, 0.32f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.85f, 0.87f, 0.92f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.85f, 0.87f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.10f, 0.19f, 0.49f, 0.20f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.10f, 0.19f, 0.49f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.10f, 0.19f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.85f, 0.87f, 0.92f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.85f, 0.87f, 0.92f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.85f, 0.87f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.85f, 0.87f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.10f, 0.19f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.85f, 0.87f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.10f, 0.19f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.10f, 0.19f, 0.49f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void DunnoSheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.65f, 0.00f, 0.06f, 0.03f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.06f, 0.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.00f, 0.06f, 0.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.65f, 0.00f, 0.06f, 3.14f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.65f, 0.00f, 0.06f, 3.14f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.65f, 0.00f, 0.06f, 3.14f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 3.14f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 3.14f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.65f, 0.00f, 0.06f, 3.14f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.65f, 0.00f, 0.06f, 3.14f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.65f, 0.00f, 0.06f, 3.14f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.00f, 0.06f, 3.14f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65f, 0.00f, 0.06f, 3.14f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.06f, 3.14f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.42f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.05f, 0.27f, 0.48f, 0.59f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 0.24f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.42f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.42f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.85f, 0.89f, 0.92f, 0.32f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.85f, 0.89f, 0.92f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.85f, 0.89f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.19f, 0.43f, 0.63f, 0.20f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.19f, 0.43f, 0.63f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.19f, 0.43f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.85f, 0.89f, 0.92f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.85f, 0.89f, 0.92f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.85f, 0.89f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.85f, 0.89f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.19f, 0.43f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.85f, 0.89f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.19f, 0.43f, 0.63f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.19f, 0.43f, 0.63f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void BlueSheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f); // - 
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.13f, 0.65f, 0.92f, 0.78f); // - 
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f); // - 
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.58f); // - 
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f); // - 
		style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f); // - 
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // - 
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f); // - 
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.13f, 0.65f, 0.92f, 0.78f); // + 
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f); // - 
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f); // - 
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.47f); // - 
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f); // - 
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f); // - 
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.13f, 0.65f, 0.92f, 0.78f); // + 
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.13f, 0.65f, 0.92f, 0.37f); // + 
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_Button] = ImVec4(0.13f, 0.65f, 0.92f, 0.75f); // + 
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.65f, 0.92f, 0.86f); // + 
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_Header] = ImVec4(0.13f, 0.65f, 0.92f, 0.76f); // + 
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.65f, 0.92f, 0.86f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_Column] = ImVec4(0.15f, 0.00f, 0.00f, 0.35f); // - 
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.13f, 0.65f, 0.92f, 0.59f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.13f, 0.65f, 0.92f, 0.63f); // + 
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.13f, 0.65f, 0.92f, 0.78f); // + 
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_CloseButton] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f); // - 
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f); // - 
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.78f); // - 
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.13f, 0.65f, 0.92f, 1.00f); // + 
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.13f, 0.65f, 0.92f, 0.43f); // + 
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.22f, 0.27f, 0.73f); // - 
	}

	void MidNight2()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void BlackSheme2()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
	}

	void green()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(0.89f, 0.92f, 0.85f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.89f, 0.92f, 0.85f, 0.58f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.17f, 0.20f, 0.14f, 0.55f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.37f, 0.49f, 0.20f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.89f, 0.92f, 0.85f, 0.30f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.37f, 0.49f, 0.20f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.53f, 0.78f, 0.17f, 0.68f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.53f, 0.78f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.53f, 0.78f, 0.17f, 0.45f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.53f, 0.78f, 0.17f, 0.35f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.53f, 0.78f, 0.17f, 0.78f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.37f, 0.49f, 0.20f, 0.57f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.37f, 0.49f, 0.20f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.53f, 0.78f, 0.17f, 0.31f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.53f, 0.78f, 0.17f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.53f, 0.78f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.53f, 0.78f, 0.17f, 0.80f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.53f, 0.78f, 0.17f, 0.24f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.53f, 0.78f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.53f, 0.78f, 0.17f, 0.44f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.53f, 0.78f, 0.17f, 0.86f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.53f, 0.78f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.53f, 0.78f, 0.17f, 0.76f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.53f, 0.78f, 0.17f, 0.86f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.53f, 0.78f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.89f, 0.92f, 0.85f, 0.32f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.89f, 0.92f, 0.85f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.89f, 0.92f, 0.85f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.53f, 0.78f, 0.17f, 0.20f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.53f, 0.78f, 0.17f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.53f, 0.78f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.89f, 0.92f, 0.85f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.89f, 0.92f, 0.85f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.89f, 0.92f, 0.85f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.89f, 0.92f, 0.85f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.53f, 0.78f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.89f, 0.92f, 0.85f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.53f, 0.78f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.53f, 0.78f, 0.17f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	}

	void pink()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.85f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.92f, 0.92f, 0.85f, 0.58f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.20f, 0.19f, 0.14f, 0.55f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.49f, 0.47f, 0.20f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.92f, 0.92f, 0.85f, 0.30f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.49f, 0.47f, 0.20f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.78f, 0.75f, 0.17f, 0.68f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.78f, 0.75f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.78f, 0.75f, 0.17f, 0.45f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.78f, 0.75f, 0.17f, 0.35f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.78f, 0.75f, 0.17f, 0.78f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.49f, 0.47f, 0.20f, 0.57f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.49f, 0.47f, 0.20f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.78f, 0.75f, 0.17f, 0.31f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.78f, 0.75f, 0.17f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.78f, 0.75f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.78f, 0.75f, 0.17f, 0.80f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.78f, 0.75f, 0.17f, 0.24f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.78f, 0.75f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.78f, 0.75f, 0.17f, 0.44f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.78f, 0.75f, 0.17f, 0.86f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.78f, 0.75f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.78f, 0.75f, 0.17f, 0.76f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.78f, 0.75f, 0.17f, 0.86f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.78f, 0.75f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.92f, 0.92f, 0.85f, 0.32f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.92f, 0.92f, 0.85f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.92f, 0.92f, 0.85f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.78f, 0.75f, 0.17f, 0.20f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.78f, 0.75f, 0.17f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.75f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.92f, 0.92f, 0.85f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.92f, 0.92f, 0.85f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.92f, 0.92f, 0.85f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.92f, 0.92f, 0.85f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.78f, 0.75f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.92f, 0.92f, 0.85f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.78f, 0.75f, 0.17f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.78f, 0.75f, 0.17f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}
	void blue()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(0.85f, 0.87f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.85f, 0.87f, 0.92f, 0.58f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.15f, 0.20f, 0.55f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.20f, 0.27f, 0.49f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.85f, 0.87f, 0.92f, 0.30f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.27f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.31f, 0.78f, 0.68f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.17f, 0.31f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.17f, 0.31f, 0.78f, 0.45f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.31f, 0.78f, 0.35f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.17f, 0.31f, 0.78f, 0.78f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.27f, 0.49f, 0.57f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.27f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.17f, 0.31f, 0.78f, 0.31f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.17f, 0.31f, 0.78f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.17f, 0.31f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.17f, 0.31f, 0.78f, 0.80f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.17f, 0.31f, 0.78f, 0.24f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.17f, 0.31f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.17f, 0.31f, 0.78f, 0.44f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.17f, 0.31f, 0.78f, 0.86f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.17f, 0.31f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.17f, 0.31f, 0.78f, 0.76f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.31f, 0.78f, 0.86f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.17f, 0.31f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.85f, 0.87f, 0.92f, 0.32f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.85f, 0.87f, 0.92f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.85f, 0.87f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.17f, 0.31f, 0.78f, 0.20f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.17f, 0.31f, 0.78f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.17f, 0.31f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.85f, 0.87f, 0.92f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.85f, 0.87f, 0.92f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.85f, 0.87f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.85f, 0.87f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.17f, 0.31f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.85f, 0.87f, 0.92f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.17f, 0.31f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.17f, 0.31f, 0.78f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	}

	void yellow()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.51f, 0.51f, 0.51f, 0.55f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.35f, 0.33f, 0.33f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.86f, 0.86f, 0.86f, 0.30f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.35f, 0.33f, 0.33f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.82f, 0.82f, 0.82f, 0.92f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(1.02f, 0.94f, 0.94f, 0.45f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.02f, 0.94f, 0.94f, 0.35f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.02f, 0.94f, 0.94f, 0.78f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.33f, 0.33f, 0.57f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.35f, 0.33f, 0.33f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.02f, 0.94f, 0.94f, 0.31f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.02f, 0.94f, 0.94f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.02f, 0.94f, 0.94f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.02f, 0.94f, 0.94f, 0.80f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.02f, 0.94f, 0.94f, 0.24f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.02f, 0.94f, 0.94f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(1.02f, 0.94f, 0.94f, 0.44f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.02f, 0.94f, 0.94f, 0.86f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.02f, 0.94f, 0.94f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(1.02f, 0.94f, 0.94f, 0.76f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.02f, 0.94f, 0.94f, 0.86f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.02f, 0.94f, 0.94f, 1.00f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.92f, 0.85f, 0.85f, 0.32f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.92f, 0.85f, 0.85f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.92f, 0.85f, 0.85f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.02f, 0.94f, 0.94f, 0.20f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.02f, 0.94f, 0.94f, 0.78f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.02f, 0.94f, 0.94f, 1.00f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.92f, 0.85f, 0.85f, 0.16f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.92f, 0.85f, 0.85f, 0.39f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.92f, 0.85f, 0.85f, 0.63f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.02f, 0.94f, 0.94f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.92f, 0.85f, 0.85f, 0.63f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.02f, 0.94f, 0.94f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.02f, 0.94f, 0.94f, 0.43f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}


	void BlackSheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;               // Global alpha applies to everything in ImGui
		style.WindowPadding = ImVec2(10, 10);     // Padding within a window
		style.WindowMinSize = ImVec2(100, 100);   // Minimum window size
		style.WindowRounding = 0.0f;               // Radius of window corners rounding. Set to 0.0f to have rectangular windows
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f); // Alignment for title bar text
		style.FramePadding = ImVec2(4, 3);       // Padding within a framed rectangle (used by most widgets)
		style.FrameRounding = 0.0f;               // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
		style.ItemSpacing = ImVec2(5, 5);       // Horizontal and vertical spacing between widgets/lines
		style.ItemInnerSpacing = ImVec2(4, 4);       // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
		style.TouchExtraPadding = ImVec2(0, 0);       // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
		style.IndentSpacing = 21.0f;              // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
		style.ColumnsMinSpacing = 6.0f;               // Minimum horizontal spacing between two columns
		style.ScrollbarSize = 16.0f;              // Width of the vertical scrollbar, Height of the horizontal scrollbar
		style.ScrollbarRounding = 9.0f;               // Radius of grab corners rounding for scrollbar
		style.GrabMinSize = 10.0f;              // Minimum width/height of a grab box for slider/scrollbar
		style.GrabRounding = 0.0f;               // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f); // Alignment of button text when button is larger than text.
		style.DisplayWindowPadding = ImVec2(22, 22);     // Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
		style.DisplaySafeAreaPadding = ImVec2(4, 4);       // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
		style.AntiAliasedLines = true;               // Enable anti-aliasing on lines/borders. Disable if you are really short on CPU/GPU.
		style.CurveTessellationTol = 1.25f;              // Tessellation tolerance. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
		style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.25f, 0.90f, 0.83f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.25f, 0.90f, 0.20f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.25f, 0.90f, 0.87f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.80f, 0.50f, 0.50f, 0.40f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_WindowBg];
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.46f, 0.65f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.25f, 0.90f, 0.83f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.50f, 0.50f, 0.90f, 0.50f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.70f, 0.70f, 0.90f, 0.60f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}
	static int iTab;
	const char* key_binds[] = { "none", "mouse1", "mouse2", "mouse3", "mouse4", "mouse5", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12" };
	const char* key_binds2[] = { "insert", "mouse1", "mouse2", "mouse3", "mouse4", "mouse5", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12" };
	void DrawAim() {
		ImGui::BeginChild("5e6gtvref", ImVec2(450, 0), true);
		{
			ImGui::PushFont(smallmenu_font);
			{

				const char* aim_mode[] = { "rage", "legit" };
				const char* acc_mode[] = { "head", "body aim", "hitscan","smart-baim","smart-velocity"};

				const char* delay_shot[] = { "off", "lag compensation" };
				const char* override_method[] = { "set", "key-press" };
				ImGui::Checkbox(("Enable aimbot"), &SETTINGS::settings.aim_bool);
				ImGui::Combo(("Aimbot Type"), &SETTINGS::settings.aim_type, aim_mode, ARRAYSIZE(aim_mode));
				if (SETTINGS::settings.aim_type == 0)
				{
					ImGui::Combo(("Aimbot Mode"), &SETTINGS::settings.acc_type, acc_mode, ARRAYSIZE(acc_mode));
					ImGui::Checkbox(("Hit-Change List"), &SETTINGS::settings.hit_list);
					if (SETTINGS::settings.hit_list)
					{
						ImGui::Text("Hit-Chance");
						ImGui::SliderFloat("auto", &SETTINGS::settings.auto_chance_val, 0, 100,"%.0f");
						ImGui::SliderFloat("scout", &SETTINGS::settings.scout_chance_val, 0, 100, "%.0f");
						ImGui::SliderFloat("awp", &SETTINGS::settings.awp_chance, 0, 100, "%.0f");
						ImGui::SliderFloat("pistols", &SETTINGS::settings.pistols_chance_val, 0, 100, "%.0f");
						ImGui::SliderFloat("revolver/deagle", &SETTINGS::settings.revolver_chance_val, 0, 100, "%.0f");

						ImGui::SliderFloat("other", &SETTINGS::settings.other_chance_val, 0, 100, "%.0f");
						ImGui::SliderFloat("taser", &SETTINGS::settings.zeus_chance_val, 0, 100, "%.0f");
					}
					ImGui::Checkbox(("Damage List"), &SETTINGS::settings.damage_list);
					if (SETTINGS::settings.damage_list)
					{
						ImGui::Text("Minimum Damage");
						ImGui::SliderFloat("auto damage", &SETTINGS::settings.auto_damage_val, 0, 100, "%.0f");
						ImGui::SliderFloat("scout damage", &SETTINGS::settings.scout_damage_val, 0, 100, "%.0f");
						ImGui::SliderFloat("awp damage", &SETTINGS::settings.awp_mindamag, 0, 100, "%.0f");
						ImGui::SliderFloat("pistol damages", &SETTINGS::settings.pistols_damage_val, 0, 100, "%.0f");
						ImGui::SliderFloat("revolver/deagle damage", &SETTINGS::settings.revolver_damage_val, 0, 100, "%.0f");
						ImGui::SliderFloat("other damage", &SETTINGS::settings.other_damage_val, 0, 100, "%.0f");
						ImGui::SliderFloat("taser damage", &SETTINGS::settings.zeus_min_val, 0, 100, "%.0f");
					}
					ImGui::Checkbox(("More Aimpoints"), &SETTINGS::settings.multi_bool);
					if (SETTINGS::settings.multi_bool)
					{
						ImGui::SliderFloat("Head Scale", &SETTINGS::settings.point_val, 0, 1);
						ImGui::SliderFloat("Body Scale", &SETTINGS::settings.body_val, 0, 1);
					}
					ImGui::Checkbox(("Safe Aimpoints/ no crash btw"), &SETTINGS::settings.multi_bool2);
					if (SETTINGS::settings.multi_bool2)
					{
						SETTINGS::settings.multi_bool = false;
						SETTINGS::settings.baimfakewalk = false;
						SETTINGS::settings.baim = false;
						ImGui::SliderFloat("Head Scale", &SETTINGS::settings.point_val, 0, 1);
						ImGui::SliderFloat("Body Scale", &SETTINGS::settings.body_val, 0, 1);
					}
					
					//	ImGui::Checkbox("auto hit-chance", &SETTINGS::settings.autohitchance);
					//	ImGui::Checkbox("auto damage", &SETTINGS::settings.automindmg);
					//	ImGui::Checkbox("auto scale", &SETTINGS::settings.autoscale);
					ImGui::SliderFloat("Taser range", &SETTINGS::settings.zeus_range, 150, 190, "%.0f");
					ImGui::Checkbox(("Auto Stop"), &SETTINGS::settings.stop_bool);
					/*if (SETTINGS::settings.stop_bool)
					{
					ImGui::Combo(("stop key"), &SETTINGS::settings.autosop2, key_binds, ARRAYSIZE(key_binds));
				    }*/
					ImGui::Checkbox(("fakelag prediction"), &SETTINGS::settings.fake_bool3);
					ImGui::Checkbox(("draw velocity"), &SETTINGS::settings.velocity);
				//	ImGui::Checkbox(("Auto Stop (+)"), &SETTINGS::settings.quickstops);
					//ImGui::Checkbox(("Lagcompensator"), &SETTINGS::settings.rage_lagcompensation);
					//const char* delay_shot[] = { "off", "lag compensation" };
					ImGui::Combo("delay shot", &SETTINGS::settings.delay_shot, delay_shot, ARRAYSIZE(delay_shot));
					ImGui::Checkbox("new fakelatency", &SETTINGS::settings.fakelatency_enabled);
					ImGui::SliderFloat("fake amounts", &SETTINGS::settings.fakelatency_amount, 0.0f, 1000.f, ("%.1f"));
					ImGui::Checkbox(("fakelag fix"), &SETTINGS::settings.fakefix_bool);
					ImGui::Checkbox(("BAIM on fakewalk"), &SETTINGS::settings.baimfakewalk);
					ImGui::Checkbox(("BAIM after x shots"), &SETTINGS::settings.antiaw);
					if (SETTINGS::settings.antiaw)
					{
						ImGui::SliderFloat("baim after", &SETTINGS::settings.poslemissed, 0, 5, "%.0f");
					}

					ImGui::Checkbox(("BAIM on key"), &SETTINGS::settings.baim);
					if (SETTINGS::settings.baim)
					{
						ImGui::Combo(("resolver key"), &SETTINGS::settings.desresolver, key_binds, ARRAYSIZE(key_binds));
						ImGui::Combo(("BAIM key"), &SETTINGS::settings.baimkey, key_binds, ARRAYSIZE(key_binds));
					}
					//std::vector<std::string> res_type = { "phack", "old phack", "samoware", "beta" };
					//const char* res_type[] = { "phack", "old phack", "samoware", "beta" };
					ImGui::Checkbox(("Resolver"), &SETTINGS::settings.resolve_bool);
					ImGui::Checkbox(("Override Enable"), &SETTINGS::settings.overrideenable);
					ImGui::Combo(("Override Key"), &SETTINGS::settings.overridekey, key_binds, ARRAYSIZE(key_binds));
					ImGui::Combo(("Override Method"), &SETTINGS::settings.overridemethod, override_method, ARRAYSIZE(override_method));
				//	ImGui::Combo(("Override Method"), &SETTINGS::settings.delay_shot, delay_shot, ARRAYSIZE(delay_shot));
				}
				else
				{
					ImGui::Checkbox(("Triggerbot"), &SETTINGS::settings.legittrigger_bool);
					ImGui::Combo(("Triggerbot key"), &SETTINGS::settings.legittrigger_key, key_binds, ARRAYSIZE(key_binds));
					ImGui::Checkbox(("Backtrack Legit"), &SETTINGS::settings.back_bool);
				}

			}
			ImGui::PopFont();
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("wrqwerfadf", ImVec2(280, 0), true);
		{
			ImGui::PushFont(smallmenu_font);
			{
				const char* antiaimmode[] = { "standing", "moving", "jumping" };
				const char* aa_pitch[] = { "none", "emotion", "fake down", "fake up", "fake zero","random","random desync","minimal" };
				const char* aa_mode[] = { "none", "backwards", "sideways", "backjitter", "lowerbody", "legit troll", "rotational", "freestanding","desync","lowerbody pysen","freestand(2)","manual-jitter"};
				const char* aa_fake[] = { "none", "desync","jitter(-)","desync-static","mega desync","decync(+)","desync-breaker","beta","zAnti","Adaptive-jitter","Adaptive"};

				ImGui::Checkbox(("Enable Antiaim"), &SETTINGS::settings.aa_bool);
				ImGui::Combo(("Mode##AA"), &SETTINGS::settings.aa_mode, antiaimmode, ARRAYSIZE(antiaimmode));

				switch (SETTINGS::settings.aa_mode)
				{
				case 0:

					ImGui::Combo(("Pitch##st"), &SETTINGS::settings.aa_pitch_type, aa_pitch, ARRAYSIZE(aa_pitch));

					ImGui::Combo(("Real##st"), &SETTINGS::settings.aa_real_type, aa_mode, ARRAYSIZE(aa_mode));

					ImGui::Combo(("Fake##st"), &SETTINGS::settings.aa_fake_type, aa_fake, ARRAYSIZE(aa_fake));


					break;
				case 1:
					ImGui::Combo(("Pitch##moving"), &SETTINGS::settings.aa_pitch1_type, aa_pitch, ARRAYSIZE(aa_pitch));

					ImGui::Combo(("Real##moving"), &SETTINGS::settings.aa_real1_type, aa_mode, ARRAYSIZE(aa_mode));

					ImGui::Combo(("Fake##moving"), &SETTINGS::settings.aa_fake1_type, aa_fake, ARRAYSIZE(aa_fake));



					break;
				case 2:
					ImGui::Combo(("Pitch##jumping"), &SETTINGS::settings.aa_pitch2_type, aa_pitch, ARRAYSIZE(aa_pitch));

					ImGui::Combo(("Real##jumping"), &SETTINGS::settings.aa_real2_type, aa_mode, ARRAYSIZE(aa_mode));

					ImGui::Combo(("Fake##jumping"), &SETTINGS::settings.aa_fake2_type, aa_fake, ARRAYSIZE(aa_fake));


					break;
				}

				ImGui::Checkbox(("Switch desync(shift)"), &SETTINGS::settings.fakewalk);

				ImGui::Checkbox(("Anti-Aim Arrows"), &SETTINGS::settings.rifk_arrow);
				ImGui::Combo(("Flip Key"), &SETTINGS::settings.flip_bool, key_binds, ARRAYSIZE(key_binds));
				switch (SETTINGS::settings.aa_mode)
				{
				case 0:

					ImGui::SliderFloat("Real##st", &SETTINGS::settings.aa_realadditive_val, -180, 180, "%.0f");
					ImGui::SliderFloat("Fake##st", &SETTINGS::settings.aa_fakeadditive_val, -180, 180, "%.0f");
					ImGui::SliderFloat("Delta##st", &SETTINGS::settings.delta_val, -119.9, 119.9, "%.0f");
					break;
				case 1:

					ImGui::SliderFloat("Real##mv", &SETTINGS::settings.aa_realadditive1_val, -180, 180, "%.0f");
					ImGui::SliderFloat("Fake##mv", &SETTINGS::settings.aa_fakeadditive1_val, -180, 180, "%.0f");
					ImGui::SliderFloat("Delta##mv", &SETTINGS::settings.delta1_val, -119.9, 119.9, "%.0f");
					break;
				case 2:

					ImGui::SliderFloat("Real##ar", &SETTINGS::settings.aa_realadditive2_val, -180, 180, "%.0f");
					ImGui::SliderFloat("Fake##ar", &SETTINGS::settings.aa_fakeadditive2_val, -180, 180, "%.0f");
					ImGui::SliderFloat("Delta##ar", &SETTINGS::settings.delta2_val, -119.9, 119.9, "%.0f");
					break;
				}


				switch (SETTINGS::settings.aa_mode)
				{
				case 0:
					ImGui::SliderFloat("Standing °", &SETTINGS::settings.spinangle, 0, 180, "%.0f");
					ImGui::SliderFloat("Standing %", &SETTINGS::settings.spinspeed, 0, 100, "%.0f");
					break;
				case 1:
					ImGui::SliderFloat("Moving °", &SETTINGS::settings.spinangle1, 0, 180, "%.0f");
					ImGui::SliderFloat("Moving %", &SETTINGS::settings.spinspeed1, 0, 100, "%.0f");
					break;
				case 2:
					ImGui::SliderFloat("Jumping °", &SETTINGS::settings.spinangle2, 0, 180, "%.0f");
					ImGui::SliderFloat("Jumping %", &SETTINGS::settings.spinspeed2, 0, 100, "%.0f");
					break;
				}


				ImGui::SliderFloat("Fake °", &SETTINGS::settings.spinanglefake, 0, 180, "%.0f");
				ImGui::SliderFloat("Fake %", &SETTINGS::settings.spinspeedfake, 0, 100, "%.0f");

			}
			ImGui::PopFont();
		}
		ImGui::EndChild();

		ImGui::SameLine();
	}
	void DrawVis() {
		ImGui::BeginChild("5e6gtvref", ImVec2(300, 0), true);
		{
			ImGui::PushFont(smallmenu_font);
			{
				const char* weptype[] =
				{
					"type1",
					"type2"

				};

				ImGui::Checkbox(("Enabled"), &SETTINGS::settings.esp_bool);
			//	ImGui::Checkbox(("D9Visual"), &SETTINGS::settings.Visuals.Enabled);
				ImGui::Checkbox(("Box"), &SETTINGS::settings.box_bool);
			//	ImGui::Combo(("Box Type"), &SETTINGS::settings.Visuals.Visuals_BoxType, weptype, ARRAYSIZE(weptype));
			//	ImGui::Checkbox(("Health"), &SETTINGS::settings.Visuals.Visuals_HealthBar);
			//	ImGui::Combo(("Health Type"), &SETTINGS::settings.Visuals.Visuals_HealthBarType, weptype, ARRAYSIZE(weptype));
				ImGui::Checkbox(("Name"), &SETTINGS::settings.name_bool);
				ImGui::Checkbox("Draw flags", &SETTINGS::settings.info_bool);
				ImGui::Checkbox("Draw money", &SETTINGS::settings.money_bool);
				ImGui::Checkbox(("Health"), &SETTINGS::settings.health_bool);
				const char* fpss[] = { "None", "Fpsboost+", "Fpsboost" };
				ImGui::Combo(("Fps boost"), &SETTINGS::settings.fps_mod, fpss, ARRAYSIZE(fpss));

				ImGui::Checkbox(("Weapons"), &SETTINGS::settings.weap_bool);
				ImGui::Checkbox(("Fov Arrows"), &SETTINGS::settings.fov_bool);
				ImGui::Checkbox(("Draw taser range"), &SETTINGS::settings.zeusrange);
				//ImGui::Checkbox(("Draw dropweapon"), &SETTINGS::settings.drawdrop);
				ImGui::Checkbox(("lagcomp"), &SETTINGS::settings.lagcomphit);
				if (SETTINGS::settings.lagcomphit)
				{
					ImGui::SliderFloat(("Alpha"), &SETTINGS::settings.lagcompalpha, 0, 255, "%.0f");
					ImGui::SliderFloat(("Duration"), &SETTINGS::settings.lagcomptime, 0, 10, "%.0f");
				}
			
			//	ImGui::Checkbox(("Enabled"), &SETTINGS::settings.Visuals.Enabled);
			//	ImGui::Checkbox(("Enabled"), &SETTINGS::settings.esp_bool);
				/*ImGui::Checkbox(("Box"), &SETTINGS::settings.Visuals.Visuals_BoxEnabled);
				ImGui::Text("Box Type");
				ImGui::Combo(("##box Type"), &SETTINGS::settings.Visuals.Visuals_BoxType, weptype, ARRAYSIZE(weptype));
				ImGui::Checkbox(("Esp team"), &SETTINGS::settings.Visuals.Visuals_EspTeam);
				ImGui::Checkbox(("Name"), &SETTINGS::settings.name_bool);
				ImGui::Checkbox(("Health"), &SETTINGS::settings.Visuals.Visuals_HealthBar);
				ImGui::Text("Health Type");
				ImGui::Combo(("##health type"), &SETTINGS::settings.Visuals.Visuals_HealthBarType, weptype, ARRAYSIZE(weptype));

				ImGui::Checkbox(("Aim Lines"), &SETTINGS::settings.Visuals.Visuals_AimLines);
				ImGui::Checkbox(("Weapons"), &SETTINGS::settings.weap_bool);
				ImGui::Text("Weapon Type");
				ImGui::Combo(("##weaponType"), &SETTINGS::settings.Visuals.Visuals_WeaponsType, weptype, ARRAYSIZE(weptype));
				ImGui::Checkbox(("Ammo"), &SETTINGS::settings.ammo_bool);
				ImGui::Text("Ammo Type");
				ImGui::Combo(("##ammoType"), &SETTINGS::settings.Visuals.Visuals_AmmoESPType, weptype, ARRAYSIZE(weptype));
				ImGui::Checkbox(("Draw taser range"), &SETTINGS::settings.zeusrange);
				ImGui::Checkbox(("Damage Esp"), &SETTINGS::settings.draw_er);
				ImGui::Checkbox(("POV Arrows"), &SETTINGS::settings.fov_bool);*/
				ImGui::Checkbox(("awhitmarker"), &SETTINGS::settings.hitmarker22);
				ImGui::ColorEdit4("damage color", SETTINGS::settings.awhit, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
				const char* crosshair_select[] = { "none", "static", "recoil" };
				const char* hitmarker[] = { "none", "gamesense", "bameware", "custom" };
				const char* spreadshit[] = { "off", "standart", "another", "rainbow" };
				const char* skyboxes[] = { "no sky", "cs_baggage", "cs_tibet", "embassy", "italy", "jungle", "office", "sky_cs15_daylight01", "sky_cs15_daylight02", "sky_day02_05", "nukeblank", "sky_venice", "sky_cs15_daylight03", "sky_cs15_daylight04", "sky_csgo_cloudy", "sky_csgo_night", "sky_csgo_night 2", "vertigo", "vertigoblue_hdr", "sky_dust", "vietnam" , "skeet test" , "skeet test" };
				ImGui::Checkbox("full bright", &SETTINGS::settings.full_bright);
				ImGui::Checkbox("night mode", &SETTINGS::settings.night_bool);
				
				ImGui::Checkbox("draw error", &SETTINGS::settings.draw_error);
				bool missed;
				bool did_hit = false;
				if (SETTINGS::settings.draw_error)
				{
					if (!did_hit)
						missed = true;
				

					if (missed) {
						ConMsg("");
					
						missed = false;
					}
					did_hit = false;
				}
				ImGui::Checkbox("world color", &SETTINGS::settings.worldcolorchanger);
				if (SETTINGS::settings.worldcolorchanger) {
					ImGui::ColorEdit4("world color", SETTINGS::settings.wcolorchanger, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview);
				}
				ImGui::Checkbox("sky color", &SETTINGS::settings.sky_color);
				if (SETTINGS::settings.sky_color) {
					ImGui::ColorEdit4("sky color", SETTINGS::settings.sky_ccolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview);
				}
				ImGui::Checkbox("Skybox Changer", &SETTINGS::settings.sky_changer);
				if (SETTINGS::settings.sky_changer)
				{
					ImGui::Combo("skybox type", &SETTINGS::settings.sky_type, skyboxes, ARRAYSIZE(skyboxes));
				}
				ImGui::Checkbox("Update (sky color,world)", &SETTINGS::settings.trash);
				ImGui::Checkbox("backtrack chams", &SETTINGS::settings.backtrack_show);
				if (SETTINGS::settings.backtrack_show)
				{
					ImGui::ColorEdit4("backtrack_col", SETTINGS::settings.backtrack_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview);
				}
			//	ImGui::Checkbox("Damage Esp", &SETTINGS::settings.draw_er);
			//	ImGui::ColorEdit4("damage color", SETTINGS::settings.Visuals.DamageESPColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview);
				//ImGui::SliderFloat("night value", &SETTINGS::settings.daytimevalue, 0, 100);
				//	ImGui::Checkbox("night mode", &SETTINGS::settings.night_bool2);
				//	ImGui::SliderFloat("night value", 0, 100, SETTINGS::settings.night_loh,0, 50);
				ImGui::Checkbox(("bullet tracers"), &SETTINGS::settings.beam_bool);

				ImGui::SliderFloat("bullet life", &SETTINGS::settings.bulletlife, 0, 30);
				ImGui::SliderFloat("bullet size", &SETTINGS::settings.bulletsize, 0, 20);


				ImGui::Checkbox(("thirdperson"), &SETTINGS::settings.tp_bool);
				//ImGui::Checkbox(("thirdperson2"), &SETTINGS::settings.dafakfkads);
				//ImGui::Combo(("thirdperson spoof"), &SETTINGS::settings.thirdspoof, key_binds, ARRAYSIZE(key_binds));
				ImGui::Combo(("thirdperson key"), &SETTINGS::settings.thirdperson_int, key_binds, ARRAYSIZE(key_binds));
				ImGui::SliderFloat(("third transparency"), &SETTINGS::settings.transparency_amnt, 0, 1);
			//	ImGui::Combo(("crosshair"), &SETTINGS::settings.xhair_type, crosshair_select, ARRAYSIZE(crosshair_select));



				ImGui::Combo(("Spread circle"), &SETTINGS::settings.spread_bool, spreadshit, ARRAYSIZE(spreadshit));
				ImGui::Checkbox("ragdol", &SETTINGS::settings.ragdol);
				ImGui::Checkbox("world blend", &SETTINGS::settings.asuswalls);
				if (SETTINGS::settings.asuswalls)
				{
					ImGui::SliderFloat("value", &SETTINGS::settings.asuswallsvalue, 0, 100, "%.0f");
				}
				ImGui::Checkbox("Asus props", &SETTINGS::settings.asus_props);
				//		ImGui::Checkbox("world color changer", &SETTINGS::settings.wolrd_enabled);
				//ImGui::ColorEdit4(("##world_color"), SETTINGS::settings.world_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
				//	ImGui::Checkbox("sky color changer",&SETTINGS::settings.sky_enabled);
				//ImGui::ColorEdit4(("##skycolor"), SETTINGS::settings.skycolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);


				ImGui::Checkbox(("remove smoke"), &SETTINGS::settings.smoke_bool);
				ImGui::Checkbox(("remove scope"), &SETTINGS::settings.scope_bool);
				ImGui::Checkbox(("remove zoom"), &SETTINGS::settings.removescoping);
				ImGui::Combo(("zoom key"), &SETTINGS::settings.zoomkey, key_binds, ARRAYSIZE(key_binds));
				ImGui::Checkbox(("fix zoom sensitivity"), &SETTINGS::settings.fixscopesens);
				ImGui::Checkbox(("enable postprocessing"), &SETTINGS::settings.matpostprocessenable);
				//	ImGui::Combo(("flashlight key"), &SETTINGS::settings.flashlightkey, key_binds, ARRAYSIZE(key_binds));
				ImGui::SliderFloat("render fov", &SETTINGS::settings.fov_val, 0, 179);
				ImGui::SliderFloat("viewmodel fov", &SETTINGS::settings.viewfov_val, 0, 179);
				ImGui::Combo(("hitmarker sound"), &SETTINGS::settings.hitmarker_val, hitmarker, ARRAYSIZE(hitmarker));
				ImGui::Checkbox(("lby indicator"), &SETTINGS::settings.lbyenable);
				ImGui::Text(("Radar"));
				ImGui::Separator();
				ImGui::Checkbox("Enable Radar", &SETTINGS::settings.Radargui.Enabled);
				ImGui::SliderFloat("Range", &SETTINGS::settings.Radargui.Range, 1, 5);
				ImGui::Checkbox("Only Enemy", &SETTINGS::settings.Radargui.OnlyEnemy);
				//ImGui::Checkbox(("fowardbacktrack"), &SETTINGS::settings.forwardbt);
				ImGui::SameLine(ImGui::GetWindowWidth() - 25);


			}
			ImGui::PopFont();
		}
		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::BeginChild("agarwegwqerg", ImVec2(300, 0), true);
		{
			ImGui::PushFont(smallmenu_font);
			{
				const char* chams_type[] = { "metallic", "basic" };
				const char* chams_mode[] = { "none", "visible", "invisible" };
				const char* glow_styles[] = { "regular", "pulsing", "outline" };
				ImGui::Combo(("enemy coloured models"), &SETTINGS::settings.chams_type, chams_mode, ARRAYSIZE(chams_mode));
				ImGui::Combo(("team coloured models"), &SETTINGS::settings.chamsteam, chams_mode, ARRAYSIZE(chams_mode));
				ImGui::Combo(("type chams"), &SETTINGS::settings.chamstype, chams_type, ARRAYSIZE(chams_type));
				const char* local_chams[] = { "none","sim fakelag: normal", "non-sim fakelag", "sim fakelag: color","Rainbow"};
				ImGui::Combo(("local chams"), &SETTINGS::settings.localchams, local_chams, ARRAYSIZE(local_chams));

				ImGui::Checkbox(("Fake chams"), &SETTINGS::settings.fakechams);
				ImGui::Checkbox(("AA Lines"), &SETTINGS::settings.aa_lines);



				ImGui::Checkbox(("enemy glow enable"), &SETTINGS::settings.glowenable);
				ImGui::Checkbox(("team glow enable"), &SETTINGS::settings.glowteam);
				ImGui::Combo(("glow style"), &SETTINGS::settings.glowstyle, glow_styles, ARRAYSIZE(glow_styles));
				ImGui::Checkbox(("local glow"), &SETTINGS::settings.glowlocal);
				ImGui::Combo(("local glow style"), &SETTINGS::settings.glowstylelocal, glow_styles, ARRAYSIZE(glow_styles));

				


			}
			ImGui::PopFont();
		}
		ImGui::EndChild();
	}
	void DrawSkins()
	{
		ImGui::BeginChild(("Skinsxd1"), ImVec2(ImGui::GetWindowWidth() / 3 - 10, 0), true, true ? ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar : ImGuiWindowFlags_NoResize | ImGuiWindowFlags_ShowBorders);
		{
			ImGui::PushFont(smallmenu_font);
			{

				ImGui::Checkbox("skinchanger", &SETTINGS::settings.skinenabled);
				ImGui::Checkbox("glovechanger", &SETTINGS::settings.glovesenabled);
				ImGui::Checkbox("modelchanger", &SETTINGS::settings.CUSTOMMODEL);
				if (SETTINGS::settings.CUSTOMMODEL)
				{
					ImGui::Combo(("ct"), &SETTINGS::settings.customct, playermodels, ARRAYSIZE(playermodels));
					ImGui::Combo(("t"), &SETTINGS::settings.customtt, playermodels, ARRAYSIZE(playermodels));
				}
				if (ImGui::Button(("Force update")))
				{
					KnifeApplyCallbk();

				}


			}
			ImGui::PopFont();

		}ImGui::EndChild();


		ImGui::SameLine();
		ImGui::BeginChild(("Skinsxd2"), ImVec2(400, 0));
		{
			ImGui::PushFont(smallmenu_font);
			{

				static bool Main = true;
				static bool Colors = false;

				static int Page = 0;

				ImGuiStyle& style = ImGui::GetStyle();


				style.ItemSpacing = ImVec2(1, 1);
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.f));

				const char* meme = "page : error";
				switch (Page)
				{
				case 0: meme = "page : 1";  break;
				case 1: meme = "page : 2";  break;
				case 2: meme = "page : 3";  break;
				case 3: meme = "page : 4";  break;
				case 4: meme = "page : 5";  break;
				default: break;
				}

				ImGui::Text(meme); ImGui::SameLine(); ImGui::Text(("                  ")); ImGui::SameLine();
				if (ImGui::Button(("-"), ImVec2(22, 22)))
				{
					if (Page != 0)
						Page--;
				};
				ImGui::SameLine();
				if (ImGui::Button(("+"), ImVec2(22, 22)))
				{
					if (Page != 3)
						Page++;
				};

				ImGui::Text(("        "));

				ImGui::PopStyleColor(); ImGui::PopStyleColor(); ImGui::PopStyleColor();
				style.ItemSpacing = ImVec2(8, 4);
				switch (Page)
				{
				case 0:
				{

					ImGui::PushItemWidth(150.0f);
					ImGui::Combo(("Knife Model"), &SETTINGS::settings.Knife, KnifeModel, ARRAYSIZE(KnifeModel));
					ImGui::PushItemWidth(150.0f);
					ImGui::Combo(("Knife Skin"), &SETTINGS::settings.KnifeSkin, knifeskins, ARRAYSIZE(knifeskins));
					ImGui::PushItemWidth(150.0f);
					ImGui::Combo(("glove model"), &SETTINGS::settings.gloves, GloveModel, ARRAYSIZE(GloveModel));
					ImGui::PushItemWidth(150.0f);
					if (SETTINGS::settings.gloves == 1)
					{

						const char* glovesskins[] =
						{
							"charred",
							"snakebite",
							"bronzed",
							"guerilla"
						};

						ImGui::Combo(("glove skin"), &SETTINGS::settings.skingloves, glovesskins, ARRAYSIZE(glovesskins));
					}
					else
						if (SETTINGS::settings.gloves == 2)
						{
							const char* glovesskins[] =
							{
								"hedge maze",
								"andoras box",
								"superconductor",
								"arid",
								"omega",
								"amphibious",
								"bronze morph"
							};

							ImGui::Combo(("glove skin"), &SETTINGS::settings.skingloves, glovesskins, ARRAYSIZE(glovesskins));
						}
						else
							if (SETTINGS::settings.gloves == 3)
							{
								const char* glovesskins[] =
								{
									"lunar weave",
									"convoy",
									"crimson weave",
									"diamondback",
									"overtake",
									"racing green",
									"king snake",
									"imperial plaid"
								};

								ImGui::Combo(("glove skin"), &SETTINGS::settings.skingloves, glovesskins, ARRAYSIZE(glovesskins));

							}
							else
								if (SETTINGS::settings.gloves == 4)
								{
									const char* glovesskins[] =
									{
										"leather",
										"spruce ddpat",
										"slaughter",
										"cobalt skulls",
										"overprint",
										"duct tape",
										"arboreal"
									};

									ImGui::Combo(("glove skin"), &SETTINGS::settings.skingloves, glovesskins, ARRAYSIZE(glovesskins));

								}
								else
									if (SETTINGS::settings.gloves == 5)
									{
										const char* glovesskins[] =
										{
											"eclipse",
											"spearmint",
											"boom",
											"cool mint",
											"turtle",
											"transport",
											"polygon",
											"pow"
										};

										ImGui::Combo(("glove skin"), &SETTINGS::settings.skingloves, glovesskins, ARRAYSIZE(glovesskins));

									}
									else
										if (SETTINGS::settings.gloves == 6)
										{
											const char* glovesskins[] =
											{
												"forest ddpat",
												"crimson kimono",
												"emerald web",
												"foundation",
												"crimson web",
												"buckshot",
												"fade",
												"mogul"
											};

											ImGui::Combo(("glove skin"), &SETTINGS::settings.skingloves, glovesskins, ARRAYSIZE(glovesskins));

										}
										else
											if (SETTINGS::settings.gloves == 7)
											{
												const char* glovesskins[] =
												{
													"emerald",
													"mangrove",
													"rattler",
													"case hardened"
												};

												ImGui::Combo(("glove skin"), &SETTINGS::settings.skingloves, glovesskins, ARRAYSIZE(glovesskins));

											}
											else
												if (SETTINGS::settings.gloves == 0)
												{
													const char* glovesskins[] = { "" };

													ImGui::Combo(("glove skin"), &SETTINGS::settings.skingloves, glovesskins, ARRAYSIZE(glovesskins));

												}




				}break;
				case 1:
				{
					ImGui::Combo(("AK-47"), &SETTINGS::settings.AK47Skin, ak47, ARRAYSIZE(ak47));
					ImGui::Combo(("M4A1-S"), &SETTINGS::settings.M4A1SSkin, m4a1s, ARRAYSIZE(m4a1s));
					ImGui::Combo(("M4A4"), &SETTINGS::settings.M4A4Skin, m4a4, ARRAYSIZE(m4a4));
					ImGui::Combo(("Galil AR"), &SETTINGS::settings.GalilSkin, galil, ARRAYSIZE(galil));
					ImGui::Combo(("AUG"), &SETTINGS::settings.AUGSkin, aug, ARRAYSIZE(aug));
					ImGui::Combo(("FAMAS"), &SETTINGS::settings.FAMASSkin, famas, ARRAYSIZE(famas));
					ImGui::Combo(("Sg553"), &SETTINGS::settings.Sg553Skin, sg553, ARRAYSIZE(sg553));
					ImGui::Combo(("UMP45"), &SETTINGS::settings.UMP45Skin, ump45, ARRAYSIZE(ump45));
					ImGui::Combo(("MAC-10"), &SETTINGS::settings.Mac10Skin, mac10, ARRAYSIZE(mac10));
					ImGui::Combo(("PP-Bizon"), &SETTINGS::settings.BizonSkin, bizon, ARRAYSIZE(bizon));
					ImGui::Combo(("TEC-9"), &SETTINGS::settings.tec9Skin, tec9, ARRAYSIZE(tec9));
					ImGui::Combo(("P2000"), &SETTINGS::settings.P2000Skin, p2000, ARRAYSIZE(p2000));
					ImGui::Combo(("P250"), &SETTINGS::settings.P250Skin, p250, ARRAYSIZE(p250));
					ImGui::Combo(("Dual-Barettas"), &SETTINGS::settings.DualSkin, dual, ARRAYSIZE(dual));
					ImGui::Combo(("Cz75-Auto"), &SETTINGS::settings.Cz75Skin, cz75, ARRAYSIZE(cz75));
					ImGui::Combo(("Nova"), &SETTINGS::settings.NovaSkin, nova, ARRAYSIZE(nova));


				}break;
				case 2:
				{
					ImGui::Combo(("XM1014"), &SETTINGS::settings.XmSkin, xm, ARRAYSIZE(xm));
					ImGui::Combo(("AWP"), &SETTINGS::settings.AWPSkin, awp, ARRAYSIZE(awp));
					ImGui::Combo(("SSG08"), &SETTINGS::settings.SSG08Skin, ssg08, ARRAYSIZE(ssg08));
					ImGui::Combo(("SCAR20"), &SETTINGS::settings.SCAR20Skin, scar20, ARRAYSIZE(scar20));
					ImGui::Combo(("G3SG1"), &SETTINGS::settings.G3sg1Skin, g3sg1, ARRAYSIZE(g3sg1));
					ImGui::Combo(("MP9"), &SETTINGS::settings.Mp9Skin, mp9, ARRAYSIZE(mp9));
					ImGui::Combo(("Glock-18"), &SETTINGS::settings.GlockSkin, glock, ARRAYSIZE(glock));
					ImGui::Combo(("USP-S"), &SETTINGS::settings.USPSkin, usp, ARRAYSIZE(usp));
					ImGui::Combo(("Deagle"), &SETTINGS::settings.DeagleSkin, deagle, ARRAYSIZE(deagle));
					ImGui::Combo(("Five-Seven"), &SETTINGS::settings.FiveSkin, five, ARRAYSIZE(five));
					ImGui::Combo(("Revolver"), &SETTINGS::settings.RevolverSkin, revolver, ARRAYSIZE(revolver));
					ImGui::Combo(("Negev"), &SETTINGS::settings.NegevSkin, negev, ARRAYSIZE(negev));
					ImGui::Combo(("M249"), &SETTINGS::settings.M249Skin, m249, ARRAYSIZE(m249));
					ImGui::Combo(("Sawed-Off"), &SETTINGS::settings.SawedSkin, sawed, ARRAYSIZE(sawed));
					ImGui::Combo(("Mag-7"), &SETTINGS::settings.MagSkin, mag, ARRAYSIZE(mag));
				}break;
				case 3:
				{
					short cw = SafeWeaponID();
						ImGui::InputText(("yourshit"), SETTINGS::settings.weapons[cw].ChangerName, 128);
					if (cw == 59 || cw == 500 || cw == 42 || cw == 507 || cw == 506 || cw == 508 || cw == 509 || cw == 515 || cw == 516 || cw == 505 || cw == 512 || cw == 523 || cw == 519 || cw == 521 || cw == 520 || cw == 522)
					{

					}
					else
					{
						ImGui::InputInt(("Stikers1"), &SETTINGS::settings.weapons[cw].Stikers1);
						ImGui::InputInt(("Stikers2"), &SETTINGS::settings.weapons[cw].Stikers2);
						ImGui::InputInt(("Stikers3"), &SETTINGS::settings.weapons[cw].Stikers3);
						ImGui::InputInt(("Stikers4"), &SETTINGS::settings.weapons[cw].Stikers4);
					}


				}break;

				default:break;
				}


			}
			ImGui::PopFont();

		}
		ImGui::EndChild();


	}
	void DrawCfg()
	{
		ImGui::BeginChild("sa3rqewfadfwe", ImVec2(800, 0), true);
		{
			ImGui::PushFont(smallmenu_font);
			{
				const char* fakelag_mode[] = { "factor", "adaptive" };
				ImGui::Checkbox(("Enable Misc"), &SETTINGS::settings.misc_bool);
				ImGui::Checkbox(("Auto Bunnyhop"), &SETTINGS::settings.bhop_bool);
				ImGui::Checkbox(("Auto Strafer"), &SETTINGS::settings.strafe_bool);
				ImGui::Checkbox(("Duck in air"), &SETTINGS::settings.duck_bool);
				ImGui::Checkbox(("trashtalk"), &SETTINGS::settings.achievement_earned);
				/*ImGui::Checkbox("preserve killfeed", &SETTINGS::settings.killfeedon);
				if (SETTINGS::settings.killfeedon)
					SETTINGS::settings.PreserveKillfeed = 90;
				else
					SETTINGS::settings.PreserveKillfeed = 0;*/
				ImGui::Checkbox(("trash if hs"), &SETTINGS::settings.trashifhs);
				ImGui::Checkbox(("trash sounds"), &SETTINGS::settings.supermeme);
				const char* flexmode[] = { "none", "fast flex", "low flex" };
				ImGui::Combo(("flex type"), &SETTINGS::settings.flexmode, flexmode, ARRAYSIZE(flexmode));
				ImGui::Checkbox(("glitch fakewalk"), &SETTINGS::settings.glitch_bool);
				ImGui::Checkbox("meme walk", &SETTINGS::settings.astro);
				ImGui::Checkbox("fake duck", &SETTINGS::settings.fakeduck);
				if (SETTINGS::settings.fakeduck)
				{
					ImGui::Combo(("fake duck key"), &SETTINGS::settings.fakeducsk, key_binds, ARRAYSIZE(key_binds));
				}
				ImGui::Checkbox("fast duck", &SETTINGS::settings.fastduck);
			/*	ImGui::Checkbox("slow walk", &SETTINGS::settings.jopakamaza);
				if (SETTINGS::settings.jopakamaza) 
				{
					ImGui::SliderFloat("slow auto", &SETTINGS::settings.slow_scar,0,100, "%.0f");
					ImGui::SliderFloat("sow ssg", &SETTINGS::settings.slow_ssg,0,100, "%.0f");
					ImGui::SliderFloat("slow awp", &SETTINGS::settings.slow_awp,0,100, "%.0f");
					ImGui::SliderFloat("slow any", &SETTINGS::settings.slow_any, 0,100, "%.0f");
					ImGui::Spacing();
					ImGui::Checkbox(("smart slow"), &SETTINGS::settings.smart_slow);
				}*/
				ImGui::Checkbox("SlowWalk", &SETTINGS::settings.jopakamaza);
				if (SETTINGS::settings.jopakamaza)
					ImGui::SliderFloat("Speed", &SETTINGS::settings.slow_scar, 0, 150);
				ImGui::Checkbox(("basic fakewalk"), &SETTINGS::settings.fakewalk);
				if (SETTINGS::settings.fakewalk)
				{
					ImGui::SliderFloat("fakewalk speed", &SETTINGS::settings.fakewalkspeed, 3, 8, "%.0f");
				}
				ImGui::Checkbox("Clantag", &SETTINGS::settings.clangtas);
				if (SETTINGS::settings.clangtas) {
					ImGui::Checkbox("phack", &SETTINGS::settings.clangtasp);
					ImGui::Checkbox("Fatality.win", &SETTINGS::settings.clangtasf);
					ImGui::Checkbox("skeet", &SETTINGS::settings.clangtass);
					ImGui::Checkbox("moneybot", &SETTINGS::settings.clangtaspp);
					ImGui::Checkbox("phack(+)", &SETTINGS::settings.clangtaspp2);
				}
				ImGui::Combo(("circle strafe"), &SETTINGS::settings.circlestrafekey, key_binds, ARRAYSIZE(key_binds));
				ImGui::Checkbox("buybot", &SETTINGS::settings.buybot_enabled);
				const char* pistol[] = { "disabled", "dealge | revolver", "elites", "p250" };
				const char* snipers[] = { "disabled", "scar20 | g3sg1", "ssg08", "awp" };
				const char* armor[] = { "disabled", "kevlar", "Helmet & kevlar" };
				if (SETTINGS::settings.buybot_enabled)
				{
					{
						ImGui::Combo(("Pistols"), &SETTINGS::settings.buybot_pistol, pistol, ARRAYSIZE(pistol));
						ImGui::Combo(("Snipers"), &SETTINGS::settings.buybot_rifle, snipers, ARRAYSIZE(snipers));
						ImGui::Combo(("Armor"), &SETTINGS::settings.buybot_armor, armor, ARRAYSIZE(armor));
						ImGui::Checkbox("Zeus", &SETTINGS::settings.buybot_zeus);
						ImGui::Checkbox("Grenades", &SETTINGS::settings.buybot_grenade);
					}
				}
				ImGui::Checkbox(("Enable FakeLag"), &SETTINGS::settings.lag_bool);
				ImGui::Combo(("Fakelag Type"), &SETTINGS::settings.lag_type, fakelag_mode, ARRAYSIZE(fakelag_mode));
				//ImGui::Combo(("disable fakelag"), &SETTINGS::settings.disable_lag, key_binds, ARRAYSIZE(key_binds));
				ImGui::Checkbox("Smart fakelag", &SETTINGS::settings.smart_lag);
				ImGui::SliderFloat("standing lag", &SETTINGS::settings.stand_lag, 1, 100, "%.0f");
				ImGui::SliderFloat("moving lag", &SETTINGS::settings.move_lag, 1, 100, "%.0f");
				ImGui::SliderFloat("jumping lag", &SETTINGS::settings.jump_lag, 1, 100, "%.0f");
				ImGui::SliderFloat("fakewalk lag", &SETTINGS::settings.fakwal, 1, 100, "%.0f");
			}
			ImGui::PopFont();
		}
		ImGui::EndChild();

	}
	void DrawColor()
	{
		ImGui::BeginChild("qwerq23rew", ImVec2(400, 0), true);
		{
			ImGui::PushFont(smallmenu_font);
			{
				ImGui::InputText("##CFG", ConfigNamexd, 64);
				static int sel;
				std::string config;
				std::vector<std::string> configs = SETTINGS::settings.GetConfigs();
				if (configs.size() > 0) {
					ImGui::ComboBoxArrayxd("Configs", &sel, configs);
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
					ImGui::PushItemWidth(220.f);
					config = configs[SETTINGS::settings.config_sel];
				}
				SETTINGS::settings.config_sel = sel;

				if (configs.size() > 0) {
					if (ImGui::Button("Load", ImVec2(50, 20)))
					{

						SETTINGS::settings.Load(config);
						GLOBAL::Msg("[PHACK] Configuration loaded.    \n");
					}
				}
				ImGui::SameLine();

				if (configs.size() >= 1) {
					if (ImGui::Button("Save", ImVec2(50, 20)))
					{
						SETTINGS::settings.Save(config);
						GLOBAL::Msg("[PHACK] Configuration saved.    \n");
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Create", ImVec2(50, 20)))
				{
					std::string ConfigFileName = ConfigNamexd;
					if (ConfigFileName.size() < 1)
					{
						ConfigFileName = "settings";
					}
					SETTINGS::settings.CreateConfig(ConfigFileName);
					GLOBAL::Msg("[PHACK] Configuration created.    \n");
				}
				ImGui::SameLine();
				if (config.size() >= 1) {
					if (ImGui::Button("Delete", ImVec2(50, 20)))
					{
						SETTINGS::settings.Remove(config);
						GLOBAL::Msg("[PHACK] Configuration removed.    \n");
					}
				}
				ImGui::Combo(("menu key"), &SETTINGS::settings.menu, key_binds2, ARRAYSIZE(key_binds2));
				static int iMenuSheme = 1;

				const char* ThemesList[] = { "Purple" , "Default" , "Light Pink" , "Dark Blue" , "MidNight" , "Night" , "Dunno" , "Blue"  , "Black" , "Green" , "Yellow" , "Light Blue" , "Light Grey" , "pHooK" };

			ImGui::PushItemWidth(362.f);
				ImGui::Combo("Menu Color", &iMenuSheme, ThemesList, IM_ARRAYSIZE(ThemesList));

				ImGui::Separator();

				if (ImGui::Button("Apply Color"))
				{
					if (iMenuSheme == 0)
						purple();
					else if (iMenuSheme == 1)
						DefaultSheme1();
					else if (iMenuSheme == 2)
						RedSheme();
					else if (iMenuSheme == 3)
						darkblue();
					else if (iMenuSheme == 4)
						MidNightSheme();
					else if (iMenuSheme == 5)
						NightSheme();
					else if (iMenuSheme == 6)
						DunnoSheme();
					else if (iMenuSheme == 7)
						BlueSheme();
					else if (iMenuSheme == 8)
						BlackSheme2();
					else if (iMenuSheme == 9)
						green();
					else if (iMenuSheme == 10)
						pink();
					else if (iMenuSheme == 11)
						blue();
					else if (iMenuSheme == 12)
						yellow();
					else if (iMenuSheme == 13)
						BlackSheme();
				}
			}
			ImGui::PopFont();
			ImGui::PushFont(smallmenu_font);
			ImGui::Spacing();
			ImGui::Text(("Box Colors"));
			ImGui::ColorEdit4(("##box"), SETTINGS::settings.boxer_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);

			ImGui::Text(("Chams"));
			ImGui::ColorEdit4(("Enemy visible##chams"), SETTINGS::settings.vmodel_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Enemy invisible##chams"), SETTINGS::settings.imodel_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Team visible##chams"), SETTINGS::settings.teamvis_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Team invisible##chams"), SETTINGS::settings.teaminvis_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Local##chams"), SETTINGS::settings.localchams_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);


			ImGui::Text(("Glows"));
			ImGui::ColorEdit4(("Enemy##glow"), SETTINGS::settings.glow_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Team##glow"), SETTINGS::settings.teamglow_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Local##glow"), SETTINGS::settings.glowlocal_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::Text(("Bullet Tracers"));


			ImGui::ColorEdit4(("Local##tracer"), SETTINGS::settings.bulletlocal_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Enemy##tracer"), SETTINGS::settings.bulletenemy_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Team##tracer"), SETTINGS::settings.bulletteam_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);







			ImGui::Text(("Other"));
			ImGui::ColorEdit4(("Grenade prediction##xd"), SETTINGS::settings.grenadepredline_col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Spread circle##xd"), SETTINGS::settings.spreadcirclecol, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			ImGui::ColorEdit4(("Fake chams##xd"), SETTINGS::settings.fakechamscol, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			if (SETTINGS::settings.rifk_arrow)
			{
				ImGui::ColorEdit4("AA Fake", SETTINGS::settings.aa_fake2, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
				ImGui::ColorEdit4("AA Real", SETTINGS::settings.aa_fake3, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar);
			}




			ImGui::PopFont();
		}
		ImGui::EndChild();
		ImGui::SameLine();

	}
	void ButtonColor(int r, int g, int b)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Button] = ImColor(r, g, b);
		style.Colors[ImGuiCol_ButtonHovered] = ImColor(r, g, b);
		style.Colors[ImGuiCol_ButtonActive] = ImColor(r, g, b);
	}
	ImVec2 pos;
	void GUI_Init(IDirect3DDevice9* pDevice)
	{
		static int hue = 140;
		ImGui_ImplDX9_Init(INIT::Window, pDevice);

		ImFont* cheat_font;
		ImFont* title_font;
		ImGuiIO& io = ImGui::GetIO();
		//io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Untitled1.ttf", 15.0f);


		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImColor(15, 15, 15, 255);
		style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
		style.Colors[ImGuiCol_Border] = ImColor(15, 15, 15, 255);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15f, 0.60f, 0.78f, 0.78f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.70f, 0.02f, 0.60f, 0.22f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);

	//	pDevice->GetViewport(&Menuxd::viewPort);
	//	D3DXCreateFont(pDevice, 9, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DRAFT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, ("Verdana"), &Menuxd::fntVerdana9);
	//	D3DXCreateFont(pDevice, 10, 5, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, ("Verdana"), &Menuxd::fntVerdana10);
	//	D3DXCreateFont(pDevice, 11, 5, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, ("Verdana"), &Menuxd::fntVerdana11);
		//D3DXCreateFont(pDevice, 12, 5, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, ("Verdana"), &Menuxd::fntVerdana12);
	//	D3DXCreateFont(pDevice, 11, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, ("csgo_icons"), &Menuxd::fntWeaponIcon);
	//	font_menu = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(gamer, 34952, 24.f);
		style.WindowRounding = 0.f;
		style.FramePadding = ImVec2(4, 0);
		style.WindowPadding = ImVec2(0, 0);
		style.ItemSpacing = ImVec2(0, 0);
		style.ScrollbarSize = 10.f;
		style.ScrollbarRounding = 0.f;
		style.GrabMinSize = 5.f;
		
		menu_fon = io.Fonts->AddFontFromFileTTF("C:\\phack\\Untitled1.ttf", 18);
		menu_font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(rawData_compressed_data_base85, 18);
		smallmenu_font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(smalll_compressed_data_base85, 13);
		d3d_init = true;
	}
	long __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
	{
		static auto ofunc = directz.get_original<EndSceneFn>(42);

		D3DCOLOR rectColor = D3DCOLOR_XRGB(255, 0, 0);
		D3DRECT BarRect = { 1, 1, 1, 1 };
		ImGuiStyle& style = ImGui::GetStyle();
		pDevice->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, rectColor, 0, 0);

		if (!d3d_init)
			GUI_Init(pDevice);
		//ImGui::GetIO().MouseDrawCursor = menu_open;

		static const D3DRENDERSTATETYPE backupStates[] = { D3DRS_COLORWRITEENABLE, D3DRS_ALPHABLENDENABLE, D3DRS_SRCBLEND, D3DRS_DESTBLEND, D3DRS_BLENDOP, D3DRS_FOGENABLE };
		static const int size = sizeof(backupStates) / sizeof(DWORD);

		DWORD oldStates[size] = { 0 };

		for (int i = 0; i < size; i++)
			pDevice->GetRenderState(backupStates[i], &oldStates[i]);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);

		ImGui_ImplDX9_NewFrame();
		if (INTERFACES::Engine->IsConnected()) DrawRadar();




		//if (tImage == nullptr)D3DXCreateTextureFromFileInMemoryEx(pDevice, &NameArry, sizeof(NameArry), 564, 845, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tImage);

		if (menu_open)
		{

			POINT mp;

			GetCursorPos(&mp);
			ImGuiIO& io = ImGui::GetIO();
			io.MousePos.x = mp.x;
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Appearing);
			style.WindowPadding = ImVec2(8, 8);
			ImGui::SetNextWindowSize(ImVec2(860, 840), ImGuiSetCond_FirstUseEver);
			io.MousePos.y = mp.y;

			//ImGui::Begin("##Stackhackrecode", &menu_open, ImVec2(1300, 600), 1, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
			ImGui::Begin("PHACK", &menu_open, ImVec2(840, 560), 1.f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_ShowBorders| ImGuiWindowFlags_NoScrollbar);
			{

				pos = ImGui::GetWindowPos();

				ImGui::Columns(2, nullptr, false);
				ImGui::SetColumnOffset(1, 135);

				ButtonColor(15, 15, 15);
				ImGui::Button("##upprtabs", ImVec2(114, 20));

				ImGui::SameLine();
				ButtonColor(50, 50, 50);
				ImGui::Button("##rageupline", ImVec2(1, 20));
				ImGui::PushFont(menu_fon);
				/*rage*/
				{
					if (iTab == 0) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##rageupline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));

					ButtonColor(15, 15, 15);
					if (iTab == 0) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
					if (ImGui::Button("a", ImVec2(118, 100))) iTab = 0;

					ImGui::SameLine();

					if (iTab != 0)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

					if (iTab == 0) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##ragedownline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));

				}

				/*legit*/
				{
					if (iTab == 1) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##lupline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));

					ButtonColor(15, 15, 15);
					if (iTab == 1) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
					if (ImGui::Button("c", ImVec2(118, 100))) iTab = 1;

					ImGui::SameLine();

					if (iTab != 1)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

					if (iTab == 1) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##ldownline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));
				}

				/*visuals*/
				{
					if (iTab == 2) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##vupline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));

					ButtonColor(15, 15, 15);
					if (iTab == 2) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
					if (ImGui::Button("e", ImVec2(118, 100))) iTab = 2;
					ImGui::SameLine();

					if (iTab != 2)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

					if (iTab == 2) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##vdownline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));
				}

				/*misc*/
				{
					if (iTab == 3) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##mupline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));

					ButtonColor(15, 15, 15);
					if (iTab == 3) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
					if (ImGui::Button("d", ImVec2(118, 100))) iTab = 3;
					ImGui::SameLine();

					if (iTab != 3)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

					if (iTab == 3) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##mdownline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));
				}

				/*skins*/
				{
					if (iTab == 4) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##supline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));

					ButtonColor(15, 15, 15);
					if (iTab == 4) style.Colors[ImGuiCol_Text] = ImColor(200, 200, 200); else style.Colors[ImGuiCol_Text] = ImColor(80, 80, 80);
					if (ImGui::Button("f", ImVec2(118, 100))) iTab = 4;
					ImGui::SameLine();

					if (iTab != 4)ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##gfgfgfgfgfgf", ImVec2(1, 100));

					if (iTab == 4) ButtonColor(50, 50, 50); else ButtonColor(15, 15, 15);
					ImGui::Button("##sdownline", ImVec2(118, 1));

					ImGui::SameLine();

					ButtonColor(50, 50, 50);
					ImGui::Button("##fgfgfg", ImVec2(1, 1));
				}
				ImGui::PopFont();
				ButtonColor(15, 15, 15);
				ImGui::Button("##upprtabs", ImVec2(118, 20));

				ImGui::SameLine();

				ButtonColor(50, 50, 50);
				ImGui::Button("##rageupline", ImVec2(1, 20));

				ImGui::NextColumn();
				style.WindowPadding = ImVec2(8, 8);
				style.ItemSpacing = ImVec2(4, 4);
			/*	style.Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
				style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
				style.Colors[ImGuiCol_WindowBg] = ImColor(15, 15, 15, 255);
				style.Colors[ImGuiCol_ChildWindowBg] = ImColor(15, 15, 15, 255);
				style.Colors[ImGuiCol_Border] = ImColor(15, 15, 15, 255);
				style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
				style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
				style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
				style.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
				style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
				style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
				style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
				style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
				style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
				style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15f, 0.60f, 0.78f, 0.78f);
				style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
				style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.15f, 0.60f, 0.78f, 1.00f);
				style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
				style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
				style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
				style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
				style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
				style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
				style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.70f, 0.02f, 0.60f, 0.22f);*/
				ImGui::BeginChild("main", ImVec2(700, 540));
				{
					if (iTab == 0)
					{
						DrawAim();
					}
					if (iTab == 1)
					{
						DrawVis();
					}
					if (iTab == 2)
					{
						DrawSkins();
					}
					if (iTab == 3)
					{
						DrawCfg();
					}
					if (iTab == 4)
					{
						DrawColor();
					}
					//do tabs here
				}ImGui::EndChild();

			}
			ImGui::End();

			style.Colors[ImGuiCol_WindowBg] = ImColor(30, 30, 30, 255);
			ImGui::SetNextWindowPos(ImVec2(pos.x - 4, pos.y - 4));
			/*ImGui::Begin("borderr", NULL, ImVec2(848, 568), 1.f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
			{

			}ImGui::End();*/
		}

		/*ImGui::SetNextWindowSize(ImVec2(543.3, 543.3));
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.0f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, SETTINGS::settings.sanadrawborder);
		ImGui::Begin("##PharxdamcyTabs", &_xd, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
		{
		if (SETTINGS::settings.sanaenabled) ImGui::Image(tImage, ImVec2(SETTINGS::settings.picturesw, SETTINGS::settings.picturesh));
		}
		ImGui::End();*/






		ImGui::Render();


		//if (SETTINGS::settings.spread_bool == 3)
		//	Menuxd::drawfatalpricel(pDevice);
		

		//if (SETTINGS::settings.Visuals.Enabled)
	//	if (SETTINGS::settings.Visuals.Enabled)
		//	D9Visuals::Render(pDevice);
	//	if (SETTINGS::settings.spread_bool == 3)
	//		Menuxd::drawfatalpricel(pDevice);

		/*if (INTERFACES::Engine->IsConnected())
		{

			//	D9Visuals::Render(pDevice);
			//	if (SETTINGS::settings.Visuals.Visuals_HealthBar) D9Visuals::DrawHealth(entity, pDevice);
				//DamageESP::Draw();
			//	if (SETTINGS::settings.zeusrange) Menuxd::DrawZeusRange(entity, pDevice);
				Menuxd::DrawFilledRectangle(Menuxd::viewPort.Width - 205, 3, 17, 105, D3DCOLOR_ARGB(230, 15, 15, 15), pDevice);
				Menuxd::DrawStringWithFont(Menuxd::fntVerdana11, Menuxd::viewPort.Width - 200, 5, D3D_COLOR_BLACK(240), ("     PHACK BETA"));
				Menuxd::DrawStringWithFont(Menuxd::fntVerdana10, Menuxd::viewPort.Width - 201, 6, D3D_COLOR_WHITE(240), ("     PHACK BETA"));
				//visuals->watermark();
		}
		*/

		for (int i = 0; i < size; i++)
			pDevice->SetRenderState(backupStates[i], oldStates[i]);

		return ofunc(pDevice);
	}

	long __stdcall Hooked_EndScene_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto ofunc = directz.get_original<EndSceneResetFn>(16);

		if (!d3d_init)
			return ofunc(pDevice, pPresentationParameters);

		ImGui_ImplDX9_InvalidateDeviceObjects();

		auto hr = ofunc(pDevice, pPresentationParameters);

		ImGui_ImplDX9_CreateDeviceObjects();

		return hr;
	}


	typedef bool(__thiscall *FireEventClientSide)(void*, SDK::IGameEvent*);

	bool __fastcall Hooked_FireEventClientSide(void *ecx, void* edx, SDK::IGameEvent* pEvent) {

		static auto ofunct = fireevent.get_original<FireEventClientSide>(9);

		if (!pEvent)
			return ofunct(ecx, pEvent);

		//DamageESP::HandleGameEvent(pEvent);

	
		return ofunct(ecx, pEvent);
	}




	void InitHooks()
	{
		iclient_hook_manager.Init(INTERFACES::Client);
		original_frame_stage_notify = reinterpret_cast<FrameStageNotifyFn>(iclient_hook_manager.HookFunction<FrameStageNotifyFn>(37, HookedFrameStageNotify));

		panel_hook_manager.Init(INTERFACES::Panel);
		original_paint_traverse = reinterpret_cast<PaintTraverseFn>(panel_hook_manager.HookFunction<PaintTraverseFn>(41, HookedPaintTraverse));

		model_render_hook_manager.Init(INTERFACES::ModelRender);
		original_draw_model_execute = reinterpret_cast<DrawModelExecuteFn>(model_render_hook_manager.HookFunction<DrawModelExecuteFn>(21, HookedDrawModelExecute));

		scene_end_hook_manager.Init(INTERFACES::RenderView);
		original_scene_end = reinterpret_cast<SceneEndFn>(scene_end_hook_manager.HookFunction<SceneEndFn>(9, HookedSceneEnd));

		trace_hook_manager.Init(INTERFACES::Trace);
		original_trace_ray = reinterpret_cast<TraceRayFn>(trace_hook_manager.HookFunction<TraceRayFn>(5, HookedTraceRay));

		override_view_hook_manager.Init(INTERFACES::ClientMode);
		original_override_view = reinterpret_cast<OverrideViewFn>(override_view_hook_manager.HookFunction<OverrideViewFn>(18, HookedOverrideView));
		original_create_move = reinterpret_cast<CreateMoveFn>(override_view_hook_manager.HookFunction<CreateMoveFn>(24, HookedCreateMove));
		original_viewmodel_fov = reinterpret_cast<GetViewmodelFOVFn>(override_view_hook_manager.HookFunction<GetViewmodelFOVFn>(35, GetViewmodelFOV));

		auto sv_cheats = INTERFACES::cvar->FindVar("sv_cheats");
		get_bool_manager = VMT::VMTHookManager(reinterpret_cast<DWORD**>(sv_cheats));
		original_get_bool = reinterpret_cast<SvCheatsGetBoolFn>(get_bool_manager.HookFunction<SvCheatsGetBoolFn>(13, HookedGetBool));

		if (SETTINGS::settings.fakelatency_enabled && INTERFACES::Engine->IsInGame() && INTERFACES::Engine->IsConnected() && (*INTERFACES::client_state)->m_NetChannel)
		{
			net_channel_hook_manager.Init((DWORD**)(*INTERFACES::client_state)->m_NetChannel);
			original_send_datagram = reinterpret_cast<SendDatagramFn>(net_channel_hook_manager.HookFunction<SendDatagramFn>(46, hkSendDatagram));
		}

		fireevent.setup(INTERFACES::GameEventManager);
		fireevent.hook_index(9, Hooked_FireEventClientSide);


		while (!(INIT::Window = FindWindowA("Valve001", nullptr)))
			Sleep(100);
		if (INIT::Window)
			INIT::OldWindow = (WNDPROC)SetWindowLongPtr(INIT::Window, GWL_WNDPROC, (LONG_PTR)Hooked_WndProc);
		DWORD DeviceStructureAddress = **(DWORD**)(UTILS::FindSignature("shaderapidx9.dll", "A1 ?? ?? ?? ?? 50 8B 08 FF 51 0C") + 1);
		if (DeviceStructureAddress) {
			directz.setup((DWORD**)DeviceStructureAddress);
			directz.hook_index(16, Hooked_EndScene_Reset);
			directz.hook_index(42, Hooked_EndScene);

		}
	}
	void EyeAnglesPitchHook(const SDK::CRecvProxyData *pData, void *pStruct, void *pOut)
	{
		*reinterpret_cast<float*>(pOut) = pData->m_Value.m_Float;

		auto entity = reinterpret_cast<SDK::CBaseEntity*>(pStruct);
		if (!entity)
			return;

	}
	void EyeAnglesYawHook(const SDK::CRecvProxyData *pData, void *pStruct, void *pOut)
	{
		*reinterpret_cast<float*>(pOut) = pData->m_Value.m_Float;

		auto entity = reinterpret_cast<SDK::CBaseEntity*>(pStruct);
		if (!entity)
			return;
	}
	void InitNetvarHooks()
	{
		UTILS::netvar_hook_manager.Hook("DT_CSPlayer", "m_angEyeAngles[0]", EyeAnglesPitchHook);
		UTILS::netvar_hook_manager.Hook("DT_CSPlayer", "m_angEyeAngles[1]", EyeAnglesYawHook);
	}
}