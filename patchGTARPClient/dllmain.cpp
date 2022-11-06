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

FSignal<void()> g_onDetachPlugin;

const char SAMP_CMP[] = "E86D9A0A0083C41C85C0";
const char GTARP_CMP[] = "4C00750883C8FFE9A302";

#define DECLARATION_VERSION(v_maj, v_min, v_patch) \
    const int CURRENT_VERSION_MAJ = v_maj; \
    const int CURRENT_VERSION_MIN = v_min; \
    const int CURRENT_VERSION_PAT = v_patch;

DECLARATION_VERSION(9, 0, 0)
#define CURRENT_VERSION "9.0.0"
const char* g_szCurrentVersion = CURRENT_VERSION;
#define CHECK_VERSION(NEW_MAJ, NEW_MIN, NEW_PATCH, old_maj, old_min, old_patch) \
    (NEW_MAJ > old_maj ||  \
    (NEW_MAJ == old_maj && NEW_MIN > old_min) || \
    (NEW_MAJ == old_maj && NEW_MIN == old_min && NEW_PATCH > old_patch))

#define GITHUB_URL      "github.com/Tim4ukys/patchGTARPClient"

#define UPDATE_DELAY 12000

// ----------------------------------------

#include "OldHUD.h"
#include "UnlockConect.h"
#include "CustomFont.h"
#include "WhiteID.h"
//#include "ReplaceableTXD.h"
#include "DelCarTable.h"
#include "SortScreenshot.h"
#include "CustomHelp.h"
#include "FastScreenshot.h"
#include "tfro.h"

#include "Menu.h"

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
            auto [vMaj, vMin, vPatch] = snippets::versionParse(j["vers"].get<std::string>());

            if (CHECK_VERSION(vMaj, vMin, vPatch, CURRENT_VERSION_MAJ, CURRENT_VERSION_MIN, CURRENT_VERSION_PAT))
            {
                //_spawnl(_P_OVERLAY, "updater_patchGTARPclient.exe", "updater_patchGTARPclient.exe", NULL);
                PROCESS_INFORMATION info;
                STARTUPINFOA         infoStart{sizeof(STARTUPINFOA)};
                CreateProcessA("updater_patchGTARPclient.exe", NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &infoStart, &info);
                //system("updater_patchGTARPclient.exe");
                TerminateProcess(GetCurrentProcess(), -1);
            }
        }

        char hexBuff[10 * 2 + 1]{};

        if (patch__getHEX(g_gtarpclientBase.getAddress(0xBABE), hexBuff, 10U); 
            strcmp(hexBuff, GTARP_CMP)) {
            MessageBoxW(
                NULL,
                L"ERROR: Эта версия плагина ещё не поддерживает эту версию клиента игры.\n\n"
                L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENT_VERSION L"\n"
                L"GitHub: " GITHUB_URL,
                L"!000patchGTARPClientByTim4ukys.ASI",
                MB_ICONERROR
            );
            g_Log.Write("gtarp_cmp: %s", hexBuff);
            TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
        } 
        else if (patch__getHEX(g_sampBase.getAddress(0xBABE), hexBuff, 10); 
            strcmp(hexBuff, SAMP_CMP)) {
            MessageBoxW(
                NULL,
                L"ERROR: Эта версия плагина ещё не поддерживает эту версию SAMP.\n\n"
                L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENT_VERSION L"\n"
                L"GitHub: " GITHUB_URL,
                L"!000patchGTARPClientByTim4ukys.ASI",
                MB_ICONERROR);
            g_Log.Write("samp_cmp: %s", hexBuff);
            TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
        } 
        else
            g_Log << R"(Version "gtarp_clientside.asi" and "samp.dll" supported!!)";

        g_Log << "[loader]: gtarp_clientside.asi - injected. Start patching.";

#define PROCESS(a) {std::thread(a::Process)}
        std::thread cock[]{PROCESS(OldHUD), PROCESS(UnlockConect), PROCESS(CustomFont), PROCESS(WhiteID), /*PROCESS(ReplaceableTXD),*/
                           PROCESS(DelCarTable), PROCESS(SortScreenshot), PROCESS(CustomHelp), PROCESS(FastScreenshot),
                           PROCESS(Menu), PROCESS(TFRO)};
        for (auto& thr : cock)
            thr.join();
#undef PROCESS

        g_Log << "[loader]: end patching. Destroy 'LdrpDereferenceModule' hook.";
        g_pLdrpDereferenceModule->uninstallHook();
    }

    return origFnc((ldrrModuleDLL*)a1, a2);
}

// ------------------------------

FSignal<void()> g_onInitSamp;

uint64_t                        g_ui64GameLoopJumpTrampline;
std::unique_ptr<PLH::x86Detour> g_pGameLoopDetour;
NOINLINE void   gameLoopDetourFNC() {
    ((void (*)())g_ui64GameLoopJumpTrampline)(); // call original

    static bool s_bIsInit = false;
    if (s_bIsInit || !g_pSAMP->isSAMPInit())
        return;

    static bool s_bIsRunSignals = false;
    if (!s_bIsRunSignals) {
        for (auto& fnc : g_onInitSamp) {
            fnc();
        }
        s_bIsRunSignals = true;
    }
    
    static auto s_oldTime = GetTickCount64();
    if (GetTickCount64() - s_oldTime > UPDATE_DELAY)
    {
        auto j = nlohmann::json::parse(client::downloadStringFromURL(R"(https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json)"));
        auto [vMaj, vMin, vPatch] = snippets::versionParse(j["vers"].get<std::string>());
        if (CHECK_VERSION(vMaj, vMin, vPatch, CURRENT_VERSION_MAJ, CURRENT_VERSION_MIN, CURRENT_VERSION_PAT))
        {
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {990000}ВНИМАНИЕ: {FF9900}Вышло обновление!");
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Пожалуйста, обновите плагин!");
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Сайт: {990000}" GITHUB_URL "{FF9900}!");
        }
        /*g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}У нас появился {990000}Telegram-Bot{FF9900}!");
        g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Подробнее: {990000}https://t.me/patchClient_bot");*/
        g_Log.Write("[UPDATE]: Last version: %d.%d.%d", vMaj, vMin, vPatch);
        s_bIsInit = true;
    }
}

// ------------------------------
// Audio engine

FSignal<void()> g_initAudioTracks;

uint64_t                        g_uiOrigAudioInit;
std::unique_ptr<PLH::x86Detour> g_pAudioInitDetour;
int __fastcall initSAMPDetour(PVOID pthis, PVOID trash, char* a2, int a3, const char* a4, int a5) {
    BASS_Init(-1, 44100, 0, 0, nullptr);

    size_t&&                 size_a_cocks = g_initAudioTracks.size();
    std::vector<std::thread> cocks;
    cocks.resize(size_a_cocks);
    for (size_t i{}; i < size_a_cocks; ++i) {
        cocks[i] = std::thread(g_initAudioTracks[i]);
    }

    for (auto& thr : cocks)
        thr.join();


    return FNC_CAST(initSAMPDetour, g_uiOrigAudioInit)(pthis, trash, a2, a3, a4, a5);
}

// ------------------------------

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: 
    {
        g_pD3D9Hook = new D3D9Hook();

        // ------------

        if (GetModuleHandleA("!000patchGTARPClientByTim4ukys.ASI") != hModule) {
            wchar_t moduleName[MAX_PATH]{};
            GetModuleFileNameW(hModule, moduleName, ARRAYSIZE(moduleName));
            wchar_t* name = wcsrchr(moduleName, L'\\') + 1;
            MessageBoxW(NULL,
                        L"Пожайлуста, переименуйте плагин на: !000patchGTARPClientByTim4ukys.ASI",
                        name,
                        MB_ICONERROR | MB_OK);
            exit(EXIT_FAILURE);
        }

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
        g_pGameLoopDetour = std::make_unique<PLH::x86Detour>(UINT64(OFFSETS::GTA_SA::GAMELOOP),
                                                             UINT64(&gameLoopDetourFNC),
                                                             &g_ui64GameLoopJumpTrampline);
        g_pGameLoopDetour->hook();

        // ------------

        auto handleNTDLL = GetModuleHandleA("ntdll.dll");
        auto nAddressLdrLoadDll = DWORD(GetProcAddress(handleNTDLL, "LdrLoadDll"));

        auto addr = patch__FindPattern("ntdll.dll",
                                       "\x8B\x4C\x24\x18\x8B\x54\x24\x1C\x8B\x41\x18\x89\x02\xE8",
                                       "x???x???x??xxx",
                                       nAddressLdrLoadDll);
        addr += 13;

        g_pLdrpDereferenceModule = new patch::callHook(addr);
        g_pLdrpDereferenceModule->installHook(&loadModule, false);

        g_pAudioInitDetour = std::make_unique<PLH::x86Detour>(UINT64(g_sampBase.getAddress(0xB5F0)),
                                                              UINT64(&initSAMPDetour),
                                                              &g_uiOrigAudioInit);
        g_pAudioInitDetour->hook();
    }
        break;
    case DLL_PROCESS_DETACH:
        for (const auto& fnc : g_onDetachPlugin) {
            fnc();
        }

        SAFE_DELETE(g_pD3D9Hook);
        SAFE_DELETE(g_pLdrpDereferenceModule);
        SAFE_DELETE(g_pSAMP);
        g_Config.saveFile();
        BASS_Free();
        break;
    }
    return TRUE;
}
