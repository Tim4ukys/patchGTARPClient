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
#include "offsets.hpp"

#ifdef _MSC_VER
#pragma warning( disable : 4099 )
#pragma warning( disable : 4250 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4733 )
#pragma warning( disable : 4996 )
#endif

// stl
#include <string>
#include <functional>
// old (C) Library
#include <cstdint>
#include <cstdio>
#include <ctime>
// win-api
#include <Windows.h>

#pragma comment(lib, "Crypt32.lib")
// Hook Library
#include <polyhook2/Detour/x86Detour.hpp>
#include <polyhook2/CapstoneDisassembler.hpp>

#pragma comment(lib, "capstone.lib")
#pragma comment(lib, "PolyHook_2.lib")
// Plugin SDK
#include "plugin.h"
#include "CSprite2d.h"

#pragma comment(lib, "plugin.lib")
// DirectX SDK
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
// OpenSSL
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
// My library
#include "CLog.hpp"
extern CLog* g_pLog;

#include "CConfig.hpp"
#include "patch.hpp"
#include "snippets.hpp"
#include "Client.hpp"

typedef ULONGLONG TICK;

#define SAFE_UNHOOK(p) if (p) { p->unHook(); delete p; p=nullptr; }
#define SAFE_DELETE(p) if (p) { delete p; p=nullptr; }
#define SAFE_DELETEARRAY(p) if (p) { delete[] p; p=nullptr; }
#define SAFE_RELEASE(p) if (p) { p->Release(); p=nullptr; }

#include "CSAMP.h"

#endif