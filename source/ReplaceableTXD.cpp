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
#include "ReplaceableTXD.h"
#include "offsets.hpp"

/* Этот файл отключает проверку на повреждение GTARP_HUD */

void ReplaceableTXD::Process() {
    patch::setRaw(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::DISABLECHECKTXD), "\xEB\x42", 2);
}