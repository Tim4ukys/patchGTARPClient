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
#include "offsets.hpp"
#include <process.h>

const char SAMP_CMP[] = "E86D9A0A0083C41C85C0";
const char GTARP_CMP[] = "5E5DC20400CCCCCCCCCC";

#define CURRENT_VERSION "6.1.1"
#define GITHUB_URL      "github.com/Tim4ukys/patchGTARPClient"

#define UPDATE_DELAY 12000

// ----------------------------------------

#include "OldHUD.h"
#include "UnlockConect.h"
#include "CustomFont.h"
#include "WhiteID.h"
#include "ReplaceableTXD.h"
#include "DelCarTable.h"
#include "SortScreenshot.h"
#include "DisableSnowWindow.h"
#include "CustomHelp.h"
#include "FastScreenshot.h"

// ----------------------------------------

patch::callHook *g_pLdrpDereferenceModule = nullptr;
PDWORD __fastcall loadModule(struct ldrrModuleDLL* a1, PVOID a2) {
    struct ldrrModuleDLL {
        uint32_t pad_0[6]; // +0h
        HANDLE   hModule; // +18h
        uint32_t pad_1[5]; // +1Ch
        wchar_t* pPluginName; // +30h
    }* _a1 = (ldrrModuleDLL*)a1;

    static auto origFnc = reinterpret_cast<PDWORD(__fastcall*)(ldrrModuleDLL*, PVOID)>(g_pLdrpDereferenceModule->getOriginal());

    if (!wcscmp(_a1->pPluginName, L"gtarp_clientside.asi"))
    {
        if (std::filesystem::exists(std::filesystem::path("updater_patchGTARPclient.exe"))) {
            auto j = nlohmann::json::parse(client::downloadStringFromURL(R"(https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json)"));
            auto vers = j["vers"].get<std::string>();
            if (strcmp(vers.c_str(), CURRENT_VERSION) != NULL) {
                //_spawnl(_P_OVERLAY, "updater_patchGTARPclient.exe", "updater_patchGTARPclient.exe", NULL);
                PROCESS_INFORMATION info;
                STARTUPINFOA         infoStart{sizeof(STARTUPINFO)};
                CreateProcessA("updater_patchGTARPclient.exe", NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &infoStart, &info);
                //system("updater_patchGTARPclient.exe");
                TerminateProcess(GetCurrentProcess(), -1);
            }
        }

        if (auto cmp = patch::getHEX(g_gtarpclientBase.getAddress(0xBABE), 10U); cmp != GTARP_CMP) {
            MessageBoxW(
                NULL,
                L"ERROR: Эта версия плагина ещё не поддерживает эту версию клиента игры.\n\n"
                L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENT_VERSION L"\n"
                L"GitHub: " GITHUB_URL,
                L"!000patchGTARPClientByTim4ukys.ASI",
                MB_ICONERROR
            );
            g_Log.Write("gtarp_cmp: %s", cmp.c_str());
            TerminateProcess(GetCurrentProcess(), -1);
        } 
        else if (cmp = patch::getHEX(g_sampBase.getAddress(0xBABE), 10); cmp != SAMP_CMP) {
            MessageBoxW(
                NULL,
                L"ERROR: Эта версия плагина ещё не поддерживает эту версию SAMP.\n\n"
                L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENT_VERSION L"\n"
                L"GitHub: " GITHUB_URL,
                L"!000patchGTARPClientByTim4ukys.ASI",
                MB_ICONERROR);
            g_Log.Write("samp_cmp: %s", cmp.c_str());
            TerminateProcess(GetCurrentProcess(), -1);
        } 
        else
            g_Log << R"(Version "gtarp_clientside.asi" and "samp.dll" supported!!)";

        g_Log << "[loader]: gtarp_clientside.asi - injected. Start patching.";

#define PROCESS(a) {std::thread(a::Process)}
        //std::function<void()> cock[]{PROCESS(OldHUD), PROCESS(UnlockConect), PROCESS(CustomFont), PROCESS(WhiteID), PROCESS(ReplaceableTXD),
                                     //PROCESS(DelCarTable), PROCESS(SortScreenshot), PROCESS(DisableSnowWindow), PROCESS(CustomHelp)};
        //for (const auto& fnc : cock)
            //fnc();
        std::thread cock[]{PROCESS(OldHUD), PROCESS(UnlockConect), PROCESS(CustomFont), PROCESS(WhiteID), PROCESS(ReplaceableTXD),
                           PROCESS(DelCarTable), PROCESS(SortScreenshot), PROCESS(DisableSnowWindow), PROCESS(CustomHelp),
                           PROCESS(FastScreenshot)};
        for (auto& thr : cock)
            thr.join();
#undef PROCESS

        g_Log << "[loader]: end patching. Destroy 'LdrpDereferenceModule' hook.";
        g_pLdrpDereferenceModule->uninstallHook();
    }

    return origFnc((ldrrModuleDLL*)a1, a2);
}

// ------------------------------

uint64_t        g_ui64GameLoopJumpTrampline;
PLH::x86Detour* g_pGameLoopDetour = nullptr;
NOINLINE void   gameLoopDetourFNC() {
    ((void (*)())g_ui64GameLoopJumpTrampline)(); // call original

    static bool s_bIsInit = false;
    if (s_bIsInit || !g_pSAMP->isSAMPInit())
        return;
    
    static auto s_oldTime = GetTickCount64();
    if (GetTickCount64() - s_oldTime > UPDATE_DELAY)
    {
        auto j = nlohmann::json::parse(client::downloadStringFromURL(R"(https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json)"));
        auto vers = j["vers"].get<std::string>();
        if (strcmp(vers.c_str(), CURRENT_VERSION) != NULL)
        {
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {990000}ВНИМАНИЕ: {FF9900}Вышло обновление!");
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Пожалуйста, обновите плагин!");
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Сайт: {990000}" GITHUB_URL "{FF9900}!");
        }
        g_Log.Write("[UPDATE]: Last version: %s", vers.c_str());
        s_bIsInit = true;
    }
}

// ------------------------------
// Audio engine

FSignal<void()> g_initAudioTracks;

uint64_t g_uiOrigAudioInit;
PLH::x86Detour *g_pAudioInitDetour;
int __fastcall initSAMPDetour(PVOID pthis, PVOID trash, char* a2, int a3, const char* a4, int a5) {
    BASS_Init(-1, 44100, 0, 0, nullptr);

    for (auto& fnc : g_initAudioTracks)
        fnc();

    return FNC_CAST(initSAMPDetour, g_uiOrigAudioInit)(pthis, trash, a2, a3, a4, a5);
}

// ------------------------------

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: 
    {
        g_pD3D9Hook = new D3D9Hook();

        // ------------

        OSVERSIONINFOA vers;
        ZeroMemory(&vers, sizeof(OSVERSIONINFOA));
        vers.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

        if (GetVersionExA(&vers)) {
            g_Log.Write("[PC INFO]: Windows version: %s %u.%u build: %u | PlatformID - %u", vers.szCSDVersion, vers.dwMajorVersion,
                        vers.dwMinorVersion, vers.dwBuildNumber, vers.dwPlatformId);
        }

        // ------------
        // Проверка на обновления
        g_pSAMP = new SAMP();
        PLH::CapstoneDisassembler dis(PLH::Mode::x86);
        g_pGameLoopDetour = new PLH::x86Detour(
            reinterpret_cast<PCHAR>(OFFSETS::GTA_SA::GAMELOOP), reinterpret_cast<PCHAR>(&gameLoopDetourFNC), &g_ui64GameLoopJumpTrampline, dis);
        g_pGameLoopDetour->hook();

        // ------------

        auto handleNTDLL = GetModuleHandleA("ntdll.dll");
        auto nAddressLdrLoadDll = DWORD(GetProcAddress(handleNTDLL, "LdrLoadDll"));

        auto addr = patch::FindPattern("ntdll.dll", "\x8B\x4C\x24\x18\x8B\x54\x24\x1C\x8B\x41\x18\x89\x02\xE8", "x???x???x??xxx", nAddressLdrLoadDll);
        addr += 13;

        g_pLdrpDereferenceModule = new patch::callHook(addr);
        g_pLdrpDereferenceModule->installHook(&loadModule, false);

        g_pAudioInitDetour = new PLH::x86Detour(PCHAR(g_sampBase.getAddress(0xB5F0)), PCHAR(&initSAMPDetour), &g_uiOrigAudioInit, dis);
        g_pAudioInitDetour->hook();
    }
        break;
    case DLL_PROCESS_DETACH:
        SAFE_DELETE(g_pD3D9Hook);
        SAFE_DELETE(g_pLdrpDereferenceModule);
        SAFE_DELETE(g_pSAMP);
        SAFE_DELETE(g_pGameLoopDetour);
        g_Config.saveFile();
        BASS_Free();
        break;
    }
    return TRUE;
}
