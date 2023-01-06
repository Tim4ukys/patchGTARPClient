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
#include "WhiteID.h"
#include "offsets.hpp"

/* Этот файл делает ID цвета *MY CUM* */

void WhiteID::turnOn() {
    const char* formatPlayerTag = "%s {FFFFFF}(%d)";
    static char s_arrBuffPlayerTag[128 + 8];

    std::uint8_t buff[5]{ 0x68 }; // push ...
    *reinterpret_cast<std::uint32_t*>(buff + 1) = (std::uint32_t)formatPlayerTag;
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::RENDERPLAYERTAG_FORMAT), buff);
    *reinterpret_cast<std::uint32_t*>(buff + 1) = (std::uint32_t)s_arrBuffPlayerTag;
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::RENDERPLAYERTAG_PUSHBUFF_1), buff);
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::RENDERPLAYERTAG_PUSHBUFF_2), buff);
}

void WhiteID::turnOff() {
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::RENDERPLAYERTAG_FORMAT), m_oldBytes[0]);
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::RENDERPLAYERTAG_PUSHBUFF_1), m_oldBytes[1]);
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::RENDERPLAYERTAG_PUSHBUFF_2), m_oldBytes[2]);
}

void WhiteID::init() {
    patch::getBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::RENDERPLAYERTAG_FORMAT), m_oldBytes[0]);
    patch::getBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::RENDERPLAYERTAG_PUSHBUFF_1), m_oldBytes[1]);
    patch::getBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::RENDERPLAYERTAG_PUSHBUFF_2), m_oldBytes[2]);

    if (m_bState = g_Config["samp"]["isWhiteID"].get<bool>())
        turnOn();
}
WhiteID::~WhiteID() {
    if (m_bState)
        turnOff();
}