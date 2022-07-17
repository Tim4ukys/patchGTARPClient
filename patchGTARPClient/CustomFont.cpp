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
#include "CustomFont.h"

/**
* Этот файл заменяет стандартный шрифт в чате на любой другой
*/

std::string CustomFont::s_fontFaceName;

CustomFont::stFont* CustomFont::s_pFontCE;
ID3DXFont*          CustomFont::s_pShadowFont;

uint64_t                        CustomFont::s_ui64OnChatFontReset;
std::unique_ptr<PLH::x86Detour> CustomFont::s_onChatFontReset;
void __fastcall CustomFont::onChatFontReset(PVOID p_this, PVOID trash) {
    ((void(__fastcall*)(PVOID, PVOID))s_ui64OnChatFontReset)(p_this, trash);

    SAFE_RELEASE(s_pFontCE->m_pFont);
    SAFE_RELEASE(s_pShadowFont);

    const int iFontSize = ((int (*)())g_sampBase.getAddress(0xC5B20))();
    const int iFontWeight = ((int (*)())g_sampBase.getAddress(0xC5BD0))();

    g_Log.Write("[CustomFont]: iFontSize: %d | iFontWeight: %d", iFontSize, iFontWeight);

    D3DXCreateFontA(LPDIRECT3DDEVICE9(RwD3D9GetCurrentD3DDevice()), iFontSize, 0, iFontWeight, 1,
                    FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                    DEFAULT_PITCH, s_fontFaceName.c_str(), &s_pFontCE->m_pFont);

    D3DXCreateFontA(LPDIRECT3DDEVICE9(RwD3D9GetCurrentD3DDevice()), iFontSize, 0, iFontWeight, 1,
                    FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                    DEFAULT_PITCH, s_fontFaceName.c_str(), &s_pShadowFont);
}

uint64_t                        CustomFont::s_ui64OnLostDevice;
std::unique_ptr<PLH::x86Detour> CustomFont::s_onLostDevice;
void __fastcall CustomFont::onLostDevice(PVOID p_this, PVOID trash) {
    ((void(__fastcall*)(PVOID, PVOID))s_ui64OnLostDevice)(p_this, trash);

    s_pFontCE->m_pFont->OnLostDevice();
    s_pShadowFont->OnLostDevice();
}

uint64_t                        CustomFont::s_ui64OnResetDevice;
std::unique_ptr<PLH::x86Detour> CustomFont::s_onResetDevice;
void __fastcall CustomFont::onResetDevice(PVOID p_this, PVOID trash) {
    ((void(__fastcall*)(PVOID, PVOID))s_ui64OnResetDevice)(p_this, trash);

    s_pFontCE->m_pFont->OnResetDevice();
    s_pShadowFont->OnResetDevice();
}

void CustomFont::Process() {
    if (!g_Config["samp"]["isCustomFont"].get<bool>())
        return;

    s_fontFaceName = g_Config["samp"]["fontFaceName"].get<std::string>();

    s_pFontCE = (stFont*) new uint8_t[sizeof(stFont)];
    ZeroMemory(s_pFontCE, sizeof(stFont));
    // Ставим VTable из SA-MP. В нём находятся функции из сампа, которые делают такие секас цвета
    *(DWORD*)s_pFontCE = g_sampBase.getAddress(0xEA3B8);

    ///

    s_onChatFontReset = std::make_unique<PLH::x86Detour>(UINT64(g_sampBase.getAddress(0x6B170)),
                                                         UINT64(&onChatFontReset),
                                                         &s_ui64OnChatFontReset);
    s_onChatFontReset->hook();

    s_onLostDevice = std::make_unique<PLH::x86Detour>(UINT64(g_sampBase.getAddress(0x6AA10)),
                                                      UINT64(&onLostDevice),
                                                      &s_ui64OnLostDevice);
    s_onLostDevice->hook();

    s_onResetDevice = std::make_unique<PLH::x86Detour>(UINT64(g_sampBase.getAddress(0x6AA50)),
                                                       UINT64(&onResetDevice),
                                                       &s_ui64OnResetDevice);
    s_onResetDevice->hook();

    ///

    char raw[]{
        '\xbe', 0, 0, 0, 0, // mov esi, &pFontCE
        '\x8B', '\x06', // mov eax, [esi]
        '\x89', '\x44', '\x24', '\x10', // mov [esp+214h+pFontCE], eax
        '\xb8', 0, 0, 0, 0, // mov eax, &pFont
        '\x8b', '\x30' // mov esi, [eax]
    };

    *(stFont***)(raw + 1) = &s_pFontCE;
    *(ID3DXFont***)(raw + 12) = &s_pShadowFont;
    patch__setRawThroughJump(g_sampBase.getAddress(0x66D06), raw, ARRAYSIZE(raw), 9, FALSE);
}