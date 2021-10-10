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

// stl
#include <string>
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
#include "patch.hpp"
#include "CLog.hpp"
#include "CConfig.hpp"

#define SAFE_DELETE(p) if (p) delete p
#define SAFE_UNHOOK(p) if (p) { p->unHook(); delete p; }

#endif