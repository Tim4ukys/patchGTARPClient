#ifndef _CSAMP_H_
#define _CSAMP_H_

#include <WinBase.h>
#include <cstdint>

#ifndef _D3D9_H_
typedef uint32_t D3DCOLOR;
#endif

typedef void(__cdecl* CMDPROC)(const char*);

enum eChatMessageType {
	CHAT_TYPE_NONE = 0,
	CHAT_TYPE_CHAT = 2,
	CHAT_TYPE_INFO = 4,
	CHAT_TYPE_DEBUG = 8
};

class CSAMP {
public:
	CSAMP();
	~CSAMP();

	bool isSAMPInit();

	void AddChatMessage(D3DCOLOR color, const char* fmt, ...);
	void AddCmdProc(const char* cmdName, CMDPROC cmdHandler);

private:

	const uint32_t sampOffsets[5]
	{
		/*sampInfo, sampChat, sampInput, fnAddEntry, fnCmdRect*/
		0x026E8DC, 0x26E8C8, 0x026E8CC, 0x00067460, 0x0069000, // 037 r3-1
	};

	DynamicLibrary samp{ "samp.dll" };

	void initPointerList();

	// pointers
	PVOID g_pNetGame = nullptr, g_pChat = nullptr, g_pInputInfo = nullptr;

	void(__thiscall* g_fncAddEntry)(PVOID pChat, uint8_t eType, const char* szString, const char* szNick, D3DCOLOR dwTextColor, D3DCOLOR dwNickColor) = nullptr;
	void(__thiscall* g_fncAddCmdProc)(PVOID pInput, const char* szCmdName, CMDPROC cmdHandler) = nullptr;
};


#endif
