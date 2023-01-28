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

enum CursorMode {
    CURSOR_NONE,
    CURSOR_LOCKKEYS_NOCURSOR,
    CURSOR_LOCKCAMANDCONTROL,
    CURSOR_LOCKCAM,
    CURSOR_LOCKCAM_NOCURSOR
};

class SAMP {
public:
    
    SAMP();
    //~SAMP();

    bool isSAMPInit();

    void addChatMessage(D3DCOLOR color, const char* fmt, ...);
    void cmdRect(const char* szCmdName, CMDPROC pCmdProc);

    void setCursorMode(int nMode, BOOL bImmediatelyHideCursor);
    void redraw();

    void setHour(unsigned char hour);

private:

    void initPointerList();

    PVOID g_pNetGame = nullptr, g_pChat = nullptr, g_pInput = nullptr, g_pGame = nullptr;

    void(__thiscall* g_fncAddEntry)(PVOID pChat, char eType, const char* szString, const char* szNick, D3DCOLOR dwTextColor, D3DCOLOR dwNickColor) = nullptr;
    void(__thiscall* g_fncAddCmdProc)(PVOID pInput, const char* szCmdName, CMDPROC cmdHandler) = nullptr;
    void(__thiscall* g_fncSetCursorMode)(PVOID pGame, int nMode, BOOL bImmediatelyHideCursor) = nullptr;
};
