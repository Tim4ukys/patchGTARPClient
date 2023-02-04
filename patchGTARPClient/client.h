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
    static std::array<std::string, 3U> sortURLtext(std::string URL);

public:
    /**
    * @brief Скачивает сайт в виде строки
    * @param URL строка
    * @warning поддерживается только HTTP и HTTPS
    * @example downloadStringFromURL("http://youtube.com.ua/cock/suck")
    */
    static std::string downloadStringFromURL(std::string_view URL);
};
