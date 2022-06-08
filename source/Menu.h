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
class Menu {
public:
    static void Process();
    static void show_cursor(bool show);
    static void set_style();
    static void background();
    static void title_menu();
    static void render_warning();
    static void render_doska();
};
