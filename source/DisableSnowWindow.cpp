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
#include "DisableSnowWindow.h"
#include "offsets.hpp"

/* Этот файл отключает заморозку окна на больших скоростях */

void DisableSnowWindow::Process() {
    if (g_Config["vehicleHud"]["isDisableSnowWindow"].get<bool>()) {
        patch::setRaw(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::DISABLEWINTERWINDOW), "\xEB\x79" /* jmp 0x7b */, 2U);
    }
}