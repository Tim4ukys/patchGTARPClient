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
#include "offsets.hpp"

D3D9Hook::D3D9Hook() {
    g_onInitSamp += [this]() {
        PVOID hookPrs = reinterpret_cast<void*>(getDeviceAddress(17));
        PVOID hookRes = reinterpret_cast<void*>(getDeviceAddress(16));

        present.m_pDetour = new PLH::x86Detour(UINT64(hookPrs), UINT64(&HookedPresent), &present.m_uiJump);
        reset.m_pDetour = new PLH::x86Detour(UINT64(hookRes), UINT64(&HookedReset), &reset.m_uiJump);

        present.m_pDetour->hook();
        reset.m_pDetour->hook();

        onInitDevice.call(*g_sampBase.getAddr<LPDIRECT3DDEVICE9*>(OFFSETS::SAMP::D3D9DEVICE));
    };
}

NOINLINE HRESULT __stdcall D3D9Hook::HookedPresent(IDirect3DDevice9* pDevice, const RECT* pSrcRect, const RECT* pDestRect, HWND hDestWindow, const RGNDATA* pDirtyRegion) {
    g_pD3D9Hook->onPresentEvent.call(pDevice, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);

    return ((HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*))g_pD3D9Hook->present.m_uiJump)
        (pDevice, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);
}

NOINLINE HRESULT __stdcall D3D9Hook::HookedReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
    g_pD3D9Hook->onLostDevice.call(pDevice, pPresentParams);

    HRESULT ret =
        ((HRESULT(__stdcall*)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS * pPresentParams)) g_pD3D9Hook->reset.m_uiJump)(pDevice, pPresentParams);

    g_pD3D9Hook->onResetDevice.call(pDevice, pPresentParams);

    return ret;
}

DWORD D3D9Hook::getDeviceAddress(int VTableIndex) {
    PDWORD VTable;
    *reinterpret_cast<DWORD*>(&VTable) = **g_sampBase.getAddr<DWORD**>(OFFSETS::SAMP::D3D9DEVICE);
    return VTable[VTableIndex];
};