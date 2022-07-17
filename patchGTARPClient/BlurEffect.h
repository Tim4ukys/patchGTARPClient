/****************************************************
*                                                   *
*    Developer: Tim4ukys, CyberMor                  *
*                                                   *
*    email: tim4ukys.dev@yandex.ru                  *
*           cyber.mor.2020@gmail.ru                 *
*                                                   *
*    vk: vk.com/tim4ukys                            *
*                                                   *
*    License: GNU GPLv3                             *
*                                                   *
****************************************************/

#pragma once

#include <memory>

#include <d3d9.h>
#include <d3dx9.h>

#include "Resource.h"

class BlurEffect {

    BlurEffect() = delete;
    BlurEffect(const BlurEffect&) = delete;
    BlurEffect(BlurEffect&&) = delete;
    BlurEffect& operator=(const BlurEffect&) = delete;
    BlurEffect& operator=(BlurEffect&&) = delete;

public:
    explicit BlurEffect(IDirect3DDevice9* pDevice);

    ~BlurEffect() noexcept;

public:
    void Render(const RECT& r, float level) const noexcept;

    void onLostDevice();
    void onResetDevice(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams);

private:
    void Draw(const RECT &r) const noexcept;

private:
    IDirect3DDevice9* pDevice;

    IDirect3DSurface9* pDeviceBackBuffer{nullptr};

    UINT backBufferWidth{0};
    UINT backBufferHeight{0};

    ID3DXEffect*                 pEffect{nullptr};
    IDirect3DVertexDeclaration9* pVertexDeclaration{nullptr};

    IDirect3DTexture9* pBackBufferTexture{nullptr};
    IDirect3DSurface9* pBackBufferSurface{nullptr};
    IDirect3DTexture9* pTempBufferTexture{nullptr};
    IDirect3DSurface9* pTempBufferSurface{nullptr};
    IDirect3DTexture9* pFrontBufferTexture{nullptr};
    IDirect3DSurface9* pFrontBufferSurface{nullptr};
};