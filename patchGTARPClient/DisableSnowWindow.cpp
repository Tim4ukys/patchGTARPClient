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

void DisableSnowWindow::Process() {
    if (!g_Config["vehicleHud"]["isDisableSnowWindow"].get<bool>()) {
        patch__setRaw(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::NEWYEAR_DISABLEWINTERWINDOW), "\xEB", 1);
    }
}
