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
class SortScreenshot : public MLoad {
    std::uint8_t m_oldBytes[2][5];

    char m_fullPathScreenshot[22] = "%s"; // '%s\\screens\\xxxx-xx-xx'
    char m_pathScreenshot[35]{};          // '\\screens\\xxxx-xx-xx\\sa-mp-%03i.png'

    void turnOn() override;
    void turnOff() override;

public:
    void init() override;
    ~SortScreenshot() override;
};
