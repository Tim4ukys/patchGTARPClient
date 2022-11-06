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
    /*
        Sig - The signature by which the code section can be found.
        offs - The number of bits that need to be added to get the desired offset.
        can be found in - This means that this variable is used in this place.
    */
    enum class GTARP : unsigned int {
        /* Sig: 83 F8 FF 75 14 6A 00 FF 15 ? ? ? ? 50 FF 15 ? ? ? ? E9 ? ? ? ? */
        LOCKCONNECTTOOTHERSERVER = 0x5119,
        /* Sig(offs: +5h): A3 ? ? ? ? B9 ? ? ? ? 8B C4 C7 00 ? ? ? ? E8 ? ? ? ? 51 89 45 94  */
        INITTEXTURE_INITHOOK = 0x1C4DD,
        /* Sig: 55 8B EC 83 EC 70 */
        INITTEXTURE_FNC = 0x1BEB0,

        /* [can be found in] Sig: 8B 04 B5 ? ? ? ? 8B 0D ? ? ? ? FF 30 8B 41 20 6A 01 FF D0 8B 0D ? ? ? ? */
        ARRAYSERVERLOGO = 0xD8DAC, /* 01 - +0h, 02 - +4h, test - +8h*/
        /* Sig(offs: +6h): 0F 82 ? ? ? ? A1 ? ? ? ? */
        SERVERID = 0xCDB20, // 2 - test server

        /* Sig: 55 8B EC 83 EC 48 A1 ? ? ? ? 33 C5 89 45 FC 56 */
        DRAWHUD = 0x1C8A0,
        /* Sig: 85 C0 75 1F C7 05 ? ? ? ? ? ? ? ? */
        DRAWSPEEDOMETER_DRAWRAMKA = 0x25DCA,

        /* [can be found in] Sig: 8B 0D ? ? ? ? 83 C4 08 85 C9 */
        WANTEDLEVEL = 0xD8B60,

        /* Sig: 8B C8 E8 ? ? ? ? 3D ? ? ? ? */
        //DISABLECHECKTXD = 0xF134, goodbay epta

        //DISABLEWINTERWINDOW = 0x26384, goodbay epta

        /* Sig(offs: +5h): A3 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 51 */
        DISABLE_CALL_SET_CUSTOM_RADAR = 0xF486,
        /* Sig: FF D7 83 C4 10 8B D8 E9 ? ? ? ? 68 ? ? ? ? 56 E8 ? ? ? ? 83 C4 08 85 C0 75 45 */
        CUSTOM_PATH_TXD_HUD = 0x2ECAF, 
        
        /* Sig: 85 C0 0F 95 45 F4 88 1E */
        DISABLE_RECHANGE_SCALE_STATE = 0x2F2D9
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