#include "dllmain.h"
#include "CSAMP.h"

CSAMP::CSAMP()
{
    initPointerList();
}
CSAMP::~CSAMP()
{

}

#define GET_OFFSET(a) samp.getAddress(static_cast<uint32_t>(sampOffsets[a]))

void CSAMP::initPointerList()
{
    g_pNetGame = *reinterpret_cast<PVOID*>(GET_OFFSET(0));
    g_pChat = *reinterpret_cast<PVOID*>(GET_OFFSET(1));
    g_pInputInfo = *reinterpret_cast<PVOID*>(GET_OFFSET(2));

    g_fncAddEntry = reinterpret_cast<decltype(g_fncAddEntry)>(GET_OFFSET(3));
    g_fncAddCmdProc = reinterpret_cast<decltype(g_fncAddCmdProc)>(GET_OFFSET(4));
}

bool CSAMP::isSAMPInit()
{
	if (!g_pNetGame) initPointerList();
	return g_pNetGame != nullptr;
}

void CSAMP::AddChatMessage(D3DCOLOR color, const char* fmt, ...)
{
    if (!g_pNetGame || !g_pChat || !g_fncAddEntry) return;

    char buff[512]{};
    va_list arrg = nullptr;
    va_start(arrg, fmt);
    vsprintf_s(buff, fmt, arrg);
    va_end(arrg);
    //buff[143] = '\0';

    g_fncAddEntry(g_pChat, CHAT_TYPE_DEBUG, buff, nullptr, color, NULL);
}

void CSAMP::AddCmdProc(const char* cmdName, CMDPROC cmdHandler)
{
    if (!g_pNetGame || !g_pInputInfo || !g_fncAddCmdProc) return;

    g_fncAddCmdProc(g_pInputInfo, cmdName, cmdHandler);
}