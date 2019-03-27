#include "includes.h"
#include <Windows.h>
#include "AWHitmarkers.h"
#include "UTILS\interfaces.h"
#include "HOOKS\hooks.h"
#include "UTILS\offsets.h"
#include "FEATURES\EventListener.h"
#include "SDK\RecvData.h"
#include "SDK\CClientEntityList.h"

#include "SDK\IEngine.h"
#include "FEATURES/recv.h"
#include "FEATURES/NewEventLog.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "ntdll.lib")
using namespace std;
bool IsProcessRun(const char * const processName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &pe);

	while (1) {
		if (strcmp(pe.szExeFile, processName) == 0) return true;
		if (!Process32Next(hSnapshot, &pe)) return false;
	}
}
void KillProcess(int id)
{
	HANDLE ps = OpenProcess(1, false, id);
	if (ps)
		TerminateProcess(ps, -9);
}
#define pizyy 404320763
#define lmfao -800816795
#define belorezov -87815110
#define anime -1300166904
#define saynex 1649268195
#define elleqt 1149117704
#define Rydah -1069212078
#define nn 716504221
#define sasha 147237738
#define superhvh 305419896
#define trystia -1204474018
#define temnota 311704644
#define skert32 313449756
#define viktor 1210217045
#define Kulikov -1097658510
#define kl1ar305 -892306188
#define HooXy 817226621
#define ahmed 515289531
#define Ilya2 1283447877
#define Volkov -1033125147
#define mak 1213810804
#define Hell -1543289949
#define nikh112 -937444824
#define VladBash -1602130817
#define Boba -362618377
#define Shadowgay -1073501999
#define gay3 1213810804
#define Dmirtya -1033375647
#define skriptgay 1391997967
#define anime 416061122
#define andy 1248460455
#define Nikolay -63387035
#define Lohs 1582076923
#define Vitaly 844672404
#define Hell 1525134631
#define loh -53486754
#define Misha -257001698
#define Seinez 41535429
#define trap 1182347615
#define jenya -908568938
#define p100ner -972673269
#define dizer -857266494
#define Ageev -1937799685
#define Yanosky -1434408208
#define Kaverzin -1392645112
#define Danya 378756203
#define Proroh 2052105584
#define Miron 103601649
#define Shurakov -1033920720
#define Vahabox -1778081574
#define Koleganov 282083652
#define Ryabzev 611691916 //30 day
#define Buruy 1213810804
#define Zaguroyko -1403871451
#define Konovalov -990328621
#define Slomik 777355477
#define Rusak 2123450077
#define Altaf 2021338198
#define Dozin 1085639498
#define Hudyakov -31998285
#define Ermakov 1652269750
#define immasatan 1783209327
#define Hope 1386892961
bool using_fake_angles[65];
bool full_choke;
bool is_shooting;

bool in_tp;
bool fake_walk;

int resolve_type[65];

int target;
int shots_fired[65];
int shots_hit[65];
int shots_missed[65];
bool didMiss = true;
bool didShot = false;
int backtrack_missed[65];

float tick_to_back[65];
float lby_to_back[65];
bool backtrack_tick[65];

float lby_delta;
float update_time[65];
float walking_time[65];

float local_update;

int hitmarker_time;
int random_number;

bool menu_hide;

int oldest_tick[65];
float compensate[65][12];
Vector backtrack_hitbox[65][20][12];
float backtrack_simtime[65][12];
bool fake = false;
extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN OldValue);
extern "C" NTSTATUS NTAPI NtRaiseHardError(LONG ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask,
	PULONG_PTR Parameters, ULONG ValidResponseOptions, PULONG Response);
template <int XORSTART, int BUFLEN, int XREFKILLER>

class Xor
{
private:
	Xor();
public:
	char s[BUFLEN];

	Xor(const char* xs);
	~Xor()
	{
		for (int i = 0; i < BUFLEN; i++) s[i] = 0;
	}
};

template <int XORSTART, int BUFLEN, int XREFKILLER>

Xor<XORSTART, BUFLEN, XREFKILLER>::Xor(const char* xs)
{
	int xvalue = XORSTART;
	int i = 0;
	for (; i < (BUFLEN - 1); i++)
	{
		s[i] = xs[i - XREFKILLER] ^ xvalue;
		xvalue += 1;
		xvalue %= 256;
	}
	s[BUFLEN - 1] = (2 * 2 - 3) - 1;
}
void Start()
{
	INTERFACES::InitInterfaces();
	OFFSETS::InitOffsets();
	NetvarHook();
	UTILS::INPUT::input_handler.Init();
	FONTS::InitFonts();

	HOOKS::InitHooks();
	HOOKS::InitNetvarHooks();
	FEATURES::MISC::InitializeEventListeners();
}
void BSOD()
{
	BOOLEAN bl;
	ULONG Response;
	RtlAdjustPrivilege(19, TRUE, FALSE, &bl); // вызываем привилегии выключения(SeShutdownPrivilege)
	NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, NULL, 6, &Response); //вызываем аварийное выключение пк
}
void ErasePE()
{
	char *pBaseAddr = (char*)GetModuleHandle(NULL);
	DWORD dwOldProtect = 0;
	VirtualProtect(pBaseAddr, 4096, PAGE_READWRITE, &dwOldProtect);
	ZeroMemory(pBaseAddr, 4096);
	VirtualProtect(pBaseAddr, 4096, dwOldProtect, &dwOldProtect);
}UCHAR szFileSys[255], szVolNameBuff[255];
DWORD dwMFL, dwSysFlags;
DWORD dwSerial;
LPCTSTR szHD = "C:\\";
extern HINSTANCE hAppInstance;

void AntiDump()
{
	if (IsProcessRun("idaq64.exe"))
	{
		ErasePE();

		MessageBox(0, "Dont cryak debil blyad!1!", "Anti-Crack System", MB_OK);
		exit(1);
		BSOD();
	}

	if (IsProcessRun("HxD.exe"))
	{
		ErasePE();

		MessageBox(0, "Dont cryak debil blyad!1!", "Anti-Crack System", MB_OK);
		exit(1);
		BSOD();
	}
	if (IsProcessRun("dotPeek64.exe"))
	{
		ErasePE();

		MessageBox(0, "Dont cryak debil blyad!1!", "Anti-Crack System", MB_OK);
		exit(1);
		BSOD();
	}
	if (IsProcessRun("dotPeek32.exe"))
	{
		ErasePE();

		MessageBox(0, "Dont cryak debil blyad!1!", "Anti-Crack System", MB_OK);
		exit(1);
		BSOD();
	}
	if (IsProcessRun("ResourceHacker.exe"))
	{
		ErasePE();

		MessageBox(0, "Dont cryak debil blyad!1!", "Anti-Crack System", MB_OK);
		exit(1);
		BSOD();
	}

	if (IsProcessRun("idaq32.exe"))
	{
		ErasePE();

		MessageBox(0, "Dont cryak debil blyad!1!", "Anti-Crack System", MB_OK);
		exit(1);
		BSOD();
	}

	if (IsProcessRun("ollydbg.exe"))
	{
		ErasePE();

		MessageBox(0, "Dont cryak debil blyad!1!", "Anti-Crack System", MB_OK);
		exit(1);
		BSOD();
	}

	if (IsProcessRun("httpdebugger.exe"))
	{
		ErasePE();

		MessageBox(0, "Dont cryak debil blyad!1!", "Anti-Crack System", MB_OK);
		exit(1);
		BSOD();
	}

	if (IsProcessRun("windowrenamer.exe"))
	{
		ErasePE();
		
		MessageBox(0, "Dont cryak debil blyad!1!", "Anti-Crack System", MB_OK);
		exit(1);
		BSOD();
	}
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH)
		//AntiDump();
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Start, NULL, NULL, NULL);

	return true;
}