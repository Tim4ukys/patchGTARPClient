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

void DelCarTable::turnOn() {
    patch::setJump<patch::TYPE_JMP::sml>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::DRAWSPEEDOMETER_DRAWRAMKA), 30);
}
void DelCarTable::turnOff() {
    patch::setRaw(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::DRAWSPEEDOMETER_DRAWRAMKA), "\x85\xC0");
}

void DelCarTable::init() {
    if (m_bState = !g_Config["vehicleHud"]["isDrawHelpTablet"].get<bool>())
        turnOn();
}