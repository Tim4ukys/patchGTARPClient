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
*/

// ----------------------------------------------------------
// Исправление уровня розыска

uint64_t        g_ui64DrawWantedJumpTrampline;
PLH::x86Detour* g_pDrawWantedDetour = nullptr;
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
    CSprite2d* m_pSprites = nullptr;
    float      m_fIconPos[2];
    bool       m_bState;
} g_serverIcon;

int   drawServerIcon() {
    if (g_serverIcon.m_bState) {
        g_serverIcon.m_pSprites->Draw(
            SCREEN_COORD_LEFT(g_serverIcon.m_fIconPos[0]), SCREEN_COORD_TOP(g_serverIcon.m_fIconPos[1]),
            SCREEN_COORD(static_cast<float>(g_serverIcon.m_pSprites->m_pTexture->raster->width / 2)),
            SCREEN_COORD(static_cast<float>(g_serverIcon.m_pSprites->m_pTexture->raster->height / 2)),
            CRGBA(0xFF, 0xFF, 0xFF));
    }

    return NULL;
}

uint64_t        g_ui64LoadTextureHudJumpTrampline;
PLH::x86Detour* g_pLoadTextureHudDetour = nullptr;
NOINLINE void   loadTextureHudDetourFNC() {
    ((void(__cdecl*)())g_ui64LoadTextureHudJumpTrampline)(); // call original

    RwTexture** serverIcon = reinterpret_cast<RwTexture**>(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::ARRAYSERVERLOGO));
    //while (!serverIcon[0] || !serverIcon[1] || !serverIcon[2]) Sleep(100);

    g_serverIcon.m_pSprites = new CSprite2d;
    auto serverID = *reinterpret_cast<int*>(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::SERVERID));
    serverID = serverID > 2 || serverID < 0 ? 0 : serverID;

    g_serverIcon.m_pSprites->m_pTexture = serverIcon[serverID];

    plugin::patch::ReplaceFunction(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::DRAWHUD), &drawServerIcon);
}

// -----------------------------

void OldHUD::Process() {

    bFixTimeZone = g_Config["clock"]["fixTimeZone"].get<bool>();

    g_serverIcon.m_bState = g_Config["serverIcon"]["state"].get<bool>();
    g_serverIcon.m_fIconPos[0] = g_Config["serverIcon"]["x"].get<float>();
    g_serverIcon.m_fIconPos[1] = g_Config["serverIcon"]["y"].get<float>();

    // --------

    auto dis = PLH::CapstoneDisassembler(PLH::Mode::x86);

    /*
        Возрващает самповский худ
        .text:0002B81D - start nop
        .text:0002B97A - end
        ---
        size = 0x168
    */
    patch::fill(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::INITTEXTURE_INITHOOK), 0x15D, 0x90);

    /* Возвращает часы */
    patch::fill(g_sampBase.GET_ADDR(OFFSETS::SAMP::ENABLECLOCK), 2, 0x90);
    
    /* Фиксит время на часах */
    plugin::patch::ReplaceFunctionCall(static_cast<uint32_t>(OFFSETS::GTA_SA::DRAWHUD_CLOCK_SPRINTF), &drawClockSprintfDetourFNC);

    /* Исправляет уровень розыска */
    g_pDrawWantedDetour = new PLH::x86Detour(reinterpret_cast<PCHAR>(OFFSETS::GTA_SA::DRAWWANTED), reinterpret_cast<PCHAR>(&drawWantedDetourFNC), 
        &g_ui64DrawWantedJumpTrampline, dis);
    g_pDrawWantedDetour->hook();

    /* Фиксит положение иконки сервера */
    g_pLoadTextureHudDetour = new PLH::x86Detour(
        reinterpret_cast<PCHAR>(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::INITTEXTURE_FNC)), reinterpret_cast<PCHAR>(&loadTextureHudDetourFNC),
        &g_ui64LoadTextureHudJumpTrampline, dis);
    g_pLoadTextureHudDetour->hook();
}