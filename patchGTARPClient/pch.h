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
#ifndef PCH_H
#define PCH_H

#define FNC_CAST(fnc, addr) ((decltype(fnc)*)addr)

#define SAFE_DELETE(p) \
    if (p) { \
        delete p; \
        p = nullptr; \
    }

#define SAFE_RELEASE(p) \
    if (p) { \
        p->Release();\
        p=nullptr; \
    }

#define GET_ADDR(a) getAddress((unsigned long)(a))

// fix compilation
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS


// STL
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <functional>
#include <array>
#include <regex>
#include <filesystem>
#include <fstream>
#include <thread>
#include <mutex>
#include <memory>


// C (Old-Library)
#include <cstring>
#include <cstdint>
#include <cassert>


// WIN-API
#include <Windows.h>
#include <Psapi.h>
#include <memory.h>


// DirectX 9
#include <d3d9.h>
#include <d3dx9.h>


// BOOST
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>


// Poly-Hook2
#include <polyhook2/Detour/x86Detour.hpp>


// Plugin SDK
#define GTASA
#define _LA_SUPPORT
#define _DX9_SDK_INSTALLED
#define PLUGIN_SGV_10US

#include <plugin.h>
#include <CSprite2d.h>


// third_party
#include "json.hpp"
#include "bass.h"
#include "BlurEffect.h"


// my library
#include "FSignal.h"
#include "patch.hpp" 
#include "client.h"
#include "snippets.h"
#include "Log.h"
#include "Config.h"
#include "SAMP.h"
#include "D3D9Hook.h"

#include "MLoad.h"

extern SAMP*     g_pSAMP;
extern D3D9Hook* g_pD3D9Hook;
extern Log       g_Log;
extern Config    g_Config;
extern HMODULE   g_DLLModule;
extern snippets::DynamicLibrary g_gtarpclientBase;
extern snippets::DynamicLibrary g_sampBase;
extern snippets::DynamicLibrary g_D3DX9_25;
extern snippets::DynamicLibrary g_SF;

extern FSignal<void()> g_onInitAudio;
extern FSignal<void()> g_onInitSamp;

extern snippets::FastMap<std::string, std::shared_ptr<MLoad>> g_modules;

extern const char* g_szCurrentVersion;

#endif //PCH_H
