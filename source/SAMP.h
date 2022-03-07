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

#ifndef _D3D9_H_
typedef DWORD D3DCOLOR;
#endif

typedef void(__cdecl* CMDPROC)(const char*);

enum eChatMessageType {
    CHAT_TYPE_NONE = 0,
    CHAT_TYPE_CHAT = 2,
    CHAT_TYPE_INFO = 4,
    CHAT_TYPE_DEBUG = 8
};

class SAMP {
public:
    
    SAMP();
    //~SAMP();

    bool isSAMPInit();

    void addChatMessage(D3DCOLOR color, const char* fmt, ...);

private:

    void initPointerList();

    PVOID g_pNetGame = nullptr, g_pChat = nullptr, g_pInput = nullptr;

    void(__thiscall* g_fncAddEntry)(PVOID pChat, char eType, const char* szString, const char* szNick, D3DCOLOR dwTextColor, D3DCOLOR dwNickColor) = nullptr;
    void(__thiscall* g_fncAddCmdProc)(PVOID pInput, const char* szCmdName, CMDPROC cmdHandler) = nullptr;
};
