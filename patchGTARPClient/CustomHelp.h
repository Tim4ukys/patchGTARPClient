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
#pragma once
class CustomHelp : public MLoad {
    std::uint8_t m_oldByte[5];

    D3DXIMAGE_INFO     m_ImageInfo;
    LPDIRECT3DTEXTURE9 m_pTexture;
    LPD3DXSPRITE       m_pSprite;
    UINT               m_uiWindowSize[2];

    decltype(D3DXMatrixScaling)* fncMatrixScaling{};

    void turnOn() override;
    void turnOff() override;

public:
    void init() override;
    ~CustomHelp() override;
};
