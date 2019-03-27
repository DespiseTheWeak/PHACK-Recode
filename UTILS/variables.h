#pragma once

/*
global variables and other stuff that is probably too small to put in another 
file (and i'm too lazy to make files for all of them) would go in here.
Stuff like fonts and shit
*/

namespace GLOBAL
{
	extern HWND csgo_hwnd;

	extern bool should_send_packet;
	extern bool shoot;
	extern bool is_fakewalking;
	extern int choke_amount;

	extern Vector real_angles;
	extern Vector angles;
	extern Vector fake_angles;
	extern Vector strafe_angle;

	extern int randomnumber;
	extern float flHurtTime;
	extern bool DisableAA;
	extern bool Aimbotting;

	using msg_t = void(__cdecl*)(const char*, ...);
	extern msg_t		Msg;

	extern Vector FakePosition;
	extern int ground_tickz;
	extern bool CircleStraferActive;
	extern SDK::CUserCmd originalCMD;
	extern bool NewRound;
	extern int MenuTab;
}
namespace FONTS
{
	extern unsigned int menu_tab_font;
	extern unsigned int menu_checkbox_font;
	extern unsigned int menu_slider_font;
	extern unsigned int menu_groupbox_font;
	extern unsigned int menu_combobox_font;
	extern unsigned int menu_window_font;
	extern unsigned int numpad_menu_font;
	extern unsigned int visuals_esp_font;
	extern unsigned int visuals_xhair_font;
	extern unsigned int visuals_side_font;
	extern unsigned int visuals_name_font;
	extern unsigned int visuals_lby_font;
	extern unsigned int visuals_grenade_pred_font;
	extern 	unsigned int chlen;
	extern 	unsigned int chlen_if_hs;
	extern unsigned int  in_game_logging_font;
	bool ShouldReloadFonts();
	void InitFonts();
}
namespace SETTINGS
{
	class Sticker_t
	{
	public:
		int   iID = 0;
		float flWear = 0.f;
		float flScale = 1.f;
		int   iRotation = 0;
	};
	class Weapon_tTT
	{
	public:
		int SkinsWeapon;
		int SkinsKnife;
		int VremennyiWeapon;
		int VremennyiKnife;
		int Stikers1;
		int Stikers2;
		int Stikers3;
		int Stikers4;
		float ChangerWear = 0;
		int ChangerStatTrak = 0;
		char ChangerName[32] = "";
		bool ChangerEnabled;
	};

	class CSettings
	{
	public:
		// returns true/false whether the function succeeds, usually returns false if file doesn't exist
		bool Save(std::string file_name);
		bool Load(std::string file_name);

		bool Remove(std::string file_name);

		void CreateConfig(std::string name); // creates a blank config

		std::vector<std::string> GetConfigs();

		bool skinenabled;
		int Knife;

		Weapon_tTT weapons[520];

		struct wpnz
		{
			bool  EnabledStickers;
			bool  EnabledSkins;
			int   ChangerSkin;
			char  ChangerName[32] = "";
			int   ChangerStatTrak;
			int   ChangerSeed;
			float ChangerWear;
			Sticker_t Stickers[5];
		}Weapons[519];

		bool fix;
		bool backaimhead;
		int rage_lagcompensation_type=0;
		bool rage_lagcompensation;

		bool CUSTOMMODEL;
		int customct;
		int customtt;

		int gloves;
		int skingloves;
		float glovewear;
		bool glovesenabled;
		bool rankchanger;
		int KnifeSkin;
		int rank_id = 9;
		int wins = 100;
		int level = 40;
		int friendly = 999;
		int teaching = 999;
		int leader = 999;
		int AK47Skin;
		int GalilSkin;
		int M4A1SSkin;
		int M4A4Skin;
		int AUGSkin;
		int FAMASSkin;
		int AWPSkin;
		int SSG08Skin;
		bool fakelatency_enabled;
		bool fixshit;
		float fakelatency_amount;
		int SCAR20Skin;
		int P90Skin;
		int Mp7Skin;
		int NovaSkin;
		int UMP45Skin;
		int GlockSkin;
		int SawedSkin;
		int USPSkin;
		int MagSkin;
		int XmSkin;
		int DeagleSkin;
		int DualSkin;
		int FiveSkin;
		int RevolverSkin;
		int Mac10Skin;
		int tec9Skin;
		int Cz75Skin;
		int NegevSkin;
		int M249Skin;
		int Mp9Skin;
		int P2000Skin;
		int BizonSkin;
		int Sg553Skin;
		int P250Skin;
		int G3sg1Skin;
		int res_type;
		bool info_bool2;


		bool friendfire;
		bool bhop_bool;
		bool strafe_bool;
		bool esp_bool;
		int chams_type;
		int xhair_type;
		bool tp_bool;
		bool aim_bool;
		int aim_type;
		bool aa_bool;
		int aa_pitch;
		int aa_type;
		int acc_type;
		bool up_bool;
		bool misc_bool;
		int config_sel;
		bool beam_bool;
		bool stop_bool;
		bool night_bool;
		bool box_bool;
		bool name_bool;
		bool weap_bool;
		bool health_bool;
		bool info_bool;
		bool back_bool;
		bool lag_bool;
		int box_type;
		bool reverse_bool;
		bool multi_bool;
		bool fakefix_bool;
		bool angle_bool;
		bool tp_angle_bool;
		bool glow_bool;
		bool dist_bool;
		bool fov_bool;
		bool smoke_bool;
		bool scope_bool;
		bool predict_bool;
		bool fake_bool;
		int media_type;
		bool novis_bool;
		bool localglow_bool;
		bool duck_bool;
		bool money_bool;
		int delay_shot;
		int lag_type;
		bool cham_bool;
		bool resolve_bool;
		bool ammo_bool;
		int spread_bool;


		float spreadcirclecol[4] = { 1.0f,0.0f,0.0f,0.3f };

		float stand_lag;
		float move_lag;
		float jump_lag;

		bool debug_bool;


		float vmodel_col[4] = { 1.0f,1.0f,0.0f,0.5f };

		float imodel_col[4] = { 0.0f,1.0f,0.0f,0.5f };
		
	
		
		CColor box_col;
		CColor name_col;
		CColor weapon_col;
		CColor distance_col;

		
		float localchams_col[4] = { 1.0f,0.0f,1.0f,0.5f };
		float grenadepredline_col[4] = { 1.0f,0.0f,1.0f,0.5f };

		float bulletlocal_col[4] = { 1.0f,0.0f,1.0f,1.0f };
		float bulletenemy_col[4] = { 0.0f,1.0f,1.0f,1.0f };
		float bulletteam_col[4]{ 1.0f,1.0f,1.0f,1.0f };

		float bulletlife = 3.0f;
		float bulletsize = 2.0f;
	
		CColor menu_col = CColor(60, 60, 60);
		CColor checkbox_col = CColor(5, 135, 5);
		CColor slider_col = CColor(5, 135, 5);
		CColor tab_col = CColor(91, 91, 91);
		
		float glow_col[4] = { 0.5f,0.5f,1.0f,1.0f };

		float glowlocal_col[4] = { 0.3f,0.3f,0.9f,0.7f };
		
		CColor fov_col;

		float chance_val;
		float damage_val;
		float delta_val;
		float point_val;
		float body_val;
		bool misc_clantag;

		bool localesp;
		int localchams;
		float fov_val = 90;
		float viewfov_val = 68;

		bool fakechams;
		float fakechamscol[4] = { 1.0f,1.0f,1.0f,1.0f };


		int flip_bool;
		int aa_side;

		bool legit_bool;
		int legit_key;
		bool rcs_bool;
		float legitfov_val;
		int legitbone_int;
		float rcsamount_min;
		float rcsamount_max;
		float legitaim_val;
		bool legittrigger_bool;
		int legittrigger_key;

		int thirdperson_int;
		int flip_int;

		bool glowenable;
		int glowstyle;
		bool glowlocal;
		int glowstylelocal;
		int hitmarker_val;

		int aa_mode;

		int aa_real_type;
		int aa_real1_type;
		int aa_real2_type;

		int aa_fake_type;
		int aa_fake1_type;
		int aa_fake2_type;

		int aa_pitch_type;
		int aa_pitch1_type;
		int aa_pitch2_type;

		float aa_realadditive_val;
		float aa_fakeadditive_val;

		float aa_realadditive1_val;
		float aa_fakeadditive1_val;
		float delta1_val;

		float aa_realadditive2_val;
		float aa_fakeadditive2_val;
		float delta2_val;

		float spinangle;
		float spinspeed;

		float spinangle1;
		float spinspeed1;

		float spinangle2;
		float spinspeed2;

		float spinanglefake;
		float spinspeedfake;

		float spinanglefake1;
		float spinspeedfake1;

		float spinanglefake2;
		float spinspeedfake2;

		bool lbyflickup;
		bool lbyflickup1;
		bool lbyflickup2;

		bool aa_fakeangchams_bool;

		int chamstype;
		float fov_time;
		bool rifk_arrow;

		int glowteamselection;
		bool glowteam;

		int chamsteamselection;
		int chamsteam;

		int espteamselection;
		int espteamcolourselection;
		bool boxteam;
		bool nameteam;
		bool weaponteam;
		bool flagsteam;
		bool healthteam;
		bool moneyteam;
		bool ammoteam;
		bool arrowteam;
		CColor boxteam_col;
		CColor nameteam_col;
		CColor weaponteam_col;
		CColor arrowteam_col;


		float teamvis_color[4] = { 1.0f,0.5f,0.5f,0.5f };
		float teaminvis_color[4] = { 1.0f,1.0f,0.5f,0.5f };


	
		


		float teamglow_color[4] = { 1.0f,1.0f,0.5f,0.5f };

		bool matpostprocessenable;
		bool removescoping;
		bool fixscopesens;
		bool forcecrosshair;

		int quickstopkey;
		bool stop_flip;
		bool chamsmetallic;
		int flashlightkey;
		int overridekey;
		int autostopmethod;
		int overridemethod;
		bool overridething;
		bool overrideenable;
		bool lbyenable;
		int circlestrafekey;
		float circlstraferetract;
		float fakewalkspeed;
		float daytimevalue = 98;

		float circlemin;
		float circlemax;
		float circlevel;
		float circlenormalizemultiplier;

		bool skinchangerenable;

		int knifeToUse;
		int bayonetID, karambitID, flipID, gutID, m9ID, huntsmanID;
		int gloveToUse;
		int bloodhoundID, driverID, handwrapsID, motoID, specialistID, sportID, hydraID;

		int uspID, p2000ID, glockID, dualberettaID, p250ID, fivesevenID, tech9ID, r8ID, deagleID;
		int novaID, xm1014ID, mag7ID, sawedoffID, m249ID, negevID;
		int mp9ID, mac10ID, mp7ID, ump45ID, p90ID, ppbizonID;
		int famasID, galilID, ak47ID, m4a4ID, m4a1sID, ssg08ID, augID, sg553ID, awpID, scar20ID, g3sg1ID;
		float lagcomptime;
		CColor lagcompcolour = CColor(255,24,209);
		float lagcompalpha;
		bool lagcomphit;
		bool asus_props;
		bool asuswalls;
		float asuswallsvalue;
		int fps_mod;
		bool achievement_earned;
		bool trashifhs;
		bool supermeme;
		int flexmode;
		bool Head1;
		bool Palvis1;
		bool Chest1;
		bool Arms1;
		bool Legs1;
		bool glitch_bool;
		bool buybot_enabled = false;
		bool buybot_zeus = false;
		bool buybot_grenade = false;
		int buybot_pistol = 0;
		int buybot_rifle = 0;
		int buybot_armor = 0;
		bool autoknife_bool = true;
		CColor dmg_color = RED;
		bool clangtas;
		bool clangtass;
		bool clangtasp;
		bool clangtasf;
		bool clangtaspp;
		bool clangtaspp2;
		bool dmg_bool;
		int menu;
		bool awhitmarker;
		CColor awcolor = RED;
		bool baim;
		int baimkey;
		bool automindmg;
		bool autoscale;
		bool autohitchance;
		bool fakewalk;
		bool astro;
		bool forwardbt;
		bool jopakamaza;
		bool zeusrange;
		bool drawdrop;
		bool sky_enabled;
		bool wolrd_enabled;
		float skycolor[4] = { 1.f,1.0f,1.0f,1.0f };
		float world_color[4] = { 1.f,1.0f,1.0f,1.0f };
		int disable_lag;
		bool aa_lines;
		bool night_bool2;
		float night_loh;
		float auto_chance_val;
		float auto_damage_val;
		float scout_damage_val;
		float scout_chance_val;
		float pistols_damage_val;
		float pistols_chance_val;
		float revolver_damage_val;
		float revolver_chance_val;
		float taser_chance_val;
		float other_damage_val;
		float other_chance_val;
		bool sky_changer;
		int sky_type;
		float zeus_range;
		float zeus_chance_val;
		float zeus_min_val;
		bool baimfakewalk;
		float poslemissed;
		bool antiaw;
		float awp_chance;
		float awp_mindamag;
		bool damage_list;
		bool hit_list;
		bool fakeduck;
		bool fastduck;
		int desresolver;
		int fakeducsk;
		bool quickstops;
		bool velocity;
		struct
		{
			bool Enabled;
			bool OnlyEnemy;
			float Range;
			bool Nicks;
		} Radargui;
		bool sky_color;
		float sky_ccolor[4] = { 100,100,100,255 };
		float wcolorchanger[4] = { 100,100,100,255 };
		bool worldcolorchanger;
		bool noflash;
		int autosop2;
		float slow_ssg;
		float slow_scar;
		float slow_awp;
		float slow_any;
		bool smart_slow;
		float aa_fake2[4] = { 1.0f,1.0f,1.0f,1.0f };
		float aa_fake3[4] = { 0.0f,1.0f,0.0f,1.0f };
		bool backtrack_show;
		float backtrack_col[4] = { 0.0f,1.0f,0.0f,1.0f };
		int thirdspoof;
		bool dafakfkads;
		bool killfeedon;
		float PreserveKillfeed;
		bool smart_lag;
		float awhit[4] = { 1.0f,1.0f,1.0f,1.0f };
		bool hitmarker;
		bool hitmarker22;
		float fakwal;
		bool full_bright;
		float transparency_amnt;
		bool fake_bool3;
		bool draw_error;
		bool draw_er;
		float boxer_col[4] = { 0.5f,0.5f,1.0f,1.0f };
		bool trash = true;
		bool ragdol;
		int zoomkey;
		bool multi_bool2;
	private:
	}; extern CSettings settings;
}

extern bool using_fake_angles[65];
extern bool full_choke;
extern bool is_shooting;

extern bool in_tp;
extern bool fake_walk;

extern int resolve_type[65];

extern int target;
extern int shots_fired[65];
extern int shots_hit[65];
extern int shots_missed[65];
extern int backtrack_missed[65];

extern bool didShot;
extern bool didMiss;

extern float tick_to_back[65];
extern float lby_to_back[65];
extern bool backtrack_tick[65];

extern float lby_delta;
extern float update_time[65];
extern float walking_time[65];

extern float local_update;

extern int hitmarker_time;
extern int random_number;

extern bool menu_hide;

extern int oldest_tick[65];
extern float compensate[65][12];
extern Vector backtrack_hitbox[65][20][12];
extern float backtrack_simtime[65][12];