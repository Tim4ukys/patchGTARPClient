#pragma once

class OFFSETS {
public:

	enum class GTARP : unsigned int {
        LOCKCONNECTTOOTHERSERVER = 0x494C,
        INITTEXTURE_INITHOOK = 0x1BD3D,
        INITTEXTURE_FNC = 0x1B710,

        ARRAYSERVERLOGO = 0xDAC2C, /* 01 - +0h, 02 - +4h, test - +8h*/
        SERVERID = 0xCF990,

        DRAWHUD = 0x1C100,
        DRAWSPEEDOMETER_DRAWRAMKA = 0x25A22,

        WANTEDLEVEL = 0xDAB1C,

        DISABLECHECKTXD = 0xE6E4
	};

    enum class SAMP : unsigned int {
        ENABLECLOCK = 0xA0D84,

        CREATEFONT = 0xC650A,

        RENDERPLAYERTAG_FORMAT = 0x74E35,
        RENDERPLAYERTAG_PUSHBUFF_1 = 0x74E3A,
        RENDERPLAYERTAG_PUSHBUFF_2 = 0x74E80,

        FORMATPATHSCREENSHOT = 0x61B52,
        FORMATFULLPATHSCREENSHOT = 0xC4573
    };
    
    enum class GTASA : unsigned int {
        DRAWWANTED = 0x58D9A0,
        DRAWHUD_CLOCK_SPRINTF = 0x58EBAF,
        GAMELOOP = 0x53BEE0
    };

};