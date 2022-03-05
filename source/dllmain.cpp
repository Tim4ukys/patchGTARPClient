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
#include "snippets.h"
#include "Log.h"

//snippets::DynamicLibrary g_gtarpclient{ "gtarp_clientside.asi" };
Log g_Log("!000patchGTARPClientByTim4ukys.log");

patch::callHook *g_pLdrpDereferenceModule = nullptr;
PDWORD __fastcall loadModule(struct ldrrModuleDLL* a1, PVOID a2) {
    struct ldrrModuleDLL {
        uint32_t pad_0[6]; // +0h
        HANDLE   hModule; // +18h
        uint32_t pad_1[5]; // +1Ch
        wchar_t* pPluginName; // +30h
    }* _a1 = (ldrrModuleDLL*)a1;

    auto origFnc = reinterpret_cast<PDWORD(__fastcall*)(ldrrModuleDLL*, PVOID)>(g_pLdrpDereferenceModule->getOriginal());

    //g_Log.Write("hModule: %p | name: %s", DWORD(a1), ConvertWideToANSI(_a1->pPluginName).c_str());
    //MessageBoxA(NULL, "Stop", "patchTest", MB_OK | MB_ICONINFORMATION);
    if (!wcscmp(_a1->pPluginName, L"gtarp_clientside.asi"))
    {
        //g_Log << "[loader]: gtarp_clientside.asi - injected. Start hooking.";


        g_Log << "[loader]: end hooking. Destroy 'LdrpDereferenceModule' hook.";
        g_pLdrpDereferenceModule->uninstallHook();
    }

    return origFnc((ldrrModuleDLL*)a1, a2);
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
