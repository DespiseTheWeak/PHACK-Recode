#include "../includes.h"

#include "interfaces.h"
#include "../FEATURES/Movement.h"
#include "../SDK/ISurface.h"
#include "render.h"
#include "../SDK/IEngine.h"
#include "../SDK/CUserCmd.h"
#include <urlmon.h>
#pragma comment (lib, "urlmon.lib")
namespace GLOBAL
{
	HWND csgo_hwnd = FindWindow(0, "Counter-Strike: Global Offensive");

	bool should_send_packet;
	bool shoot;
	bool is_fakewalking;
	int choke_amount;

	Vector real_angles;
	Vector fake_angles;
	Vector strafe_angle;
	Vector angles;

	int randomnumber;
	float	flHurtTime;
	bool DisableAA;
	bool Aimbotting;

	using msg_t = void(__cdecl*)(const char*, ...);
	msg_t Msg = reinterpret_cast< msg_t >(GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg"));

	Vector FakePosition;
	int ground_tickz;
	bool CircleStraferActive;
	SDK::CUserCmd originalCMD;
	bool NewRound;
}
namespace FONTS
{
	unsigned int menu_tab_font;
	unsigned int menu_checkbox_font;
	unsigned int menu_slider_font;
	unsigned int menu_groupbox_font;
	unsigned int menu_combobox_font;
	unsigned int menu_window_font;
	unsigned int numpad_menu_font;
	unsigned int visuals_esp_font;
	unsigned int visuals_xhair_font;
	unsigned int visuals_side_font;
	unsigned int visuals_name_font;
	unsigned int visuals_lby_font;
	unsigned int visuals_grenade_pred_font;
	unsigned int  in_game_logging_font;
	unsigned int chlen;
	unsigned int chlen_if_hs;
	unsigned int MenuTab;
	bool ShouldReloadFonts()
	{
		static int old_width, old_height;
		int width, height;
		INTERFACES::Engine->GetScreenSize(width, height);

		if (width != old_width || height != old_height)
		{
			old_width = width;
			old_height = height;
			return true;
		}
		return false;
	}
	void InitFonts()
	{
		static bool is_init = false;
		if (!is_init)
		{
			is_init = true;
			URLDownloadToFile(NULL, ("https://phack.pw/Untitled1.ttf"), ("C:\\phack\\Untitled1.ttf"), NULL, NULL);
			AddFontResource(("C:\\phack\\Untitled1.ttf"));
		}
		menu_tab_font = RENDER::CreateF("Calibri", 18, 550, 0, 0, NULL);
		menu_checkbox_font = RENDER::CreateF("Verdana", 16, 550, 0, 0, NULL);
		menu_slider_font = RENDER::CreateF("Verdana", 14, 550, 0, 0, NULL);
		menu_groupbox_font = RENDER::CreateF("Verdana", 16, 550, 0, 0, NULL);
		menu_combobox_font = RENDER::CreateF("Verdana", 14, 550, 0, 0, NULL);
		menu_window_font = RENDER::CreateF("Verdana", 16, 550, 0, 0, NULL);
		numpad_menu_font = RENDER::CreateF("Tahoma", 13, 600, 0, 0, SDK::FONTFLAG_OUTLINE);
		//visuals_esp_font = RENDER::CreateF("Tahoma", 9, 600, 0, 0, SDK::FONTFLAG_OUTLINE);
		visuals_esp_font = RENDER::CreateF("Smallest Pixel-7", 10, 100, 0, 0, SDK::FONTFLAG_OUTLINE);
		visuals_xhair_font = RENDER::CreateF("Tahoma", 18, 600, 0, 0, SDK::FONTFLAG_OUTLINE);
		visuals_side_font = RENDER::CreateF("Calibri", 46, 900, 0, 0, NULL);
		visuals_name_font = RENDER::CreateF("Tahoma", 12, 300, 0, 0, SDK::FONTFLAG_ANTIALIAS | SDK::FONTFLAG_DROPSHADOW);
		visuals_lby_font = RENDER::CreateF("Verdana", 32, 650, 0, 0, SDK::FONTFLAG_ANTIALIAS | SDK::FONTFLAG_DROPSHADOW);
		visuals_grenade_pred_font = RENDER::CreateF("undefeated", 16, 400, 0, 0, SDK::FONTFLAG_DROPSHADOW | SDK::FONTFLAG_ANTIALIAS);
		in_game_logging_font = RENDER::CreateF("Smallest Pixel-7", 15, 550, 0, 0, SDK::FONTFLAG_OUTLINE);
		//visuals_esp_font = RENDER::CreateF("Calibri", 11, 550, 0, 0, SDK::FONTFLAG_OUTLINE);
		chlen = RENDER::CreateF("Tahoma", 20, 550, 0, 0, SDK::FONTFLAG_ANTIALIAS | SDK::FONTFLAG_OUTLINE);
		chlen_if_hs = RENDER::CreateF("Tahoma", 26, 550, 0, 0, SDK::FONTFLAG_ANTIALIAS | SDK::FONTFLAG_OUTLINE | SDK::FONTFLAG_ITALIC);
	}
}
namespace SETTINGS
{
	CSettings settings; 

	bool CSettings::Save(std::string file_name)
	{
		std::string file_path = "C:\\PHACK\\" + file_name + ".cfg";

		std::fstream file(file_path, std::ios::out | std::ios::in | std::ios::trunc);
		file.close();

		file.open(file_path, std::ios::out | std::ios::in);
		if (!file.is_open())
		{
			file.close();
			return false;
		}

		const size_t settings_size = sizeof(CSettings);
		for (int i = 0; i < settings_size; i++)
		{
			byte current_byte = *reinterpret_cast<byte*>(uintptr_t(this) + i);
			for (int x = 0; x < 8; x++)
			{
				file << (int)((current_byte >> x) & 1);
			}
		}

		file.close();

		return true;
	}
	bool CSettings::Load(std::string file_name)
	{
		CreateDirectory("C:\\PHACK", NULL);

		std::string file_path = "C:\\PHACK\\" + file_name + ".cfg";

		std::fstream file;
		file.open(file_path, std::ios::out | std::ios::in);
		if (!file.is_open())
		{
			file.close();
			return false;
		}

		std::string line;
		while (file)
		{
			std::getline(file, line);

			const size_t settings_size = sizeof(CSettings);
			if (line.size() > settings_size * 8)
			{
				file.close();
				return false;
			}
			for (int i = 0; i < settings_size; i++)
			{
				byte current_byte = *reinterpret_cast<byte*>(uintptr_t(this) + i);
				for (int x = 0; x < 8; x++)
				{
					if (line[(i * 8) + x] == '1')
						current_byte |= 1 << x;
					else
						current_byte &= ~(1 << x);
				}
				*reinterpret_cast<byte*>(uintptr_t(this) + i) = current_byte;
			}
		}

		file.close();

		return true;
	}
	void CSettings::CreateConfig(std::string name)
	{
		CreateDirectory("C:\\PHACK\\", NULL); CreateDirectory("C:\\PHACK\\", NULL);
		std::ofstream ofs("C:\\PHACK\\" + name + ".cfg");
	}

	bool CSettings::Remove(std::string file_name)
	{
		CreateDirectory("C:\\PHACK", NULL);

		std::string file_path = "C:\\PHACK\\" + file_name + ".cfg";
		remove(file_path.c_str());

		return true;
	}

	std::vector<std::string> CSettings::GetConfigs()
	{
		std::vector<std::string> configs;

		WIN32_FIND_DATA ffd;
		auto directory = "C:\\PHACK\\*";
		auto hFind = FindFirstFile(directory, &ffd);

		while (FindNextFile(hFind, &ffd))
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string file_name = ffd.cFileName;
				if (file_name.size() < 4) // .cfg
					continue;

				std::string end = file_name;
				end.erase(end.begin(), end.end() - 4); // erase everything but the last 4 letters
				if (end != ".cfg")
					continue;

				file_name.erase(file_name.end() - 4, file_name.end()); // erase the .cfg part
				configs.push_back(file_name);
			}
		}

		return configs;
	}
}