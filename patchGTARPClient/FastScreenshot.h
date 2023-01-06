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
class FastScreenshot : public MLoad {
    std::uint8_t m_byteOrig[5];
    static DWORD s_hTakeSound;

    static void __cdecl hkTakeScreenshot();

    void turnOn() override;
    void turnOff() override;

public:
    void init() override;
    ~FastScreenshot() override;
};
