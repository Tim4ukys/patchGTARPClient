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
#include "SortScreenshot.h"
#include "offsets.hpp"

/* Этот файл сортирует скриншоты по датам */

void SortScreenshot::init() {
    SYSTEMTIME timeInfo{};
    GetLocalTime(&timeInfo);

    sprintf_s(m_pathScreenshot, "\\screens\\%d-%02d-%02d", timeInfo.wYear, timeInfo.wMonth, timeInfo.wDay);

    strcat(m_fullPathScreenshot, m_pathScreenshot);
    strcat(m_pathScreenshot, "\\sa-mp-%03i.png");

    // ----

    patch::getBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FORMATPATHSCREENSHOT), m_oldBytes[0]);
    patch::getBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FORMATFULLPATHSCREENSHOT), m_oldBytes[1]);

    // ----

    if (m_bState = g_Config["samp"]["isSortingScreenshots"].get<bool>())
        turnOn();
}
SortScreenshot::~SortScreenshot() {
    if (m_bState)
        turnOff();
}

void SortScreenshot::turnOff() {
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FORMATPATHSCREENSHOT), m_oldBytes[0]);
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FORMATFULLPATHSCREENSHOT), m_oldBytes[1]);
}

void SortScreenshot::turnOn() {
    std::uint8_t buff[5]{0x68}; // push ...
    *reinterpret_cast<std::uint32_t*>(buff + 1) = (std::uint32_t)m_pathScreenshot;
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FORMATPATHSCREENSHOT), buff);
    *reinterpret_cast<std::uint32_t*>(buff + 1) = (std::uint32_t)m_fullPathScreenshot;
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FORMATFULLPATHSCREENSHOT), buff);
}