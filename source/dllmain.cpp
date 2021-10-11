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
#define CURRENTVERSION L"v1.1"
#define GITHUBURL L"github.com/Tim4ukys/patchGTARPClient"

CLog* g_pLog        = nullptr;
CConfig* g_pConfig  = nullptr;

DynamicLibrary g_gtarpclientside{ "gtarp_clientside.asi" };

PLH::CapstoneDisassembler g_dis = { PLH::Mode::x86 };

///////////////////////////////////////////////////////////////////

uint64_t g_ui64DrawWantedJumpTrampline;
PLH::x86Detour* g_pDrawWantedDetour = nullptr;
NOINLINE void drawWantedDetourFNC()
{
    FindPlayerWanted(-1)->SetWantedLevel( *reinterpret_cast<DWORD*>(g_gtarpclientside.getAddress(0xDD450)) );

    ((void(__cdecl*)())g_ui64DrawWantedJumpTrampline)(); // call original
}

int sprintfDetourFNC(char* buff, const char* f, ...)
{
    auto t = time(0);
    auto local_t = localtime(&t);
    auto r = sprintf(buff, "%02d:%02d", local_t->tm_hour, local_t->tm_min);

    return r;
}

///////////////////////////////////////////////////////////////////

CSprite2d* g_pServerSprite = nullptr;

int drawServerIcon()
{
    auto conf = g_pConfig->getConfig();
    if (conf->m_serverIcon.m_state)
    {
        g_pServerSprite->Draw(
            SCREEN_COORD_LEFT(conf->m_serverIcon.m_x), SCREEN_COORD_TOP(conf->m_serverIcon.m_y), SCREEN_COORD(184), SCREEN_COORD(72), CRGBA(0xFF, 0xFF, 0xFF)
        );
    }

    return NULL;
}

void MainThread()
{
    while (!g_gtarpclientside.getAddress()) Sleep(10);

    DynamicLibrary dlSAMP{ "samp.dll" };

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
    else if (patch::getHEX(dlSAMP.getAddress(0xBABE), 10) != SAMP_CMP) {
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
    patch::fill(dlSAMP.getAddress(OFFSETS::SAMP_ENABLECLOCK), 2, 0x90);

    /*
        Фиксим время на часах
    */
    plugin::patch::ReplaceFunctionCall(static_cast<uint32_t>(OFFSETS::GTASA_DRAWHUD_CLOCK_SPRINTF), &sprintfDetourFNC);

    g_pLog->Log("All patching succesful!!");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        if (hModule != GetModuleHandleA(xorstr("!000patchGTARPClientByTim4ukys.ASI"))) {
            TerminateProcess(GetCurrentProcess(), -1);
        }

        g_pLog = new CLog(L"!000patchGTARPClientByTim4ukys.log");
        g_pConfig = new CConfig(L"!000patchGTARPClientByTim4ukys.json");

        CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(MainThread), NULL, NULL, NULL);

        break;
    case DLL_PROCESS_DETACH:

        SAFE_UNHOOK(g_pDrawWantedDetour);

        SAFE_DELETE(g_pServerSprite);
        SAFE_DELETE(g_pConfig);
        SAFE_DELETE(g_pLog);
        break;
    }
    return TRUE;
}