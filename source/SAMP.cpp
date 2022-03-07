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
        FNC_ADDENTRY,
        FNC_CMDRECT
    };
    const uint32_t sampOffsets[]{
        /*sampInfo, sampChat, sampInput, fnAddEntry, fnCmdRect*/
        0x026E8DC, 0x26E8C8, 0x026E8CC, 0x00067460, 0x0069000 // 037 r3-1
    };

    g_pNetGame = *(PVOID*)g_sampBase.getAddress(sampOffsets[INFO]);
    g_pChat = *(PVOID*)g_sampBase.getAddress(sampOffsets[CHAT]);
    g_pInput = *(PVOID*)g_sampBase.getAddress(sampOffsets[INPUT]);

    g_fncAddEntry = reinterpret_cast<decltype(g_fncAddEntry)>(g_sampBase.getAddress(sampOffsets[FNC_ADDENTRY]));
    g_fncAddCmdProc = reinterpret_cast<decltype(g_fncAddCmdProc)>(g_sampBase.getAddress(sampOffsets[FNC_CMDRECT]));
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