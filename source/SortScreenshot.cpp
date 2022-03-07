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
#include "SortScreenshot.h"
#include "offsets.hpp"

/* Этот файл сортирует скриншоты по датам */

void SortScreenshot::Process() {
    if (g_Config["samp"]["isSortingScreenshots"].get<bool>()) {
        static char s_fullPathScreenshot[22] = "%s"; // '%s\\screens\\xxxx-xx-xx'
        static char s_pathScreenshot[35];            // '\\screens\\xxxx-xx-xx\\sa-mp-%03i.png'

        /*
        auto t = time(0);
        auto pLocalTime = localtime(&t);
        
        SYSTEMTIME - быстрее в ~7 раз
        */

        SYSTEMTIME timeInfo;
        GetLocalTime(&timeInfo);

        sprintf_s(s_pathScreenshot, "\\screens\\%d-%02d-%02d", timeInfo.wYear, timeInfo.wMonth, timeInfo.wDay);

        strcat(s_fullPathScreenshot, s_pathScreenshot);
        strcat(s_pathScreenshot, "\\sa-mp-%03i.png");

        patch::setPushOffset(g_sampBase.GET_ADDR(OFFSETS::SAMP::FORMATPATHSCREENSHOT), reinterpret_cast<uint32_t>(s_pathScreenshot));
        patch::setPushOffset(g_sampBase.GET_ADDR(OFFSETS::SAMP::FORMATFULLPATHSCREENSHOT), reinterpret_cast<uint32_t>(s_fullPathScreenshot));
    }
}