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
#include "DelCarTable.h"
#include "offsets.hpp"

/**
* Этот файл убирает табличку, которая появляется когда перс садится в авто
*/

void DelCarTable::Process() {
    if (!g_Config["vehicleHud"]["isDrawHelpTablet"].get<bool>()) {
        patch__fill(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::DRAWSPEEDOMETER_DRAWRAMKA), 0x1E, 0x90 /*NOP*/);
    }
}