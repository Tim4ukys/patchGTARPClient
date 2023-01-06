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
#include "UnlockConect.h"
#include "offsets.hpp"

/**
* Этот файл выключает блокировку на подключения на стороние сервера
*/

void UnlockConect::init() {
    patch::setRaw(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::LOCKCONNECTTOOTHERSERVER), "\xEB\x17\x90");
}