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

ID3DXFont* g_pChatFont = nullptr;

void chatFontReset() {
    SAFE_RELEASE(g_pChatFont);

    int iFontSize = ((int (*)())g_sampBase.getAddress(0xC5B20))();
    int iFontWeight = ((int (*)())g_sampBase.getAddress(0xC5BD0))();

    g_Log.Write("[CustomFont]: iFontSize: %d | iFontWeight: %d", iFontSize, iFontWeight);

    D3DXCreateFontA(LPDIRECT3DDEVICE9(RwD3D9GetCurrentD3DDevice()), iFontSize, 0, iFontWeight, 1, FALSE, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, g_Config["samp"]["fontFaceName"].get<std::string>().c_str(), 
                    &g_pChatFont);
}

void chatFontResetDevice() {
    if (g_pChatFont)
        g_pChatFont->OnResetDevice();
}

void chatFontLostDevice() {
    if (g_pChatFont)
        g_pChatFont->OnLostDevice();
}

//patch::callHook g_createFontHook;
std::unique_ptr<patch::callHook> g_createFontHook;
std::string                      g_fontFaceName;
void WINAPI Direct9CreateFontA__Detour(int a1, int a2, int a3, int a4, int a5, int a6, int a7, 
                                      int a8, int a9, int a10, const char* fontFaceName, int a12, 
                                      int a13, int a14, int a15) {
    void(WINAPI * orig)(int, int, int, int, int, int, int,
                       int, int, int, const char*,
                       int, int, int, int) = reinterpret_cast<decltype(orig)>(g_createFontHook->getOriginal());
    //g_Log.Write("[CustomFont]: Create font \"%s\"", fontFace.c_str());
    return orig(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, g_fontFaceName.c_str(), a12, a13, a14, a15);
}

void CustomFont::Process() {
    if (g_Config["samp"]["isCustomFont"].get<bool>()) {
        g_fontFaceName = g_Config["samp"]["fontFaceName"].get<std::string>();

        patch__setJump(g_sampBase.getAddress(0x6AA7F), uint32_t(&chatFontResetDevice), 5U, TRUE);
        patch__setJump(g_sampBase.getAddress(0x6AA3F), uint32_t(&chatFontLostDevice), 5U, TRUE);
        patch__setJump(g_sampBase.getAddress(0x6B36C), uint32_t(&chatFontReset), 5U, TRUE);

        ///

        char raw[6]{'\x8B', '\x35', 0, 0, 0, 0}; // 8B 35 ? ? ? ?

        *(DWORD*)(DWORD(raw + 2)) = DWORD(&g_pChatFont); // mov esi, [pFont]

        patch__fill(g_sampBase.getAddress(0x66D06), 3U, 0x90);
        patch__setRawThroughJump(g_sampBase.getAddress(0x66D06), raw, 6, 5U, TRUE);

        ///

        g_createFontHook = std::make_unique<patch::callHook>(g_sampBase.getAddress(0x6B230));
        g_createFontHook->installHook(&Direct9CreateFontA__Detour);
    }
}