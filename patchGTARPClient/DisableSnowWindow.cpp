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

void DisableSnowWindow::turnOff() {
    patch::writeMemory<std::uint8_t>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::NEWYEAR_DISABLEWINTERWINDOW), 0x74);
}
void DisableSnowWindow::turnOn() {
    patch::writeMemory<std::uint8_t>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::NEWYEAR_DISABLEWINTERWINDOW), 0xEB);
}

void DisableSnowWindow::init() {
    if (m_bState = !g_Config["vehicleHud"]["isDisableSnowWindow"].get<bool>())
        turnOn();
}
