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

const char SAMP_CMP[] = "E86D9A0A0083C41C85C0";
const char GTARP_CMP[] = "5E5DC20400CCCCCCCCCC";

#define CURRENT_VERSION "6.0.0"
#define GITHUB_URL      "github.com/Tim4ukys/patchGTARPClient"

// ----------------------------------------

#include "OldHUD.h"
#include "UnlockConect.h"
#include "CustomFont.h"
#include "WhiteID.h"
#include "ReplaceableTXD.h"
#include "DelCarTable.h"
#include "SortScreenshot.h"
#include "DisableSnowWindow.h"

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

#define PROCESS(a) {a::Process}
        std::function<void()> cock[]{PROCESS(OldHUD), PROCESS(UnlockConect), PROCESS(CustomFont), PROCESS(WhiteID), PROCESS(ReplaceableTXD),
                                     PROCESS(DelCarTable), PROCESS(SortScreenshot), PROCESS(DisableSnowWindow)};
        for (const auto& fnc : cock)
            fnc();
#undef PROCESS

        g_Log << "[loader]: end patching. Destroy 'LdrpDereferenceModule' hook.";
        g_pLdrpDereferenceModule->uninstallHook();
    }

    return origFnc((ldrrModuleDLL*)a1, a2);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: 
    {
        OSVERSIONINFOA vers;
        ZeroMemory(&vers, sizeof(OSVERSIONINFOA));
        vers.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

        if (GetVersionExA(&vers)) {
            g_Log.Write("[PC INFO]: Windows version: %s %u.%u build: %u | PlatformID - %u", vers.szCSDVersion, vers.dwMajorVersion,
                        vers.dwMinorVersion, vers.dwBuildNumber, vers.dwPlatformId);
        }

        // ------------

        auto handleNTDLL = GetModuleHandleA("ntdll.dll");
        auto nAddressLdrLoadDll = DWORD(GetProcAddress(handleNTDLL, "LdrLoadDll"));

        auto addr = patch::FindPattern("ntdll.dll", "\x8B\x4C\x24\x18\x8B\x54\x24\x1C\x8B\x41\x18\x89\x02\xE8", "x???x???x??xxx", nAddressLdrLoadDll);
        addr += 13;

        g_pLdrpDereferenceModule = new patch::callHook(addr);
        g_pLdrpDereferenceModule->installHook(&loadModule, false);
    }
        break;
    case DLL_PROCESS_DETACH:
        SAFE_DELETE(g_pLdrpDereferenceModule);
        g_Config.saveFile();
        break;
    }
    return TRUE;
}
