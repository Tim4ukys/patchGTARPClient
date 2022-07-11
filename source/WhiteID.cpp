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

void WhiteID::Process() {
    if (g_Config["samp"]["isWhiteID"].get<bool>())
    {
        const char* formatPlayerTag = "%s {FFFFFF}(%d)";
        static char s_arrBuffPlayerTag[128 + 8];

        patch__setPushOffset(g_sampBase.GET_ADDR(OFFSETS::SAMP::RENDERPLAYERTAG_FORMAT), reinterpret_cast<uint32_t>(formatPlayerTag));
        patch__setPushOffset(g_sampBase.GET_ADDR(OFFSETS::SAMP::RENDERPLAYERTAG_PUSHBUFF_1), reinterpret_cast<uint32_t>(&s_arrBuffPlayerTag));

        patch__setPushOffset(g_sampBase.GET_ADDR(OFFSETS::SAMP::RENDERPLAYERTAG_PUSHBUFF_2), reinterpret_cast<uint32_t>(&s_arrBuffPlayerTag));
    }
}