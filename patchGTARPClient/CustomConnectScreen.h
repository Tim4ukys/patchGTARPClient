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
class CustomConnectScreen : public MLoad {
    using sub_1F270_t = bool(__fastcall*)(DWORD*, void*, void*, int, char);
    static sub_1F270_t sub_1F270_orig;
    static unsigned char s_nHour;
    static bool __fastcall sub_1F270(DWORD* pthis, void* trash, void* a2, int a3, char a4);

public:
    void init() override;
};
