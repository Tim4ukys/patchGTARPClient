/****************************************************
*                                                   *
*    Developer: Tim4ukys                            *
*                                                   *
*    email: tim4ukys.dev@yandex.ru                  *
*    vk: vk.com/tim4ukys                            *
*                                                   *
*    License: GNU GPLv3                             *
*                                                   *
****************************************************/
#ifndef _DLLMAIN_H_
#define _DLLMAIN_H_

#define WIN32_LEAN_AND_MEAN

#pragma warning( disable : 4099 )
#pragma warning( disable : 4250 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4733 )
#pragma warning( disable : 4996 )

enum class OFFSETS : unsigned int {
    // gtarp_clientside.asi + ...
    GTARP_LOCKCONNECTTOOTHERSERVER = 0x48F2,
    GTARP_INITTEXTURE_INITHOOK = 0x1B81D,

    GTARP_ARRAYSERVERLOGO = 0xDD69C,
    GTARP_SERVERID = 0xCD990,

    GTARP_DRAWHUD = 0x1BBE0,
    GTARP_DRAWSPEEDOMETER_DRAWRAMKA = 0x24CDA,

    GTARP_DISABLECHECKTXD = 0xE834,

    // samp.dll + ...
    SAMP_ENABLECLOCK = 0xA0D84,

    SAMP_CREATEFONT = 0xC650A,

    SAMP_RENDERPLAYERTAG_FORMAT = 0x74E35,
    SAMP_RENDERPLAYERTAG_PUSHBUFF_1 = 0x74E3A,
    SAMP_RENDERPLAYERTAG_PUSHBUFF_2 = 0x74E80,

    SAMP_FORMATPATHSCREENSHOT = 0x61B52,
    SAMP_FORMATFULLPATHSCREENSHOT = 0xC4573,

    // gta_sa.exe
    GTASA_DRAWWANTED = 0x58D9A0,
    GTASA_DRAWHUD_CLOCK_SPRINTF = 0x58EBAF,
    GTASA_GAMELOOP = 0x53BEE0
};

// stl
#include <string>
#include <functional>
// old (C) Library
#include <cstdint>
#include <cstdio>
#include <ctime>
// win-api
#include <Windows.h>
// Hook Library
#include <polyhook2/Detour/x86Detour.hpp>
#include <polyhook2/CapstoneDisassembler.hpp>

#pragma comment(lib, "capstone.lib")
#pragma comment(lib, "PolyHook_2.lib")
// Plugin SDK
#define GTASA
#define GTAGAME_NAME = "San Andreas"
#define GTAGAME_ABBR = "SA"
#define GTAGAME_ABBRLOW = "sa"
#define GTAGAME_PROTAGONISTNAME = "CJ"
#define GTAGAME_CITYNAME = "San Andreas"
#define _LA_SUPPORT
#define _DX9_SDK_INSTALLED
#define PLUGIN_SGV_10US

#include "plugin.h"
#include "CSprite2d.h"
// My library
#include "CLog.hpp"
extern CLog* g_pLog;
#include "TimeZone.hpp"

#include "CConfig.hpp"
#include "patch.hpp"
#include "snippets.hpp"
#include "Client.hpp"

typedef ULONGLONG TICK;

#define SAFE_UNHOOK(p) if (p) { p->unHook(); delete p; }
#define SAFE_DELETE(p) if (p) delete p

#include "CSAMP.h"

#endif