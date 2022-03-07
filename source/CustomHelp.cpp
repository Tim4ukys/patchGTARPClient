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

/* Этот файл заменяет окно помощи(на F1) на более стильное окно */

D3DXIMAGE_INFO     g_ImageInfo;
LPDIRECT3DTEXTURE9 g_pTexture;
LPD3DXSPRITE g_pSprite;
//RECT               g_windowRect;
DWORD g_dwWindowSize[2];

// ---------------------------

bool    stateRenderHelp = false;
WNDPROC m_pWindowProc;
LRESULT __stdcall WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_KEYUP:
    case WM_SYSKEYUP:
        switch (wParam) {
        case VK_F1:
            stateRenderHelp = false;
            break;
        }
        break;
    case WM_SIZING:
        //LPRECT(lParam)
        //memcpy(&g_windowRect, PVOID(lParam), sizeof(RECT));
        g_dwWindowSize[0] = LPRECT(lParam)->right - LPRECT(lParam)->left;
        g_dwWindowSize[1] = LPRECT(lParam)->bottom - LPRECT(lParam)->top;
        break;
    }
    return CallWindowProcA(m_pWindowProc, hWnd, msg, wParam, lParam);
}

void showHelpDialogHook() {
    stateRenderHelp = true;
}

// ---------------------------

void CustomHelp::Process() {
    if (!g_Config["samp"]["isCustomF1"].get<bool>())
        return;

    g_pD3D9Hook->onInitDevice += [](LPDIRECT3DDEVICE9 pDevice) {
        D3DXGetImageInfoFromResourceA(GetModuleHandleA("!000patchGTARPClientByTim4ukys.ASI"), MAKEINTRESOURCEA(IDR_RCDATA1), &g_ImageInfo);
        D3DXCreateTextureFromResourceExA(pDevice, GetModuleHandleA("!000patchGTARPClientByTim4ukys.ASI"), MAKEINTRESOURCEA(IDR_RCDATA1),
                                         g_ImageInfo.Width, g_ImageInfo.Height, g_ImageInfo.MipLevels, NULL, D3DFMT_A8B8G8R8,
                                         D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0xFF000000, NULL, NULL, &g_pTexture);
        D3DXCreateSprite(pDevice, &g_pSprite);

        RECT r;
        GetClientRect(**(HWND**)0xC17054, &r);
        g_dwWindowSize[0] = r.right - r.left;
        g_dwWindowSize[1] = r.bottom - r.top;

        m_pWindowProc = (WNDPROC)SetWindowLongW(**(HWND**)0xC17054, GWL_WNDPROC, (LONG)WndProcHandler);
        plugin::patch::ReplaceFunction(g_sampBase.getAddress(0x6B3C0), &showHelpDialogHook);
    };
    g_pD3D9Hook->onLostDevice += [](LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
        g_pSprite->OnLostDevice();
    };
    g_pD3D9Hook->onResetDevice += [](LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
        g_pSprite->OnResetDevice();
    };
    g_pD3D9Hook->onPresentEvent += [](IDirect3DDevice9* pDevice, const RECT* pSrcRect, const RECT* pDestRect,
                                      HWND hDestWindow, const RGNDATA* pDirtyRegion) {
        if (!stateRenderHelp) return;

        if (SUCCEEDED(g_pSprite->Begin(D3DXSPRITE_ALPHABLEND))) {
            D3DXMATRIX matWH;
            D3DXMatrixScaling(&matWH, FLOAT(g_dwWindowSize[0]) / FLOAT(g_ImageInfo.Width),
                              FLOAT(g_dwWindowSize[1]) / FLOAT(g_ImageInfo.Height), 0.0f);
            g_pSprite->SetTransform(&matWH);
            g_pSprite->Draw(g_pTexture, 0, 0, 0, -1);
            g_pSprite->End();
        }
    };
}