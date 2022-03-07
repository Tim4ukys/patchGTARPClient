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
#pragma once

class client {
public:
    /**
    * @brief Скачивает сайт в виде строки
    * @param URL строка
    * @warning поддерживается только HTTP и HTTPS
    * @example http://youtube.com.ua/cock/suck
    */
    static std::string downloadStringFromURL(std::string URL);
};
