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
#include "Updater.h"

const char SAMP_CMP[] = "E86D9A0A0083C41C85C0";
constexpr DWORD GTARP_TIMESTAMP = 0x63AB6949;

#define GITHUB_URL      "github.com/Tim4ukys/patchGTARPClient"

#define UPDATE_DELAY 12000

Updater g_Updater;

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
#include "CustomConnectScreen.h"

#include "Menu.h"

#define MK_MOD(a) \
    { std::string(#a), std::make_shared<a>() }
std::map<std::string, std::shared_ptr<MLoad>> g_modules{
    {
        MK_MOD(OldHUD),
        MK_MOD(UnlockConect),
        MK_MOD(CustomFont),
        MK_MOD(WhiteID),
        MK_MOD(DelCarTable),
        MK_MOD(SortScreenshot),
        MK_MOD(CustomHelp),
        MK_MOD(FastScreenshot),
        MK_MOD(TFRO),
        MK_MOD(DisableSnowWindow),
        MK_MOD(CustomConnectScreen)
    }
};
#undef MK_MOD

// ----------------------------------------

uint64_t                        g_ui64LdrLoadDllJumpTrampline;
std::unique_ptr<PLH::x86Detour> g_pLdrLoadDllDetour;
NOINLINE NTSTATUS __stdcall LdrLoadDllDetour(PWSTR searchPath, PULONG loadFlags, PUNICODE_STRING name, PVOID* baseAddress) {
    auto r = ((NTSTATUS(__stdcall*)(PWSTR, PULONG, PUNICODE_STRING, PVOID*))g_ui64LdrLoadDllJumpTrampline)
        (searchPath, loadFlags, name, baseAddress); // call original

    if (!wcscmp(name->Buffer, L"gtarp_clientside.asi"))
    {
        g_gtarpclientBase.updateBase((std::uintptr_t)*baseAddress);
        if (std::filesystem::exists(std::filesystem::path("updater_patchGTARPclient.exe")) && g_Updater.isHaveUpdate()) {
            //_spawnl(_P_OVERLAY, "updater_patchGTARPclient.exe", "updater_patchGTARPclient.exe", NULL);
            PROCESS_INFORMATION info;
            STARTUPINFOA        infoStart{sizeof(STARTUPINFOA)};
            CreateProcessA("updater_patchGTARPclient.exe", NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &infoStart, &info);
            // system("updater_patchGTARPclient.exe");
            TerminateProcess(GetCurrentProcess(), -1);
        }

        if (g_gtarpclientBase.getNTHeader()->FileHeader.TimeDateStamp != GTARP_TIMESTAMP) {
            MessageBoxW(
                NULL,
                L"ERROR: Эта версия плагина ещё не поддерживает эту версию клиента игры.\n\n"
                L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENT_VERSION L"\n"
                L"GitHub: " GITHUB_URL,
                L"!000patchGTARPClientByTim4ukys.ASI",
                MB_ICONERROR
            );
            g_Log.Write("gtarp_timestamp: %d", g_gtarpclientBase.getNTHeader()->FileHeader.TimeDateStamp);
            TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
        } else if (auto str = patch::getHEX(g_sampBase.getAddr<std::uintptr_t>(0xBABE), 10);
                   str != SAMP_CMP) {
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
        Menu::init();
        
        //auto&           modules = g_modules.getVars();
        std::atomic_int countRun = g_modules.size();
        for (auto& [key, point] : g_modules) {
            std::thread([&countRun](std::shared_ptr<MLoad>& p) {
                p->init();
                --countRun;
            }, std::ref(point)).detach();
        }
        while (countRun.load() > 0) {};

        g_Log << "[loader]: end patching.";
        //g_pLdrLoadDllDetour.release();
    } else if (!wcscmp(name->Buffer, L"SAMPFUNCS.asi")) {
        g_SF.updateBase((std::uintptr_t)*baseAddress);
    }

    return r;
}

// ------------------------------

FSignal<void()> g_onInitSamp;

uint64_t                        g_ui64GameLoopJumpTrampline;
std::unique_ptr<PLH::x86Detour> g_pGameLoopDetour;
NOINLINE void   gameLoopDetourFNC() {
    ((void (*)())g_ui64GameLoopJumpTrampline)(); // call original

    static bool s_bIsInit{}, s_bIsInitCocks{}, s_bIsInitRakHook{};
    if (s_bIsInit || !g_pSAMP->isSAMPInit())
        return;

    if (!s_bIsInitCocks) {
        g_onInitSamp.call();
        s_bIsInitCocks = true;
    }

    if (!s_bIsInitRakHook && rakhook::initialize()) {
        //StringCompressor::AddReference();
        s_bIsInitRakHook = true;
    }

    if (static auto s_timer = snippets::Timer<UPDATE_DELAY>();
        s_timer)
    {
        if (g_Updater.isHaveUpdate())
        {
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {990000}ВНИМАНИЕ: {FF9900}Вышло обновление!");
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Пожалуйста, обновите плагин!");
            g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Сайт: {990000}" GITHUB_URL "{FF9900}!");
        }
        g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Все самые свежие новости об обновлениях в нашем tg канале!");
        g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {FF9900}Ссылка: {990000}https://t.me/+LVGCHEsDZEhmY2My");
        s_bIsInit = true;
    }
}

// ------------------------------
// onSampInit

FSignal<void()> g_onInitAudio;

uint64_t                        g_uiSampInit;
std::unique_ptr<PLH::x86Detour> g_pSampInit;
NOINLINE int __fastcall initSAMPDetour(PVOID pthis, PVOID trash, char* a2, int a3, const char* a4, int a5) {
    auto r = FNC_CAST(initSAMPDetour, g_uiSampInit)(pthis, trash, a2, a3, a4, a5);
    
    static bool s_bIsInit{};
    if (s_bIsInit)
        return r;

    //BASS_Init(-1, 44100, 0, 0, nullptr);
    g_onInitAudio.call();
    s_bIsInit = true;
    return r;
}

// ------------------------------

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: 
    {
        g_DLLModule = hModule;
        g_pD3D9Hook = new D3D9Hook();
        g_pSAMP = new SAMP();

        //snippets::WinProcHeader::Init();
        plugin::Events::initGameEvent += []() {
            snippets::WinProcHeader::Init();
        };

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

        g_pSampInit = std::make_unique<PLH::x86Detour>(g_sampBase.getAddr<UINT64>(0xB5F0),
                                                       UINT64(&initSAMPDetour),
                                                       &g_uiSampInit);
        g_pSampInit->hook();
    }
        break;
    case DLL_PROCESS_DETACH:
        Menu::remove();
        SAFE_DELETE(g_pD3D9Hook);
        SAFE_DELETE(g_pSAMP);
        g_Config.saveFile();
        BASS_Free();
        break;
    }
    return TRUE;
}
