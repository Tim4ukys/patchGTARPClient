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
#include "D3D9Hook.h"

D3D9Hook::D3D9Hook() {
    m_pInitDevice = new patch::callHook(0x7F681A);
    m_pInitDevice->installHook(&D3D9Hook::initDeviceHook);
}
D3D9Hook::~D3D9Hook() {
    delete m_pInitDevice;
}

NOINLINE HRESULT __stdcall D3D9Hook::HookedPresent(IDirect3DDevice9* pDevice, const RECT* pSrcRect, const RECT* pDestRect, HWND hDestWindow, const RGNDATA* pDirtyRegion) {
    if (pDevice) {
        for (const auto& fnc : g_pD3D9Hook->onPresentEvent) {
            fnc(pDevice, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);
        }
    }
    return ((HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*))g_pD3D9Hook->present.m_uiJump)
        (pDevice, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);
}

NOINLINE HRESULT __stdcall D3D9Hook::HookedReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
    if (!pDevice)
        return ((HRESULT(__stdcall*)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS * pPresentParams)) g_pD3D9Hook->reset.m_uiJump)(pDevice, pPresentParams);

    for (const auto& fnc : g_pD3D9Hook->onLostDevice) {
        fnc(pDevice, pPresentParams);
    }

    HRESULT ret =
        ((HRESULT(__stdcall*)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS * pPresentParams)) g_pD3D9Hook->reset.m_uiJump)(pDevice, pPresentParams);

    for (const auto& fnc : g_pD3D9Hook->onResetDevice) {
        fnc(pDevice, pPresentParams);
    }

    return ret;
}

void D3D9Hook::initHooks() {
    PVOID hookPrs = reinterpret_cast<void*>(getDeviceAddress(17));
    PVOID hookRes = reinterpret_cast<void*>(getDeviceAddress(16));

    present.m_pDetour = new PLH::x86Detour(UINT64(hookPrs), UINT64(&HookedPresent), &present.m_uiJump);
    reset.m_pDetour = new PLH::x86Detour(UINT64(hookRes), UINT64(&HookedReset), &reset.m_uiJump);

    present.m_pDetour->hook();
    reset.m_pDetour->hook();

    for (const auto& fnc : onInitDevice) {
        fnc((LPDIRECT3DDEVICE9)RwD3D9GetCurrentD3DDevice());
    }
}
void D3D9Hook::initDeviceHook() {
    snippets::WinProcHeader::Init();
    g_pD3D9Hook->initHooks();
    return ((void (*)())g_pD3D9Hook->m_pInitDevice->getOriginal())();
}

DWORD D3D9Hook::findDevice(DWORD len) {
    DWORD dwObjBase = NULL;
    char  infoBuf[MAX_PATH]{};
    GetSystemDirectoryA(infoBuf, MAX_PATH);
    strcat_s(infoBuf, MAX_PATH, "\\d3d9.dll");
    dwObjBase = reinterpret_cast<DWORD>(LoadLibraryA(infoBuf));
    while (dwObjBase++ < dwObjBase + len) {
        if ((*reinterpret_cast<WORD*>(dwObjBase + 0x00)) == 0x06C7 &&
            (*reinterpret_cast<WORD*>(dwObjBase + 0x06)) == 0x8689 &&
            (*reinterpret_cast<WORD*>(dwObjBase + 0x0C)) == 0x8689) {
            dwObjBase += 2;
            break;
        }
    }
    return dwObjBase;
}
DWORD D3D9Hook::getDeviceAddress(int VTableIndex) {
    PDWORD VTable;
    *reinterpret_cast<DWORD*>(&VTable) = *reinterpret_cast<DWORD*>(findDevice(0x128000));
    return VTable[VTableIndex];
};