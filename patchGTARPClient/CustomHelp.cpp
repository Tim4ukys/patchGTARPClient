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
#include "CustomHelp.h"
#include "resource.h"
#include "offsets.hpp"

/* Этот файл заменяет окно помощи(на F1) на более стильное окно */

// ---------------------------

static bool s_bStateRenderHelp = false;

static std::shared_ptr<PLH::x86Detour> g_wndProcHangler;
static WNDPROC m_pWindowProc;
static LRESULT __stdcall WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_KEYUP:
    case WM_SYSKEYUP:
        switch (wParam) {
        case VK_F1:
            s_bStateRenderHelp = false;
            break;
        }
        break;
    }
    return m_pWindowProc(hWnd, msg, wParam, lParam);
}

void showHelpDialogHook() {
    s_bStateRenderHelp = true;
}

// ---------------------------

void CustomHelp::turnOn() {
    //std::uint8_t bytes[5]{0xE9};
    //*reinterpret_cast<std::uint32_t*>(bytes + 1) = (std::uint32_t)&showHelpDialogHook;
    //patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::SHOWHELP), bytes);
    plugin::patch::ReplaceFunction(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::SHOWHELP), &showHelpDialogHook);
}

void CustomHelp::turnOff() {
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::SHOWHELP), m_oldByte);
}

void CustomHelp::init() {
    fncMatrixScaling = g_D3DX9_25.getFnc<decltype(D3DXMatrixScaling)>("D3DXMatrixScaling");
    g_pD3D9Hook->onInitDevice += [this](LPDIRECT3DDEVICE9 pDevice) {
        g_D3DX9_25.callFnc<decltype(D3DXGetImageInfoFromResourceA)>("D3DXGetImageInfoFromResourceA",
                                                                    g_DLLModule,
                                                                    MAKEINTRESOURCEA(IDR_RCDATA1),
                                                                    &m_ImageInfo);
        g_D3DX9_25.callFnc<decltype(D3DXCreateTextureFromResourceExA)>("D3DXCreateTextureFromResourceExA",
                                                                       pDevice, g_DLLModule,
                                                                       MAKEINTRESOURCEA(IDR_RCDATA1),
                                                                       m_ImageInfo.Width, m_ImageInfo.Height,
                                                                       m_ImageInfo.MipLevels, NULL,
                                                                       D3DFMT_A8B8G8R8, D3DPOOL_MANAGED,
                                                                       D3DX_DEFAULT, D3DX_DEFAULT,
                                                                       0xFF000000, nullptr, nullptr,
                                                                       &m_pTexture);
        g_D3DX9_25.callFnc<decltype(D3DXCreateSprite)>("D3DXCreateSprite",
                                                       pDevice, &m_pSprite);

        g_wndProcHangler = snippets::WinProcHeader::regWinProc(WndProcHandler, &m_pWindowProc);
    };
    g_pD3D9Hook->onLostDevice += [this](LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
        m_pSprite->OnLostDevice();
    };
    g_pD3D9Hook->onResetDevice += [this](LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
        memcpy(m_uiWindowSize, pPresentParams, sizeof(UINT) * 2);
        g_Log.Write("[CustomHelp]: <DeviceReset> g_uiWindowSize: %u.%u", m_uiWindowSize[0], m_uiWindowSize[1]);
        m_pSprite->OnResetDevice();
    };
    g_pD3D9Hook->onPresentEvent += [this](IDirect3DDevice9* pDevice, const RECT* pSrcRect, const RECT* pDestRect,
                                          HWND hDestWindow, const RGNDATA* pDirtyRegion) {
        if (!s_bStateRenderHelp)
            return;

        if (SUCCEEDED(m_pSprite->Begin(D3DXSPRITE_ALPHABLEND))) {
            D3DXMATRIX matWH;
            fncMatrixScaling(&matWH, FLOAT(m_uiWindowSize[0]) / FLOAT(m_ImageInfo.Width),
                             FLOAT(m_uiWindowSize[1]) / FLOAT(m_ImageInfo.Height), 0.0f);
            m_pSprite->SetTransform(&matWH);
            m_pSprite->Draw(m_pTexture, 0, 0, 0, -1);
            m_pSprite->End();
        }
    };

    // ----

    patch::getBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::SHOWHELP), m_oldByte);
    
    // ----

    if (m_bState = g_Config["samp"]["isCustomF1"].get<bool>())
        turnOn();
}

CustomHelp::~CustomHelp() {
    if (m_bState)
        turnOff();
    g_wndProcHangler.reset();
    SAFE_RELEASE(m_pSprite);
    SAFE_RELEASE(m_pTexture);
}