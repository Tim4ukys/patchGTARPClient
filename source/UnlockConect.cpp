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

void UnlockConect::Process() {
    patch::setRaw(g_gtarpclientBase.GET_ADDR(OFFSETS::GTARP::LOCKCONNECTTOOTHERSERVER), "\xEB\x17\x90", 3);
}