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
#include <SubAuth.h>

FSignal<void()> g_onDetachPlugin;

const char SAMP_CMP[] = "E86D9A0A0083C41C85C0";
//const char GTARP_CMP[] = "4C00750883C8FFE9A302";
constexpr DWORD GTARP_TIMESTAMP = 0x63AB6949;

#define DECLARATION_VERSION(v_maj, v_min, v_patch) \
    const int CURRENT_VERSION_MAJ = v_maj; \
    const int CURRENT_VERSION_MIN = v_min; \
    const int CURRENT_VERSION_PAT = v_patch;

DECLARATION_VERSION(10, 0, 0)
#define CURRENT_VERSION "10.0.0"
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
#include "DisableSnowWindow.h"

#include "Menu.h"

// ----------------------------------------

uint64_t                        g_ui64LdrLoadDllJumpTrampline;
std::unique_ptr<PLH::x86Detour> g_pLdrLoadDllDetour;
NTSTATUS __stdcall LdrLoadDllDetour(PWSTR searchPath, PULONG loadFlags, PUNICODE_STRING name, PVOID* baseAddress) {
    auto r = ((NTSTATUS(*)(PWSTR, PULONG, PUNICODE_STRING, PVOID*))g_ui64LdrLoadDllJumpTrampline)
        (searchPath, loadFlags, name, baseAddress); // call original

    if (!wcscmp(name->Buffer, L"gtarp_clientside.asi"))
    {
        if (std::filesystem::exists(std::filesystem::path("updater_patchGTARPclient.exe"))) {
            auto        j = nlohmann::json::parse(client::downloadStringFromURL(R"(https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json)"));
            std::string s;
            j["vers"].get_to(s);
            auto [vMaj, vMin, vPatch] = snippets::versionParse(s);
            g_Log.Write("Git version: vMaj: %d | vMid: %d | vPatch: %d", vMaj, vMin, vPatch);

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

        auto base = g_gtarpclientBase.getAddress();
        IMAGE_NT_HEADERS* ntheader = reinterpret_cast<IMAGE_NT_HEADERS*>(base + reinterpret_cast<IMAGE_DOS_HEADER*>(base)->e_lfanew);

        if (ntheader->FileHeader.TimeDateStamp != GTARP_TIMESTAMP) {
            MessageBoxW(
                NULL,
                L"ERROR: Эта версия плагина ещё не поддерживает эту версию клиента игры.\n\n"
                L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENT_VERSION L"\n"
                L"GitHub: " GITHUB_URL,
                L"!000patchGTARPClientByTim4ukys.ASI",
                MB_ICONERROR
            );
            g_Log.Write("gtarp_timestamp: %d", ntheader->FileHeader.TimeDateStamp);
            TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
        } else if (auto str = std::move(patch::getHEX(g_sampBase.getAddress(0xBABE), 10));
                   str == SAMP_CMP) {
            MessageBoxW(
                NULL,
                L"ERROR: Эта версия плагина ещё не поддерживает эту версию SAMP.\n\n"
                L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENT_VERSION L"\n"
                L"GitHub: " GITHUB_URL,
                L"!000patchGTARPClientByTim4ukys.ASI",
                MB_ICONERROR);
            g_Log.Write("samp_cmp: %s", str.c_str());
            TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
        } 
        else
            g_Log << R"(Version "gtarp_clientside.asi" and "samp.dll" supported!!)";

        g_Log << "[loader]: gtarp_clientside.asi - injected. Start patching.";

#define PROCESS(a) {std::thread(a::Process)}
        std::thread cock[]{PROCESS(OldHUD), PROCESS(UnlockConect), PROCESS(CustomFont), PROCESS(WhiteID), /*PROCESS(ReplaceableTXD),*/
                           PROCESS(DelCarTable), PROCESS(SortScreenshot), PROCESS(CustomHelp), PROCESS(FastScreenshot),
                           PROCESS(Menu), PROCESS(TFRO), PROCESS(DisableSnowWindow)};
        for (auto& thr : cock)
            thr.join();
#undef PROCESS

        g_Log << "[loader]: end patching. Destroy 'LdrLoadDll' hook.";
        g_pLdrLoadDllDetour->unHook();
    }

    return r;
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
        auto        j = nlohmann::json::parse(client::downloadStringFromURL(R"(https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json)"));
        std::string s;
        j["vers"].get_to(s);
        auto [vMaj, vMin, vPatch] = snippets::versionParse(s);
        if (CHECK_VERSION(vMaj, vMin, vPatch, CURRENT_VERSION_MAJ, CURRENT_VERSION_MIN, CURRENT_VERSION_PAT))
        {
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {990000}ВНИМАНИЕ: {FF9900}Вышло обновление!");
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Пожалуйста, обновите плагин!");
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Сайт: {990000}" GITHUB_URL "{FF9900}!");
        }
        g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Все самые свежие новости о разработке в нашем tg канале!");
        g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Ссылка: {990000}https://t.me/+LVGCHEsDZEhmY2My");
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

        g_pSAMP = new SAMP();
        g_pGameLoopDetour = std::make_unique<PLH::x86Detour>(UINT64(OFFSETS::GTA_SA::GAMELOOP),
                                                             UINT64(&gameLoopDetourFNC),
                                                             &g_ui64GameLoopJumpTrampline);
        g_pGameLoopDetour->hook();

        // ------------
        auto pNTDLLHandle = GetModuleHandleA("ntdll.dll");
        if (!pNTDLLHandle) {
            g_Log.Write("pNTDLLHandle == nullptr! Abort");
            abort();
        }
        auto nAddressLdrLoadDll = UINT64(GetProcAddress(pNTDLLHandle, "LdrLoadDll"));

        g_pLdrLoadDllDetour = std::make_unique<PLH::x86Detour>(nAddressLdrLoadDll,
                                                               UINT64(&LdrLoadDllDetour),
                                                               &g_ui64LdrLoadDllJumpTrampline);
        g_pLdrLoadDllDetour->hook();

        // ------------

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
        SAFE_DELETE(g_pSAMP);
        g_Config.saveFile();
        BASS_Free();
        break;
    }
    return TRUE;
}
