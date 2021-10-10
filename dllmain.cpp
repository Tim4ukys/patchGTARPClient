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

CLog* pLog = nullptr;
CConfig* pConfig = nullptr;

uint32_t getAddress(uint32_t offset)
{
    static uint32_t baseAddess{};
    if (!baseAddess) baseAddess = reinterpret_cast<uint32_t>(GetModuleHandleA("gtarp_clientside.asi"));
    return baseAddess + offset;
}

PLH::CapstoneDisassembler dis = { PLH::Mode::x86 };

uint64_t DrawWanted__JampTramp{};
PLH::x86Detour* DrawWanted__Detour = nullptr;
NOINLINE void DrawWanted__DetourFNC()
{
    auto pWanted = FindPlayerWanted(-1); 
    pWanted->m_nWantedLevel = *(DWORD*)getAddress(0xDD450);
    pWanted->m_nWantedLevelBeforeParole = *(DWORD*)getAddress(0xDD450);

    ((void(__cdecl*)())DrawWanted__JampTramp)(); // call original
}

int __cdecl sprintf__DetourFNC(char* buff, const char* f, ...)
{
    auto t = time(0);
    auto local_t = localtime(&t);
    auto r = sprintf(buff, "%02d:%02d", local_t->tm_hour, local_t->tm_min);

    return r;
}

CSprite2d* pServerSprite = nullptr;

int drawServerIcon()
{
    static auto conf = pConfig->getConfig();
    if (conf->m_serverIcon.m_state)
    {
        pServerSprite->Draw(SCREEN_COORD_LEFT(conf->m_serverIcon.m_x), SCREEN_COORD_TOP(conf->m_serverIcon.m_y), SCREEN_COORD(184), SCREEN_COORD(72), CRGBA(0xFF, 0xFF, 0xFF));
    }

    return NULL;
}

void MainThread()
{
    while (!getAddress(NULL)) Sleep(100);

    /*
        Выключает лок на подключение на стороние сервера.
    */
    patch::setRaw(getAddress(0x48F2), "\xEB\x17\x90", 3);

    /*
        Возрващает самповский худ 

        .text:0002B81D - start nop
        .text:0002B97A - end
        ---
        size = 0x168
    */
    patch::fill(getAddress(0x1B81D), 0x15D, 0x90);

    /*
        Исправляет уровень состояние розыска.
    */
    DrawWanted__Detour = new PLH::x86Detour(PCHAR(0x58D9A0), PCHAR(&DrawWanted__DetourFNC), &DrawWanted__JampTramp, dis);
    DrawWanted__Detour->hook();

    /*
        Фиксит положение иконки сервера
    */
    RwTexture** serverIcon = reinterpret_cast<RwTexture**>(getAddress(0xDD69C));
    while (!serverIcon[0] || !serverIcon[1] || !serverIcon[2]) Sleep(100);

    pServerSprite = new CSprite2d();
    pServerSprite->m_pTexture = serverIcon[*(int*)getAddress(0xCD990)];
    plugin::patch::ReplaceFunction(getAddress(0x1BBE0), &drawServerIcon);

    /*
        Возвращает часы
    */
    DWORD sampBasseAddress;
    do
    {
        sampBasseAddress = reinterpret_cast<DWORD>(GetModuleHandleA("samp.dll"));

    } while (!sampBasseAddress);

    patch::fill(sampBasseAddress + 0xA0D84, 2, 0x90);

    /* 
        Фиксим время на часах
    */
    plugin::patch::ReplaceFunctionCall(0x58EBAF, &sprintf__DetourFNC);

    pLog->Log("All patching succesful!!");
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

        pLog = new CLog(L"!000patchGTARPClientByTim4ukys.log");
        pConfig = new CConfig(L"!000patchGTARPClientByTim4ukys.json");

        CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(MainThread), NULL, NULL, NULL);

        break;
    case DLL_PROCESS_DETACH:

        SAFE_UNHOOK(DrawWanted__Detour);

        SAFE_DELETE(pServerSprite);
        SAFE_DELETE(pConfig);
        SAFE_DELETE(pLog);
        break;
    }
    return TRUE;
}

