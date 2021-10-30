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
#ifndef _TIMEZONE_HPP_
#define _TIMEZONE_HPP_

#include <cstdint>
#include <string>
#include <array>

class TimeZone {
public:

    TimeZone(std::string UTC_local)
    {
        UTC_local.erase(UTC_local.begin() + 3U, UTC_local.end());
        m_nOffset = std::stoi(UTC_local, NULL, 10);
        g_pLog->Log("m_nOffset: %d", m_nOffset);
    }

    int8_t getOffset(std::string UTC_offset)
    {
        UTC_offset.erase(UTC_offset.begin() + 3U, UTC_offset.end());
        return std::stoi(UTC_offset, NULL, 10) - m_nOffset;
    }

    int8_t getHour(int8_t hour, std::string UTC_offset)
    {
        hour += getOffset(UTC_offset);
        return hour + (hour < 0 ? 24 : (hour > 23 ? -24 : 0));
    }

private:

    int8_t m_nOffset{};
};

#endif _TIMEZONE_HPP_