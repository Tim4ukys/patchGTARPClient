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

enum class OFFSETS : unsigned int {
    // gtarp_clientside.asi + ...
    GTARP_LOCKCONNECTTOOTHERSERVER = 0x48F2,
    GTARP_INITTEXTURE_INITHOOK = 0x1B81D,

    GTARP_ARRAYSERVERLOGO = 0xDD69C,
    GTARP_SERVERID = 0xCD990,

    GTARP_DRAWHUD = 0x1BBE0,

    // samp.dll + ...
    SAMP_ENABLECLOCK = 0xA0D84,

    // gta_sa.exe
    GTASA_DRAWWANTED = 0x58D9A0,
    GTASA_DRAWHUD_CLOCK_SPRINTF = 0x58EBAF
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
// other library
#include "qis/xorstr.hpp"
// My library
#include "CLog.hpp"
#include "CConfig.hpp"
#include "patch.hpp"
#include "snippets.hpp"

#define SAFE_UNHOOK(p) if (p) { p->unHook(); delete p; }
#define SAFE_DELETE(p) if (p) delete p

#endif