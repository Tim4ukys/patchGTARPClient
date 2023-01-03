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
class WhiteID : public MLoad {
    std::uint8_t m_oldBytes[3][5];

    void turnOn() override;
    void turnOff() override;

public:
    void init() override;
    ~WhiteID() override;
};
