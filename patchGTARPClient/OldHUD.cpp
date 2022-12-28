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
#include "pch.h"
#include "OldHUD.h"
#include "offsets.hpp"

/**
* Этот файл:
* 1) Возвращает часы
* 2) Фиксит время на часах(показывает время по МСК)
* 3) Исправляет уровень розыска
* 4) Фиксит положение иконки сервера и его размер
* 
* 5) Возвращает старый радар
*/

// ----------------------------------------------------------
// Исправление уровня розыска

uint64_t                        g_ui64DrawWantedJumpTrampline;
std::unique_ptr<PLH::x86Detour> g_pDrawWantedDetour;
NOINLINE void   drawWantedDetourFNC() {
    static auto s_piWantedLevel = reinterpret_cast<DWORD*>(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::WANTEDLEVEL));

    if (*s_piWantedLevel != FindPlayerWanted(-1)->m_nWantedLevel) {
        FindPlayerPed(-1)->GetWanted()->SetWantedLevel(*s_piWantedLevel);
    }

    ((void(__cdecl*)())g_ui64DrawWantedJumpTrampline)(); // call original
}

// ----------------------------------------------------------
// Фиксит время на часах(показывает время по МСК) 

bool bFixTimeZone;
int drawClockSprintfDetourFNC(char* buff, const char* f, ...) {
    SYSTEMTIME timeInfo;
    GetLocalTime(&timeInfo);

    int32_t hour{timeInfo.wHour};

    if (bFixTimeZone) {
        static int8_t* s_pTimeOffset = nullptr;

        if (!s_pTimeOffset) {
            TIME_ZONE_INFORMATION timeZoneInfo;
            GetTimeZoneInformation(&timeZoneInfo);

            s_pTimeOffset = new int8_t;
            *s_pTimeOffset = int8_t(timeZoneInfo.Bias / 60 + 3);
        }

        hour += *s_pTimeOffset;
        hour += hour < 0 ? 24 : (hour > 23 ? -24 : 0);
    }

    return sprintf(buff, "%02d:%02d", hour, timeInfo.wMinute);
}

// ----------------------------------------------------------
// Фиксит положение иконки сервера и его размер

struct stServerIcon {
    CSprite2d  m_Sprite[4];
    float      m_fIconPos[2];
    float      m_fIconSize[2];
    bool       m_bState;

    int* m_pIsX2;
    UINT m_uiTickCount{};
    int  m_nLamp{};
} g_serverIcon;

int   drawServerIcon() {
    if (g_serverIcon.m_bState) {
        int nLamp;
        if (GetTickCount() - g_serverIcon.m_uiTickCount <= 1000) {
            nLamp = g_serverIcon.m_nLamp;
        } else {
            g_serverIcon.m_uiTickCount = GetTickCount();
            nLamp = g_serverIcon.m_nLamp ^ 1;
            g_serverIcon.m_nLamp ^= 1u;
        }

        g_serverIcon.m_Sprite[nLamp + *g_serverIcon.m_pIsX2 * 2].Draw(
            SCREEN_COORD_RIGHT(g_serverIcon.m_fIconPos[0]), SCREEN_COORD_TOP(g_serverIcon.m_fIconPos[1]),
            SCREEN_COORD(g_serverIcon.m_fIconSize[0]),
            SCREEN_COORD(g_serverIcon.m_fIconSize[1]),
            CRGBA(0xFF, 0xFF, 0xFF));
    }

    return NULL;
}

uint64_t                        g_ui64LoadTextureHudJumpTrampline;
std::unique_ptr<PLH::x86Detour> g_pLoadTextureHudDetour;
NOINLINE void   loadTextureHudDetourFNC() {
    ((void(__cdecl*)())g_ui64LoadTextureHudJumpTrampline)(); // call original

    plugin::patch::ReplaceFunction(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::DRAWHUD), &drawServerIcon);

    if (!g_serverIcon.m_bState) return;

    //RwTexture** serverIcon = reinterpret_cast<RwTexture**>(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::ARRAYSERVERHALLOWEEN));
    //while (!serverIcon[0] || !serverIcon[1] || !serverIcon[2]) Sleep(100);

    //auto serverID = *reinterpret_cast<int*>(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::SERVERID));
    //if (serverID < 0 || serverID > 1) serverID = 2;
    g_serverIcon.m_pIsX2 = reinterpret_cast<int*>(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::NEWYEAR_ISX2));
    
    //g_serverIcon.m_Sprite.m_pTexture = reinterpret_cast<RwTexture**>(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::ARRAYSERVERLOGO))[serverID];
    for (size_t i{}; i < 4; ++i)
        g_serverIcon.m_Sprite[i].m_pTexture = ((RwTexture**)g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::NEWYEAR_SERVERLOGO))[i];
}

// -----------------------------
// return old path TXD

int fmtDirPath(char* a1 /*obuf*/, char* a2 /*fmt*/, char* a3 /*currPath*/, int a4 /*idTXDArhive*/) {
    return wsprintfA(a1, g_Config["oldHud"]["pathToTXDhud"].get<std::string>().c_str(), a3);
}

// -----------------------------
// scale fix

bool* g_pSAMPHudScale;

uint64_t                        g_ui64HudScaleFixDetourJumpTrampline;
std::unique_ptr<PLH::x86Detour> g_pHudScaleFixDetourDetour;
void hudScaleFixDetour(PCHAR a1) {
    FNC_CAST(hudScaleFixDetour, g_ui64HudScaleFixDetourJumpTrampline)(a1);
    g_Config["oldHud"]["radarScaleFix"] = *g_pSAMPHudScale/* ^ true*/;
    g_Config.saveFile();
}

// -----------------------------

void OldHUD::Process() {
    if (g_Config["oldHud"]["hud"].get<bool>()) {
        bFixTimeZone = g_Config["clock"]["fixTimeZone"].get<bool>();

        g_serverIcon.m_bState = g_Config["serverIcon"]["state"].get<bool>();
        g_serverIcon.m_fIconPos[0] = g_Config["serverIcon"]["x"].get<float>();
        g_serverIcon.m_fIconPos[1] = g_Config["serverIcon"]["y"].get<float>();
        g_serverIcon.m_fIconSize[0] = g_Config["serverIcon"]["width"].get<float>();
        g_serverIcon.m_fIconSize[1] = g_Config["serverIcon"]["height"].get<float>();
        // --------

        /*
            Возрващает самповский худ
            push    ecx // nop
            mov     dword_D8BA4, eax // save
            jmp     $+162 // 162-5=15D
        */
        //patch__fill(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::INITTEXTURE_INITHOOK), 0x162/*0x15D*/, 0x90);
        auto addr = g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::INITTEXTURE_INITHOOK);
        patch__fill(addr, 0x1, 0x90);
        patch__setRaw(addr + 6, "\xE9\x5D\x01\x00\x00", 5);

        /* Возвращает часы */
        patch__fill(g_sampBase.GET_ADDR(OFFSETS::SAMP::ENABLECLOCK), 2, 0x90);

        /* Фиксит время на часах */
        plugin::patch::ReplaceFunctionCall(static_cast<uint32_t>(OFFSETS::GTA_SA::DRAWHUD_CLOCK_SPRINTF), &drawClockSprintfDetourFNC);

        /* Исправляет уровень розыска */
        g_pDrawWantedDetour = std::make_unique<PLH::x86Detour>(UINT64(OFFSETS::GTA_SA::DRAWWANTED),
                                                               UINT64(&drawWantedDetourFNC),
                                                               &g_ui64DrawWantedJumpTrampline);
        g_pDrawWantedDetour->hook();

        /* Фиксит положение иконки сервера */
        g_pLoadTextureHudDetour = std::make_unique<PLH::x86Detour>(UINT64(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::INITTEXTURE_FNC)),
                                                                   UINT64(&loadTextureHudDetourFNC),
                                                                   &g_ui64LoadTextureHudJumpTrampline);
        g_pLoadTextureHudDetour->hook();
    }
    if (g_Config["oldHud"]["radar"].get<bool>()) {
        /* Возвращает радар */
        patch__fill(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::DISABLE_CALL_SET_CUSTOM_RADAR), 0x5, 0x90);
    }
    if (strcmp(g_Config["oldHud"]["pathToTXDhud"].get<std::string>().c_str(), "NONE") != 0) {
        const DWORD addrPatch = g_gtarpclientBase.getAddress((DWORD)OFFSETS::GTARP::CUSTOM_PATH_TXD_HUD);
        //patch::setRaw(g_gtarpclientBase.getAddress(addrPatch), "\x90\x90", 2u);
        patch__fill(addrPatch, 2u, 0x90);
        patch__setJump(addrPatch, uint32_t(&fmtDirPath), 0u, TRUE);
    }

    g_pSAMPHudScale = (bool*)g_sampBase.GET_ADDR(OFFSETS::SAMP::SCALE_HUD_FIX_STATE);
    g_Log.Write("g_pSAMPHudScale: %p | nop: 0x%X", g_pSAMPHudScale, g_sampBase.GET_ADDR(OFFSETS::SAMP::DISABLE_LOAD_SCALE_STATE));
    g_pHudScaleFixDetourDetour = std::make_unique<PLH::x86Detour>(UINT64(g_sampBase.GET_ADDR(OFFSETS::SAMP::SCALE_HUD_FIX)),
                                                                  UINT64(&hudScaleFixDetour),
                                                                  &g_ui64HudScaleFixDetourJumpTrampline);
    g_pHudScaleFixDetourDetour->hook();
    *g_pSAMPHudScale = g_Config["oldHud"]["radarScaleFix"].get<bool>();
    patch__fill(g_sampBase.GET_ADDR(OFFSETS::SAMP::DISABLE_LOAD_SCALE_STATE), 7u, 0x90);
    patch__setRaw(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::DISABLE_RECHANGE_SCALE_STATE), "\xEB\x23", 2u); /*jmp $+0x23*/
}