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
#include "dllmain.h"

const std::string SAMP_CMP{ "E86D9AA083C41C85C0" };
const std::string GTARPCLIENTSIDE_CMP{ "CCCC558BEC568D71908B" };

#define UPDATE_DELAY 12000
#define CURRENTVERSIONA "v2.0"
#define CURRENTVERSION L"v2.0"
#define GITHUBURLA "github.com/Tim4ukys/patchGTARPClient"
#define GITHUBURL L"github.com/Tim4ukys/patchGTARPClient"

CLog* g_pLog        = nullptr;
CConfig* g_pConfig  = nullptr;

DynamicLibrary g_gtarpclientside{ "gtarp_clientside.asi" };
DynamicLibrary g_dlSAMP{ "samp.dll" };

PLH::CapstoneDisassembler g_dis = { PLH::Mode::x86 };

///////////////////////////////////////////////////////////////////

uint64_t g_ui64DrawWantedJumpTrampline;
PLH::x86Detour* g_pDrawWantedDetour = nullptr;
NOINLINE void drawWantedDetourFNC()
{
    static auto s_piWantedLevel = reinterpret_cast<DWORD*>(g_gtarpclientside.getAddress(0xDD450));

    if (*s_piWantedLevel != FindPlayerWanted(-1)->m_nWantedLevel)
    {
        FindPlayerWanted(-1)->SetWantedLevel(*s_piWantedLevel);
    }

    ((void(__cdecl*)())g_ui64DrawWantedJumpTrampline)(); // call original
}

int drawClockSprintfDetourFNC(char* buff, const char* f, ...)
{
    auto t = time(0);
    auto local_t = localtime(&t);
    auto r = sprintf(buff, "%02d:%02d", local_t->tm_hour, local_t->tm_min);

    return r;
}

uint64_t g_ui64D3DXCreateFontJumpTrampline;
PLH::x86Detour* g_pD3DXCreateFontDetour = nullptr;
NOINLINE int WINAPI D3DXCreateFontDetourFNC(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11/*fontFaceName*/, int a12, int a13, int a14, int a15)
{
    return ((int(WINAPI*)(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15))g_ui64D3DXCreateFontJumpTrampline)
        (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, int(g_pConfig->getConfig()->m_samp.m_FontFaceName.c_str()), a12, a13, a14, a15);
}

////

char g_arrBuffPlayerTag[128 + 8];

int renderPlayerTagSprintfDetourFNC(char* buff, const char* f, ...)
{
    va_list args;
    va_start(args, f);
    auto r = vsprintf(g_arrBuffPlayerTag, "%s {FFFFFF}(%d)", args);
    va_end(args);

    return r;
}

uint64_t g_ui64DrawLabelJumpTrampline;
PLH::x86Detour* g_pDrawLabelDetour = nullptr;
NOINLINE void __fastcall drawLabelDetourFNC(void* p_this, void* EDX, void* pos, const char* text, uint32_t color, float distanceCamera, bool drawStatus, int status)
{
    return ((void(__thiscall*)(PVOID, PVOID, const char*, D3DCOLOR, float, bool, int))g_ui64DrawLabelJumpTrampline)
        (p_this, pos, g_arrBuffPlayerTag, color, distanceCamera, drawStatus, status);
}

////

CSAMP* g_pSAMP = nullptr;

uint64_t g_ui64GameLoopJumpTrampline;
PLH::x86Detour* g_pGameLoopDetour = nullptr;
NOINLINE void gameLoopDetourFNC()
{
    ((void(*)())g_ui64GameLoopJumpTrampline)(); // call original fnc

    static bool s_isInit{};
    if (s_isInit || !g_pSAMP->isSAMPInit()) return;

    static TICK s_oldTime = GetTickCount64();
    if (GetTickCount64() - s_oldTime > UPDATE_DELAY)
    {
        json j = json::parse(Client::downloadStringFromURL(true, "raw.githubusercontent.com", "/Tim4ukys/patchGTARPClient/master/update.json"));
        if (strcmp(j["vers"].get<std::string>().c_str(), CURRENTVERSIONA) != 0)
        {
            g_pSAMP->AddChatMessage(0x990000, "[{FF9900}patchGTARPClient{990000}] {990000}ВНИМАНИЕ: {FF9900}Вышло обновление!");
            g_pSAMP->AddChatMessage(0x990000, "[{FF9900}patchGTARPClient{990000}] {FF9900}Пожалуйста, обновите плагин!");
            g_pSAMP->AddChatMessage(0x990000, "[{FF9900}patchGTARPClient{990000}] {FF9900}Сайт: {990000}" GITHUBURLA "{FF9900}!");
        }
        g_pLog->Log("[CUpdate] Last version: %s", j["vers"].get<std::string>().c_str());
        s_isInit = true;
    }
}

///////////////////////////////////////////////////////////////////

CSprite2d* g_pServerSprite = nullptr;

int drawServerIcon()
{
    auto conf = g_pConfig->getConfig();
    if (conf->m_serverIcon.m_bState)
    {
        g_pServerSprite->Draw(
            SCREEN_COORD_LEFT(conf->m_serverIcon.m_fX), SCREEN_COORD_TOP(conf->m_serverIcon.m_fY), 
            SCREEN_COORD(static_cast<float>(g_pServerSprite->m_pTexture->raster->width / 2)), SCREEN_COORD(static_cast<float>(g_pServerSprite->m_pTexture->raster->height / 2)),
            CRGBA(0xFF, 0xFF, 0xFF)
        );
    }

    return NULL;
}

void MainThread()
{
    while (!g_gtarpclientside.getAddress()) Sleep(10);

    if (patch::getHEX(g_gtarpclientside.getAddress(0xBABE), 10) != GTARPCLIENTSIDE_CMP) {
        MessageBoxW(
            NULL,
            L"ERROR: Эта версия плагина ещё не поддерживает эту версию клиента игры.\n\n"
            L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENTVERSION L"\n"
            L"GitHub: " GITHUBURL,
            L"!000patchGTARPClientByTim4ukys.ASI",
            MB_ICONERROR
        );
        TerminateProcess(GetCurrentProcess(), -1);
    }
    else if (patch::getHEX(g_dlSAMP.getAddress(0xBABE), 10) != SAMP_CMP) {
        MessageBoxW(
            NULL,
            L"ERROR: Эта версия плагина ещё не поддерживает эту версию SAMP.\n\n"
            L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENTVERSION L"\n"
            L"GitHub: " GITHUBURL,
            L"!000patchGTARPClientByTim4ukys.ASI",
            MB_ICONERROR
        );
        TerminateProcess(GetCurrentProcess(), -1);
    }
    else {
        g_pLog->Log("gtarp_clientside & samp: Version supported!!!");
    }

    /*
        Выключает лок на подключение на стороние сервера.
    */
    patch::setRaw(g_gtarpclientside.getAddress(OFFSETS::GTARP_LOCKCONNECTTOOTHERSERVER), "\xEB\x17\x90", 3);

    /*
        Выключаем проверку на повреждение GTARP_HUD.TXD
    */
    patch::setRaw(g_gtarpclientside.getAddress(OFFSETS::GTARP_DISABLECHECKTXD), "\xEB\x42", 2);

    /*
        Возрващает самповский худ

        .text:0002B81D - start nop
        .text:0002B97A - end
        ---
        size = 0x168
    */
    patch::fill(g_gtarpclientside.getAddress(OFFSETS::GTARP_INITTEXTURE_INITHOOK), 0x15D, 0x90);

    /*
        Исправляет уровень состояние розыска.
    */
    g_pDrawWantedDetour = new PLH::x86Detour(reinterpret_cast<PCHAR>(OFFSETS::GTASA_DRAWWANTED), reinterpret_cast<PCHAR>(&drawWantedDetourFNC), &g_ui64DrawWantedJumpTrampline, g_dis);
    g_pDrawWantedDetour->hook();

    /*
        Фиксит положение иконки сервера
    */
    RwTexture** serverIcon = reinterpret_cast<RwTexture**>(g_gtarpclientside.getAddress(OFFSETS::GTARP_ARRAYSERVERLOGO));
    while (!serverIcon[0] || !serverIcon[1] || !serverIcon[2]) Sleep(100);

    g_pServerSprite = new CSprite2d();
    auto serverID = *reinterpret_cast<int*>(g_gtarpclientside.getAddress(OFFSETS::GTARP_SERVERID));
    g_pServerSprite->m_pTexture = serverIcon[serverID > 2 || serverID < 0 ? 2 : serverID];
    plugin::patch::ReplaceFunction(g_gtarpclientside.getAddress(OFFSETS::GTARP_DRAWHUD), &drawServerIcon);

    /*
        Возвращает часы
    */
    patch::fill(g_dlSAMP.getAddress(OFFSETS::SAMP_ENABLECLOCK), 2, 0x90);

    /*
        Фиксим время на часах
    */
    plugin::patch::ReplaceFunctionCall(static_cast<uint32_t>(OFFSETS::GTASA_DRAWHUD_CLOCK_SPRINTF), &drawClockSprintfDetourFNC);

    /*
        Отключаем ебучую рамку
    */
    if (!g_pConfig->getConfig()->m_vehHud.m_bIsDrawHelpTablet)
    {
        patch::fill(g_gtarpclientside.getAddress(OFFSETS::GTARP_DRAWSPEEDOMETER_DRAWRAMKA), 0x1E, 0x90/*NOP*/);
    }

    /*
        Заменяем ебучий шрифт
    */
    if (g_pConfig->getConfig()->m_samp.m_bIsCustomFont)
    {
        g_pD3DXCreateFontDetour = new PLH::x86Detour(
            reinterpret_cast<PCHAR>(g_dlSAMP.getAddress(OFFSETS::SAMP_CREATEFONT)), reinterpret_cast<PCHAR>(&D3DXCreateFontDetourFNC), &g_ui64D3DXCreateFontJumpTrampline, g_dis
        );
        g_pD3DXCreateFontDetour->hook();
    }

    /*
        Делаем ID цвета my cum
    */
    plugin::patch::ReplaceFunctionCall(static_cast<uint32_t>(g_dlSAMP.getAddress(OFFSETS::SAMP_RENDERPLAYERTAG_SPRINTF)), &renderPlayerTagSprintfDetourFNC);

    g_pDrawLabelDetour = new PLH::x86Detour( 
        reinterpret_cast<PCHAR>(g_dlSAMP.getAddress(OFFSETS::SAMP_PLAYERTAG_DRAWLABEL)), reinterpret_cast<PCHAR>(&drawLabelDetourFNC), &g_ui64DrawLabelJumpTrampline, g_dis
    );
    g_pDrawLabelDetour->hook();

    /*
        Проверяем на обновления
    */
    g_pGameLoopDetour = new PLH::x86Detour(
        reinterpret_cast<PCHAR>(OFFSETS::GTASA_GAMELOOP), reinterpret_cast<PCHAR>(&gameLoopDetourFNC), &g_ui64GameLoopJumpTrampline, g_dis
    );
    g_pGameLoopDetour->hook();

    g_pLog->Log("All patching succesful!!");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        if (hModule != GetModuleHandleA("!000patchGTARPClientByTim4ukys.ASI")) {
            TerminateProcess(GetCurrentProcess(), -1);
        }

        g_pLog = new CLog(L"!000patchGTARPClientByTim4ukys.log");
        g_pConfig = new CConfig(L"!000patchGTARPClientByTim4ukys.json");
        g_pSAMP = new CSAMP();

        CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(MainThread), NULL, NULL, NULL);

        break;
    case DLL_PROCESS_DETACH:

        SAFE_UNHOOK(g_pDrawWantedDetour);
        SAFE_UNHOOK(g_pD3DXCreateFontDetour);
        SAFE_UNHOOK(g_pDrawLabelDetour);
        SAFE_UNHOOK(g_pGameLoopDetour);

        SAFE_DELETE(g_pSAMP);
        SAFE_DELETE(g_pServerSprite);
        SAFE_DELETE(g_pConfig);
        SAFE_DELETE(g_pLog);
        break;
    }
    return TRUE;
}