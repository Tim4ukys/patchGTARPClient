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

const std::string SAMP_CMP{ "E86D9A0A0083C41C85C0" };
const std::string GTARPCLIENTSIDE_CMP{ "CCCC558BEC568D71908B" };

#define UPDATE_DELAY 12000
#define CURRENTVERSIONA "v2.1"
#define CURRENTVERSION L"v2.1"
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
    static tm* s_pLocalTima = nullptr;
    static int8_t timeOffset{};
    if (!s_pLocalTima)
    {
        auto t = time(0);
        s_pLocalTima = localtime(&t);

        if (g_pConfig->getConfig()->m_clock.m_bFixTimeZone)
        {
            json j = json::parse(Client::downloadStringFromURL(false, "worldtimeapi.org", "/api/ip"));
            TimeZone tmz{ j["utc_offset"].get<std::string>() };
            timeOffset = tmz.getOffset("+03:00");
        }
    }

    int32_t hour{ s_pLocalTima->tm_hour };

    if (g_pConfig->getConfig()->m_clock.m_bFixTimeZone)
    {
        hour += timeOffset;
        hour += hour < 0 ? 24 : (hour > 23 ? -24 : 0);
    }

    return sprintf(buff, "%02d:%02d", hour, s_pLocalTima->tm_min);
}

uint64_t g_ui64D3DXCreateFontJumpTrampline;
PLH::x86Detour* g_pD3DXCreateFontDetour = nullptr;
NOINLINE int WINAPI D3DXCreateFontDetourFNC(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11/*fontFaceName*/, int a12, int a13, int a14, int a15)
{
    return ((int(WINAPI*)(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15))g_ui64D3DXCreateFontJumpTrampline)
        (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, int(g_pConfig->getConfig()->m_samp.m_FontFaceName.c_str()), a12, a13, a14, a15);
}

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

uint64_t g_ui64LoadLibraryAHooked_Detour;
PLH::x86Detour* g_pLoadLibraryAHookedDetour = nullptr;
NOINLINE HMODULE WINAPI loadLibraryHookedDetourFNC(const char* fileName)
{
    if (!strcmp(fileName, "SAMPFUNCS.asi"))
        return LoadLibraryA(fileName);
    
    return ((HMODULE(WINAPI*)(const char* a1))g_ui64LoadLibraryAHooked_Detour)(fileName); // call original
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
    while (!g_gtarpclientside.getAddress()) {

    }

    if (auto currentCMP = patch::getHEX(g_gtarpclientside.getAddress(0xBABE), 10); currentCMP != GTARPCLIENTSIDE_CMP) {
        MessageBoxW(
            NULL,
            L"ERROR: Эта версия плагина ещё не поддерживает эту версию клиента игры.\n\n"
            L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENTVERSION L"\n"
            L"GitHub: " GITHUBURL,
            L"!000patchGTARPClientByTim4ukys.ASI",
            MB_ICONERROR
        );
        g_pLog->Log("gtarp_cmp: %s", currentCMP.c_str());
        TerminateProcess(GetCurrentProcess(), -1);
    }
    else if (currentCMP = patch::getHEX(g_dlSAMP.getAddress(0xBABE), 10); currentCMP != SAMP_CMP) {
        MessageBoxW(
            NULL,
            L"ERROR: Эта версия плагина ещё не поддерживает эту версию SAMP.\n\n"
            L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENTVERSION L"\n"
            L"GitHub: " GITHUBURL,
            L"!000patchGTARPClientByTim4ukys.ASI",
            MB_ICONERROR
        );
        g_pLog->Log("samp_cmp: %s", currentCMP.c_str());
        TerminateProcess(GetCurrentProcess(), -1);
    }
    else {
        g_pLog->Log("gtarp_clientside & samp: Version supported!!!");
    }

    /*
        Вырубаем блок на SAMPFUNCS | Work in progress
    */
    //patch::fill(g_gtarpclientside.getAddress(0xE2EA), 0x5, 0x90);
    //patch::fill(g_gtarpclientside.getAddress(0xE2F2), 0x65, 0x90);
    
    //patch::fill(g_gtarpclientside.getAddress(0xE2F8), 15u, 0x90);
    //patch::fill(g_gtarpclientside.getAddress(0xE30D), 5u, 0x90);
    //g_pLoadLibraryAHookedDetour = new PLH::x86Detour(
        //reinterpret_cast<PCHAR>(g_gtarpclientside.getAddress(0x2E290)), reinterpret_cast<PCHAR>(&loadLibraryHookedDetourFNC), &g_ui64LoadLibraryAHooked_Detour, g_dis
    //);
    //g_pLoadLibraryAHookedDetour->hook();
    

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
    if (g_pConfig->getConfig()->m_samp.m_bIsWhiteID)
    {
        const char* formatPlayerTag = "%s {FFFFFF}(%d)";
        static char s_arrBuffPlayerTag[128 + 8];

        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_RENDERPLAYERTAG_FORMAT), reinterpret_cast<uint32_t>(formatPlayerTag));
        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_RENDERPLAYERTAG_PUSHBUFF_1), reinterpret_cast<uint32_t>(&s_arrBuffPlayerTag));

        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_RENDERPLAYERTAG_PUSHBUFF_2), reinterpret_cast<uint32_t>(&s_arrBuffPlayerTag));
    }

    /*
        Проверяем на обновления
    */
    g_pGameLoopDetour = new PLH::x86Detour(
        reinterpret_cast<PCHAR>(OFFSETS::GTASA_GAMELOOP), reinterpret_cast<PCHAR>(&gameLoopDetourFNC), &g_ui64GameLoopJumpTrampline, g_dis
    );
    g_pGameLoopDetour->hook();

    /*
        Парсим скриншоты по датам
    */
    if (g_pConfig->getConfig()->m_samp.m_bIsSortingScreenshots)
    {
        static char s_fullPathScreenshot[22] = "%s"; // '%s\\screens\\xx-xx-xxxx'
        static char s_pathScreenshot[35]; // '\\screens\\xx-xx-xxxx\\sa-mp-%03i.png'

        auto t = time(0);
        auto pLocalTime = localtime(&t);

        sprintf_s(s_pathScreenshot, "\\screens\\%02d-%02d-%d", pLocalTime->tm_mday, pLocalTime->tm_mon, pLocalTime->tm_year + 1900);

        strcat(s_fullPathScreenshot, s_pathScreenshot);
        strcat(s_pathScreenshot, "\\sa-mp-%03i.png");

        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_FORMATPATHSCREENSHOT), reinterpret_cast<uint32_t>(s_pathScreenshot));
        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_FORMATFULLPATHSCREENSHOT), reinterpret_cast<uint32_t>(s_fullPathScreenshot));
    }

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

        /*
            В след обновлении перейду с CreateThread на хук KernelBase. В данной ситуации многопоточность лучше не юзать(unsafe).
        */
        CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(MainThread), NULL, NULL, NULL);

        break;
    case DLL_PROCESS_DETACH:

        SAFE_UNHOOK(g_pDrawWantedDetour);
        SAFE_UNHOOK(g_pD3DXCreateFontDetour);
        SAFE_UNHOOK(g_pGameLoopDetour);

        SAFE_DELETE(g_pSAMP);
        SAFE_DELETE(g_pServerSprite);
        SAFE_DELETE(g_pConfig);
        SAFE_DELETE(g_pLog);
        break;
    }
    return TRUE;
}