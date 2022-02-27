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
#include "loader.h"
#include "snippets.h"

loader g_pLoader;
//snippets::DynamicLibrary g_gtarpclient{ "gtarp_clientside.asi" };

patch::callHook *g_pLdrpDereferenceModule = nullptr;
PDWORD __fastcall loadModule(struct ldrrModuleDLL* a1, PVOID a2) {
    struct ldrrModuleDLL {
        uint32_t pad_0[6];
        HANDLE   hModule;
    };

    for (const auto &fnc : g_pLoader) {
        fnc();
    }
    
    g_pLdrpDereferenceModule->getOriginal<PDWORD(__fastcall*)(ldrrModuleDLL*, PVOID)>()((ldrrModuleDLL*)a1, a2);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: 
    {
        auto handleNTDLL = GetModuleHandleA("ntdll.dll");
        auto nAddressLdrLoadDll = DWORD(GetProcAddress(handleNTDLL, "LdrLoadDll"));

        auto addr = patch::FindPattern("ntdll.dll", "\x8B\x4C\x24\x18\x8B\x54\x24\x1C\x8B\x41\x18\x89\x02\xE8", "x???x???x??xxx", nAddressLdrLoadDll);
        addr += 13;

        g_pLdrpDereferenceModule = new patch::callHook(addr);
        g_pLdrpDereferenceModule->installHook(&loadModule, false);
    }
        break;
    case DLL_PROCESS_DETACH:
        delete g_pLdrpDereferenceModule;
        break;
    }
    return TRUE;
}
