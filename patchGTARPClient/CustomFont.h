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
class CustomFont : public MLoad {
    void turnOn() override;
    void turnOff() override;

public:
    void init() override;
    ~CustomFont() override;

private:

    std::uint8_t m_oldByte[2][9];

    static std::string s_fontFaceName;

    static uint64_t                        s_ui64OnChatFontReset;
    static std::unique_ptr<PLH::x86Detour> s_onChatFontReset;
    static void __fastcall onChatFontReset(PVOID p_this, PVOID trash);

    static uint64_t                        s_ui64OnLostDevice;
    static std::unique_ptr<PLH::x86Detour> s_onLostDevice;
    static void __fastcall onLostDevice(PVOID p_this, PVOID trash);

    static uint64_t                        s_ui64OnResetDevice;
    static std::unique_ptr<PLH::x86Detour> s_onResetDevice;
    static void __fastcall onResetDevice(PVOID p_this, PVOID trash);

    struct stFont : public ID3DXFont {
        ID3DXFont* m_pFont;
    };
    
    static stFont*    s_pFontCE;
    static ID3DXFont* s_pShadowFont;
};
