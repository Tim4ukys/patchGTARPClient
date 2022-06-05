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
#include "pch.h"
#include "BlurEffect.h"

BlurEffect::BlurEffect(IDirect3DDevice9* const pDevice)
    : pDevice(pDevice) {
    assert(pDevice != nullptr);

    ID3DXBuffer* pErrorBuffer{};
    D3DXCreateEffectFromResourceW(pDevice, GetModuleHandleA("!000patchGTARPClientByTim4ukys.ASI"), MAKEINTRESOURCE(IDR_BLUR1),
                                  NULL, NULL, D3DXFX_DONOTSAVESTATE | D3DXFX_NOT_CLONEABLE | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY,
                                  NULL, &this->pEffect, &pErrorBuffer);

    const D3DVERTEXELEMENT9 vertexElements[] =
    {
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0},
        {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        D3DDECL_END()
    };

    pDevice->CreateVertexDeclaration(vertexElements, &this->pVertexDeclaration);
}
BlurEffect::~BlurEffect() noexcept {
    this->pDeviceBackBuffer->Release();
    this->pEffect->Release();
    this->pVertexDeclaration->Release();
    this->pBackBufferSurface->Release();
    this->pBackBufferTexture->Release();
    this->pTempBufferSurface->Release();
    this->pTempBufferTexture->Release();
    this->pFrontBufferSurface->Release();
    this->pFrontBufferTexture->Release();
}

void BlurEffect::onLostDevice() {
    SAFE_RELEASE(pDeviceBackBuffer);

    SAFE_RELEASE(pBackBufferSurface);
    SAFE_RELEASE(pTempBufferSurface);
    SAFE_RELEASE(pFrontBufferSurface);

    SAFE_RELEASE(pBackBufferTexture);
    SAFE_RELEASE(pTempBufferTexture);
    SAFE_RELEASE(pFrontBufferTexture);
}
void BlurEffect::onResetDevice(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
    pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &this->pDeviceBackBuffer);
    
    HRESULT hr;
    if (pPresentParams != nullptr) {
        this->backBufferHeight = pPresentParams->BackBufferHeight;
        this->backBufferWidth = pPresentParams->BackBufferWidth;
    } else {
        D3DSURFACE_DESC backBufferDesc;
        hr = pDeviceBackBuffer->GetDesc(&backBufferDesc);
        if (SUCCEEDED(hr)) {
            this->backBufferWidth = backBufferDesc.Width;
            this->backBufferHeight = backBufferDesc.Height;
        }
    }
    const float iResolution[] = {static_cast<const float>(this->backBufferWidth), static_cast<const float>(this->backBufferHeight)};

    this->pEffect->SetFloatArray(static_cast<D3DXHANDLE>("iResolution"), iResolution, ARRAYSIZE(iResolution));
    this->pEffect->SetFloat(static_cast<D3DXHANDLE>("iLevel"), 0.f);

    hr = pDevice->CreateTexture(this->backBufferWidth, this->backBufferHeight, 1, D3DUSAGE_RENDERTARGET,
                                D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &this->pBackBufferTexture, nullptr);
    if (SUCCEEDED(hr)) {
        this->pBackBufferTexture->GetSurfaceLevel(0, &this->pBackBufferSurface);
    } else {
        this->pBackBufferSurface = nullptr;
        this->pBackBufferTexture = nullptr;
    }

    hr = pDevice->CreateTexture(this->backBufferWidth, this->backBufferHeight, 1, D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP,
                                D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &this->pTempBufferTexture, nullptr);
    if (SUCCEEDED(hr)) {
        this->pTempBufferTexture->GetSurfaceLevel(0, &this->pTempBufferSurface);
    } else {
        this->pTempBufferSurface = nullptr;
        this->pTempBufferTexture = nullptr;
    }

    hr = pDevice->CreateTexture(this->backBufferWidth, this->backBufferHeight, 1, D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP,
                                D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &this->pFrontBufferTexture, nullptr);
    if (SUCCEEDED(hr)) {
        this->pFrontBufferTexture->GetSurfaceLevel(0, &this->pFrontBufferSurface);
    } else {
        this->pFrontBufferSurface = nullptr;
        this->pFrontBufferTexture = nullptr;
    }
}

void BlurEffect::Render(const RECT& r, const float level) const noexcept {
    if (SUCCEEDED(this->pDevice->StretchRect(this->pDeviceBackBuffer, NULL, this->pBackBufferSurface, NULL, D3DTEXF_POINT)) &&
        SUCCEEDED(this->pEffect->SetFloat(static_cast<D3DXHANDLE>("iLevel"), std::clamp(level, 0.f, 100.f))) &&
        SUCCEEDED(this->pEffect->Begin(nullptr, NULL))) {
        if (SUCCEEDED(this->pEffect->SetTexture(static_cast<D3DXHANDLE>("iFrameTexture"), this->pBackBufferTexture)) &&
            SUCCEEDED(this->pDevice->SetRenderTarget(0, this->pTempBufferSurface)) &&
            SUCCEEDED(this->pEffect->BeginPass(0))) {
            this->Draw(r);
            this->pEffect->EndPass();
        }

        if (SUCCEEDED(this->pEffect->SetTexture(static_cast<D3DXHANDLE>("iFrameTexture"), this->pTempBufferTexture)) &&
            SUCCEEDED(this->pDevice->SetRenderTarget(0, this->pFrontBufferSurface)) &&
            SUCCEEDED(this->pEffect->BeginPass(1))) {
            this->Draw(r);
            this->pEffect->EndPass();
        }

        if (SUCCEEDED(this->pDevice->SetRenderTarget(0, this->pDeviceBackBuffer)) &&
            SUCCEEDED(this->pEffect->SetTexture(static_cast<D3DXHANDLE>("iBackBuffer"), this->pBackBufferTexture)) &&
            SUCCEEDED(this->pEffect->SetTexture(static_cast<D3DXHANDLE>("iFrameTexture"), this->pFrontBufferTexture)) &&
            SUCCEEDED(this->pEffect->BeginPass(2))) {
            this->Draw(r);
            this->pEffect->EndPass();
        }

        this->pEffect->End();
    }
}

void BlurEffect::Draw(const RECT& r) const noexcept {
    struct EffectVertex {
        float x, y, z;
        float tx0, tx1;
    };

    if (SUCCEEDED(this->pDevice->SetVertexDeclaration(this->pVertexDeclaration))) {
        const float screenWidth = static_cast<const float>(this->backBufferWidth);
        const float screenHeight = static_cast<const float>(this->backBufferHeight);

        float uv[4];
        uv[0] = r.left > 0 ? 1 / (screenWidth / r.left) : 0;
        uv[1] = r.top > 0 ? 1 / (screenHeight / r.top) : 0;
        uv[2] = r.right > 0 ? 1 / (screenWidth / r.right) : 0;
        uv[3] = r.bottom > 0 ? 1 / (screenHeight / r.bottom) : 0;

        const EffectVertex quad[] = {
            {float(r.left), float(r.top), 0.5f, uv[0], uv[1]},
            {float(r.right), float(r.top), 0.5f, uv[2], uv[1]},
            {float(r.left), float(r.bottom), 0.5f, uv[0], uv[3]},
            {float(r.right), float(r.bottom), 0.5f, uv[2], uv[3]}
        };

        _rwD3D9DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(*quad));
    }
}