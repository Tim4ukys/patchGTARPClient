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

class OFFSETS {
public:

	enum class GTARP : unsigned int {
        LOCKCONNECTTOOTHERSERVER = 0x5119,
        INITTEXTURE_INITHOOK = 0x1C83D,
        INITTEXTURE_FNC = 0x1C210,

        ARRAYSERVERLOGO = 0xD9DAC, /* 01 - +0h, 02 - +4h, test - +8h*/
        SERVERID = 0xCEB20,

        DRAWHUD = 0x1CC00,
        DRAWSPEEDOMETER_DRAWRAMKA = 0x26412,

        WANTEDLEVEL = 0xD9B60,

        DISABLECHECKTXD = 0xF134,

        DISABLEWINTERWINDOW = 0x26384,

        DISABLE_CALL_SET_CUSTOM_RADAR = 0xF486,
        CUSTOM_PATH_TXD_HUD = 0x2F5CF, 

        DISABLE_RECHANGE_SCALE_STATE = 0x2FBF9
	};

    enum class SAMP : unsigned int {
        ENABLECLOCK = 0xA0D84,

        CREATEFONT = 0xC650A,

        RENDERPLAYERTAG_FORMAT = 0x74E35,
        RENDERPLAYERTAG_PUSHBUFF_1 = 0x74E3A,
        RENDERPLAYERTAG_PUSHBUFF_2 = 0x74E80,

        FORMATPATHSCREENSHOT = 0x61B52,
        FORMATFULLPATHSCREENSHOT = 0xC4573,

        SCALE_HUD_FIX = 0x68'100,
        SCALE_HUD_FIX_STATE = 0x117'478,
        DISABLE_LOAD_SCALE_STATE = 0xC5'23B
    };
    
    enum class GTA_SA : unsigned int {
        DRAWWANTED = 0x58D9A0,
        DRAWHUD_CLOCK_SPRINTF = 0x58EBAF,
        GAMELOOP = 0x53BEE0
    };

};