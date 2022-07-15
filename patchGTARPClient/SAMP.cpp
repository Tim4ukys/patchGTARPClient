#include "pch.h"
#include "SAMP.h"

SAMP::SAMP() {
    initPointerList();
}

bool SAMP::isSAMPInit() {
    if (!g_pNetGame) initPointerList();
    return g_pNetGame != nullptr;
}

void SAMP::initPointerList() {
    enum eSampOffs {
        INFO,
        CHAT,
        INPUT,
        GAME,
        FNC_ADDENTRY,
        FNC_CMDRECT,
        FNC_SETCURSORMODE
    };
    const uint32_t sampOffsets[]{
        /*sampInfo, sampChat, sampInput, sampGame, fnAddEntry, fnCmdRect, SetCursorMode*/
        0x026E8DC, 0x026E8C8, 0x026E8CC, 0x26E8F4, 0x00067460, 0x0069000, 0x9FFE0 // 037 r3-1
    };

    g_pNetGame = *(PVOID*)g_sampBase.getAddress(sampOffsets[INFO]);
    g_pChat = *(PVOID*)g_sampBase.getAddress(sampOffsets[CHAT]);
    g_pInput = *(PVOID*)g_sampBase.getAddress(sampOffsets[INPUT]);
    g_pGame = *(PVOID*)g_sampBase.getAddress(sampOffsets[GAME]);

    g_fncAddEntry = reinterpret_cast<decltype(g_fncAddEntry)>(g_sampBase.getAddress(sampOffsets[FNC_ADDENTRY]));
    g_fncAddCmdProc = reinterpret_cast<decltype(g_fncAddCmdProc)>(g_sampBase.getAddress(sampOffsets[FNC_CMDRECT]));
    g_fncSetCursorMode = reinterpret_cast<decltype(g_fncSetCursorMode)>(g_sampBase.getAddress(sampOffsets[FNC_SETCURSORMODE]));
}

void SAMP::addChatMessage(D3DCOLOR color, const char* fmt, ...) {
    if (!g_pNetGame || !g_pChat || !g_fncAddEntry) return;

    char    buff[512]{};
    va_list arrg = nullptr;
    va_start(arrg, fmt);
    vsprintf_s(buff, fmt, arrg);
    va_end(arrg);

    g_fncAddEntry(g_pChat, CHAT_TYPE_DEBUG, buff, nullptr, color, NULL);
}

void SAMP::cmdRect(const char* szCmdName, CMDPROC pCmdProc) {
    if (!g_pNetGame || !g_pInput || !g_fncAddCmdProc) return;

    g_fncAddCmdProc(g_pInput, szCmdName, pCmdProc);
}

void SAMP::setCursorMode(int nMode, BOOL bImmediatelyHideCursor) {
    if (!g_pNetGame || !g_pGame || !g_fncSetCursorMode) return;

    g_fncSetCursorMode(g_pGame, nMode, bImmediatelyHideCursor);
}