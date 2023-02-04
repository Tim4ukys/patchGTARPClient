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
    enum class OFF : std::uintptr_t {
        INFO = 0x026E8DC,
        CHAT = 0x026E8C8,
        INPUT = 0x026E8CC,
        GAME = 0x26E8F4,
        FNC_ADDENTRY = 0x00067460,
        FNC_CMDRECT = 0x0069000,
        FNC_SETCURSORMODE = 0x9FFE0
    };

    g_pNetGame = *g_sampBase.getAddr<PVOID*>(OFF::INFO);
    g_pChat = *g_sampBase.getAddr<PVOID*>(OFF::CHAT);
    g_pInput = *g_sampBase.getAddr<PVOID*>(OFF::INPUT);
    g_pGame = *g_sampBase.getAddr<PVOID*>(OFF::GAME);

    g_fncAddEntry = g_sampBase.getAddr<decltype(g_fncAddEntry)>(OFF::FNC_ADDENTRY);
    g_fncAddCmdProc = g_sampBase.getAddr<decltype(g_fncAddCmdProc)>(OFF::FNC_CMDRECT);
    g_fncSetCursorMode = g_sampBase.getAddr<decltype(g_fncSetCursorMode)>(OFF::FNC_SETCURSORMODE);
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

void SAMP::redraw() {
    if (!g_pNetGame || !g_pChat) return;

    // pChatInfo->m_bRedraw
    *reinterpret_cast<BOOL*>((std::uintptr_t)g_pChat + 0x63DA) = 1;
}

void SAMP::setCursorMode(int nMode, BOOL bImmediatelyHideCursor) {
    if (!g_pNetGame || !g_pGame || !g_fncSetCursorMode) return;

    g_fncSetCursorMode(g_pGame, nMode, bImmediatelyHideCursor);
}

void SAMP::setHour(unsigned char hour) {
    if (!g_pNetGame) return;

    *PCHAR(*PDWORD((DWORD)g_pNetGame + 981) + 44) = hour;
}