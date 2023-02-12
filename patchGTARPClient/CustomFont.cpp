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

void CustomFont__CallResetFont() {
    g_sampBase.getAddr<void(__thiscall*)(PVOID)>(0x6B170)(*g_sampBase.getAddr<PVOID*>(0x26E8E4)); // CFonts::Reset
    g_sampBase.getAddr<void(__thiscall*)(SAMP::ChatInfo*)>(0x67A50)(g_pSAMP->getChat());          // CChat::OnResetDevice
}

CustomFont::stFont* CustomFont::s_pFontCE;
ID3DXFont*          CustomFont::s_pShadowFont;

uint64_t                        CustomFont::s_ui64OnChatFontReset;
std::unique_ptr<PLH::x86Detour> CustomFont::s_onChatFontReset;
void __fastcall CustomFont::onChatFontReset(PVOID p_this, PVOID trash) {
    ((void(__fastcall*)(PVOID, PVOID))s_ui64OnChatFontReset)(p_this, trash);

    SAFE_RELEASE(s_pFontCE->m_pFont);
    SAFE_RELEASE(s_pShadowFont);

    const int iFontSize = g_sampBase.getAddr<int (*)()>(0xC5B20)();
    const int iFontWeight = g_Config["samp"]["customFontWeight"].get<int>();
    const auto faceName = g_Config["samp"]["fontFaceName"].get<std::string>();

    D3DXCreateFontA(LPDIRECT3DDEVICE9(RwD3D9GetCurrentD3DDevice()), iFontSize, 0, iFontWeight, 1,
                    FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                    DEFAULT_PITCH, faceName.c_str(), &s_pFontCE->m_pFont);

    D3DXCreateFontA(LPDIRECT3DDEVICE9(RwD3D9GetCurrentD3DDevice()), iFontSize, 0, iFontWeight, 1,
                    FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                    DEFAULT_PITCH, faceName.c_str(), &s_pShadowFont);
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

void CustomFont::turnOn() {
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(0x66D06), m_oldByte[1]);

    /**
     * mov ecx, &s_pFontCE
     */
    char rwGetSizeLineChat[]{
        '\x90', '\xB9', 0, 0, 0, 0};
    *(stFont***)(rwGetSizeLineChat + 2) = &s_pFontCE;
    patch::setRaw(g_sampBase.getAddr<std::uintptr_t>(0x6737F), rwGetSizeLineChat); // in CChat::render

    patch::setRaw(g_sampBase.getAddr<std::uintptr_t>(0x669D4), rwGetSizeLineChat); // in recalcSize
    /**
     * nop
     * mov eax, &s_pFontCE
     */
    rwGetSizeLineChat[0] = '\x90';
    rwGetSizeLineChat[1] = '\xB8';
    patch::setRaw(g_sampBase.getAddr<std::uintptr_t>(0x669B7), rwGetSizeLineChat); // in recalcSize
}

void CustomFont::turnOff() {
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(0x66D06), m_oldByte[0]);

    patch::setRaw(g_sampBase.getAddr<std::uintptr_t>(0x6737F), "\x8B\x8E\xA2\x63\x00\x00"); // in CChat::render
    patch::setRaw(g_sampBase.getAddr<std::uintptr_t>(0x669D4), "\x8B\x8E\xA2\x63\x00\x00"); // in recalcSize
    patch::setRaw(g_sampBase.getAddr<std::uintptr_t>(0x669B7), "\x8B\x86\xA2\x63\x00\x00"); // in recalcSize
}

void CustomFont::init() {
    s_pFontCE = (stFont*)new uint8_t[sizeof(stFont)];
    ZeroMemory(s_pFontCE, sizeof(stFont));
    // Ставим VTable из SA-MP. В нём находятся функции из сампа, которые делают такие секас цвета
    *(DWORD*)s_pFontCE = g_sampBase.getAddr<DWORD>(0xEA3B8);

    s_onChatFontReset = std::make_unique<PLH::x86Detour>(g_sampBase.getAddr<UINT64>(0x6B170),
                                                         UINT64(&onChatFontReset),
                                                         &s_ui64OnChatFontReset);
    s_onLostDevice = std::make_unique<PLH::x86Detour>(g_sampBase.getAddr<UINT64>(0x6AA10),
                                                      UINT64(&onLostDevice),
                                                      &s_ui64OnLostDevice);
    s_onResetDevice = std::make_unique<PLH::x86Detour>(g_sampBase.getAddr<UINT64>(0x6AA50),
                                                       UINT64(&onResetDevice),
                                                       &s_ui64OnResetDevice);

    s_onChatFontReset->hook();
    s_onLostDevice->hook();
    s_onResetDevice->hook();

    patch::getBytes(g_sampBase.getAddr<std::uintptr_t>(0x66D06), m_oldByte[0]);

    char raw[]{
        '\xbe', 0, 0, 0, 0,             // mov esi, &pFontCE
        '\x8B', '\x06',                 // mov eax, [esi]
        '\x89', '\x44', '\x24', '\x10', // mov [esp+214h+pFontCE], eax
        '\xb8', 0, 0, 0, 0,             // mov eax, &pFont
        '\x8b', '\x30'                  // mov esi, [eax]
    };

    *(stFont***)(raw + 1) = &s_pFontCE;
    *(ID3DXFont***)(raw + 12) = &s_pShadowFont;
    patch::setRawThroughJump(g_sampBase.getAddr<std::uintptr_t>(0x66D06), raw, ARRAYSIZE(raw),
                             9, FALSE);

    // ----

    patch::getBytes(g_sampBase.getAddr<std::uintptr_t>(0x66D06), m_oldByte[1]);

    // ----

    if (!(m_bState = g_Config["samp"]["isCustomFont"].get<bool>()))
        turnOff();
    else
        turnOn();
}
CustomFont::~CustomFont() {
    s_onChatFontReset->unHook();
    s_onLostDevice->unHook();
    s_onResetDevice->unHook();

    SAFE_RELEASE(s_pFontCE->m_pFont);
    SAFE_RELEASE(s_pShadowFont);
    delete[] (std::uint8_t*)s_pFontCE;
}