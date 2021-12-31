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
#include "dllmain.h"

const std::string SAMP_CMP{ "E86D9A0A0083C41C85C0" };
const std::string GTARPCLIENTSIDE_CMP{ "CF746A8B4B0C8D433C39" };

#define UPDATE_DELAY 12000
#define CURRENTVERSIONA "v4.0"
#define CURRENTVERSION L"v4.0"
#define GITHUBURLA "github.com/Tim4ukys/patchGTARPClient"
#define GITHUBURL L"github.com/Tim4ukys/patchGTARPClient"

CLog* g_pLog        = nullptr;
CConfig* g_pConfig  = nullptr;

DynamicLibrary g_gtarpclientside{ "gtarp_clientside.asi" };
DynamicLibrary g_dlSAMP{ "samp.dll" };

PLH::CapstoneDisassembler g_dis = { PLH::Mode::x86 };

///////////////////////////////////////////////////////////////////

uint64_t g_ui64DrawWantedJumpTrampline;
PLH::x86Detour* g_pDrawWantedDetour = nullptr;
NOINLINE void drawWantedDetourFNC()
{
    static auto s_piWantedLevel = reinterpret_cast<DWORD*>(g_gtarpclientside.getAddress(OFFSETS::GTARP_WANTEDLEVEL));

    if (*s_piWantedLevel != FindPlayerWanted(-1)->m_nWantedLevel)
    {
        //g_pLog->Log("s_piWantedLevel: %d", int(*s_piWantedLevel));
        //FindPlayerWanted(-1)->SetWantedLevel(FindPlayerWanted(-1)->m_nWantedLevel = *s_piWantedLevel);
        FindPlayerPed(-1)->GetWanted()->SetWantedLevel(*s_piWantedLevel);
    }

    ((void(__cdecl*)())g_ui64DrawWantedJumpTrampline)(); // call original
}

int drawClockSprintfDetourFNC(char* buff, const char* f, ...)
{
    SYSTEMTIME timeInfo;
    GetLocalTime(&timeInfo);

    int32_t hour{ timeInfo.wHour };

    if (g_pConfig->getConfig()->m_clock.m_bFixTimeZone)
    {
        static int8_t* s_pTimeOffset = nullptr;

        if (!s_pTimeOffset)
        {
            TIME_ZONE_INFORMATION timeZoneInfo;
            GetTimeZoneInformation(&timeZoneInfo);

            s_pTimeOffset = new int8_t;
            *s_pTimeOffset = int8_t(timeZoneInfo.Bias / 60 + 3);
        }

        hour += *s_pTimeOffset;
        hour += hour < 0 ? 24 : (hour > 23 ? -24 : 0);
    }

    return sprintf(buff, "%02d:%02d", hour, timeInfo.wMinute);
}

///////////////////

ID3DXFont* g_pChatFont = nullptr;

void chatFontReset()
{
    SAFE_RELEASE(g_pChatFont);

    int iFontSize = ((int(*)())g_dlSAMP.getAddress(0xC5B20))();
    int iFontWeight = ((int(*)())g_dlSAMP.getAddress(0xC5BD0))();

    g_pLog->Log("iFontSize: %d | iFontWeight: %d", iFontSize, iFontWeight);

    D3DXCreateFontA(LPDIRECT3DDEVICE9(RwD3D9GetCurrentD3DDevice()), iFontSize, 0, iFontWeight, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, g_pConfig->getConfig()->m_samp.m_FontFaceName.c_str(), &g_pChatFont);
}

void chatFontResetDevice()
{
    if (g_pChatFont)
        g_pChatFont->OnResetDevice();
}

void chatFontLostDevice()
{
    if (g_pChatFont)
        g_pChatFont->OnLostDevice();
}

typedef int(WINAPI* _Direct9CreateFontA)(int _a1, int _a2, int _a3, int _a4, int _a5, int _a6, int _a7, int _a8, int _a9, int _a10, const char* _fontFaceName, int _a12, int _a13, int _a14, int _a15);
_Direct9CreateFontA g_pOrigDirect9CreateFontA = nullptr;

int WINAPI Direct9CreateFontA__Detour(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, const char* fontFaceName, int a12, int a13, int a14, int a15)
{
    return g_pOrigDirect9CreateFontA(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, g_pConfig->getConfig()->m_samp.m_FontFaceName.c_str(), a12, a13, a14, a15);
}

///////////////////

CSAMP* g_pSAMP = nullptr;

uint64_t g_ui64GameLoopJumpTrampline;
PLH::x86Detour* g_pGameLoopDetour = nullptr;
NOINLINE void gameLoopDetourFNC()
{
    ((void(*)())g_ui64GameLoopJumpTrampline)(); // call original fnc

    static bool s_isInit{};
    if (s_isInit || !g_pSAMP->isSAMPInit()) return;

    static TICK s_oldTime = GetTickCount64();
    if (GetTickCount64() - s_oldTime > UPDATE_DELAY)
    {
        json j = json::parse(Client::downloadStringFromURL(true, "raw.githubusercontent.com", "/Tim4ukys/patchGTARPClient/master/update.json"));
        if (strcmp(j["vers"].get<std::string>().c_str(), CURRENTVERSIONA) != 0)
        {
            g_pSAMP->AddChatMessage(0x990000, "[{FF9900}patchGTARPClient{990000}] {990000}ВНИМАНИЕ: {FF9900}Вышло обновление!");
            g_pSAMP->AddChatMessage(0x990000, "[{FF9900}patchGTARPClient{990000}] {FF9900}Пожалуйста, обновите плагин!");
            g_pSAMP->AddChatMessage(0x990000, "[{FF9900}patchGTARPClient{990000}] {FF9900}Сайт: {990000}" GITHUBURLA "{FF9900}!");
        }
        g_pLog->Log("[CUpdate] Last version: %s", j["vers"].get<std::string>().c_str());
        s_isInit = true;
    }
}

//uint64_t g_ui64LoadLibraryAHooked_Detour;
//PLH::x86Detour* g_pLoadLibraryAHookedDetour = nullptr;
//NOINLINE HMODULE WINAPI loadLibraryHookedDetourFNC(const char* fileName)
//{
//    if (!strcmp(fileName, "SAMPFUNCS.asi"))
//        return LoadLibraryA(fileName);
//    
//    return ((HMODULE(WINAPI*)(const char* a1))g_ui64LoadLibraryAHooked_Detour)(fileName); // call original
//}

///////////////////////////////////////////////////////////////////

CSprite2d* g_aServerSprite = nullptr;

int drawServerIcon()
{
    static auto s_Conf = g_pConfig->getConfig();
    if (s_Conf->m_serverIcon.m_bState)
    {
        g_aServerSprite->Draw(
            SCREEN_COORD_LEFT(s_Conf->m_serverIcon.m_fX), SCREEN_COORD_TOP(s_Conf->m_serverIcon.m_fY),
            SCREEN_COORD(static_cast<float>(g_aServerSprite->m_pTexture->raster->width / 2)), SCREEN_COORD(static_cast<float>(g_aServerSprite->m_pTexture->raster->height / 2)),
            CRGBA(0xFF, 0xFF, 0xFF));
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////

uint64_t g_ui64LoadTextureHudJumpTrampline;
PLH::x86Detour* g_pLoadTextureHudDetour = nullptr;
NOINLINE void loadTextureHudDetourFNC()
{
    ((void(__cdecl*)())g_ui64LoadTextureHudJumpTrampline)(); // call original

    RwTexture** serverIcon = reinterpret_cast<RwTexture**>(g_gtarpclientside.getAddress(OFFSETS::GTARP_ARRAYSERVERLOGO));
    //while (!serverIcon[0] || !serverIcon[1] || !serverIcon[2]) Sleep(100);

    g_aServerSprite = new CSprite2d;
    auto serverID = *reinterpret_cast<int*>(g_gtarpclientside.getAddress(OFFSETS::GTARP_SERVERID));
    serverID = serverID > 2 || serverID < 0 ? 0 : serverID;

    g_aServerSprite->m_pTexture = serverIcon[serverID];

    plugin::patch::ReplaceFunction(g_gtarpclientside.getAddress(OFFSETS::GTARP_DRAWHUD), &drawServerIcon);
}

void MainFunction()
{
    if (auto currentCMP = patch::getHEX(g_gtarpclientside.getAddress(0xBABE), 10); currentCMP != GTARPCLIENTSIDE_CMP) {
        MessageBoxW(
            NULL,
            L"ERROR: Эта версия плагина ещё не поддерживает эту версию клиента игры.\n\n"
            L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENTVERSION L"\n"
            L"GitHub: " GITHUBURL,
            L"!000patchGTARPClientByTim4ukys.ASI",
            MB_ICONERROR
        );
        g_pLog->Log("gtarp_cmp: %s", currentCMP.c_str());
        TerminateProcess(GetCurrentProcess(), -1);
    }
    else if (currentCMP = patch::getHEX(g_dlSAMP.getAddress(0xBABE), 10); currentCMP != SAMP_CMP) {
        MessageBoxW(
            NULL,
            L"ERROR: Эта версия плагина ещё не поддерживает эту версию SAMP.\n\n"
            L"Пожалуйста, дождитесь обновление патча и обновите его. Текущая версия: " CURRENTVERSION L"\n"
            L"GitHub: " GITHUBURL,
            L"!000patchGTARPClientByTim4ukys.ASI",
            MB_ICONERROR
        );
        g_pLog->Log("samp_cmp: %s", currentCMP.c_str());
        TerminateProcess(GetCurrentProcess(), -1);
    }
    else {
        g_pLog->Log("gtarp_clientside & samp: Version supported!!!");
    }

    /*
        Выключает лок на подключение на стороние сервера.
    */
    patch::setRaw(g_gtarpclientside.getAddress(OFFSETS::GTARP_LOCKCONNECTTOOTHERSERVER), "\xEB\x17\x90", 3);

    /*
        Выключаем проверку на повреждение GTARP_HUD.TXD
    */
    patch::setRaw(g_gtarpclientside.getAddress(OFFSETS::GTARP_DISABLECHECKTXD), "\xEB\x42", 2);

    /*
        Возрващает самповский худ

        .text:0002B81D - start nop
        .text:0002B97A - end
        ---
        size = 0x168
    */
    patch::fill(g_gtarpclientside.getAddress(OFFSETS::GTARP_INITTEXTURE_INITHOOK), 0x15D, 0x90);

    /*
        Исправляет уровень состояние розыска.
    */
    g_pDrawWantedDetour = new PLH::x86Detour(reinterpret_cast<PCHAR>(OFFSETS::GTASA_DRAWWANTED), reinterpret_cast<PCHAR>(&drawWantedDetourFNC), &g_ui64DrawWantedJumpTrampline, g_dis);
    g_pDrawWantedDetour->hook();

    /*
        Фиксит положение иконки сервера
    */
    g_pLoadTextureHudDetour = new PLH::x86Detour(
        reinterpret_cast<PCHAR>(g_gtarpclientside.getAddress(OFFSETS::GTARP_INITTEXTURE_FNC)), reinterpret_cast<PCHAR>(&loadTextureHudDetourFNC), &g_ui64LoadTextureHudJumpTrampline, g_dis
    );
    g_pLoadTextureHudDetour->hook();

    /*
        Возвращает часы
    */
    patch::fill(g_dlSAMP.getAddress(OFFSETS::SAMP_ENABLECLOCK), 2, 0x90);

    /*
        Фиксим время на часах
    */
    plugin::patch::ReplaceFunctionCall(static_cast<uint32_t>(OFFSETS::GTASA_DRAWHUD_CLOCK_SPRINTF), &drawClockSprintfDetourFNC);

    /*
        Отключаем ебучую рамку
    */
    if (!g_pConfig->getConfig()->m_vehHud.m_bIsDrawHelpTablet)
    {
        patch::fill(g_gtarpclientside.getAddress(OFFSETS::GTARP_DRAWSPEEDOMETER_DRAWRAMKA), 0x1E, 0x90/*NOP*/);
    }

    /*
        Заменяем ебучий шрифт
    */
    if (g_pConfig->getConfig()->m_samp.m_bIsCustomFont)
    {
        patch::setJump(g_dlSAMP.getAddress(0x6AA7F), uint32_t(&chatFontResetDevice), 5U, true);
        patch::setJump(g_dlSAMP.getAddress(0x6AA3F), uint32_t(&chatFontLostDevice), 5U, true);
        patch::setJump(g_dlSAMP.getAddress(0x6B36C), uint32_t(&chatFontReset), 5U, true);

        ///

        char raw[6]{ '\x8B', '\x35', 0, 0, 0, 0 }; // 8B 35 ? ? ? ? 

        *(DWORD*)(DWORD(raw + 2)) = DWORD(&g_pChatFont); // mov esi, [pFont]

        patch::fill(g_dlSAMP.getAddress(0x66D06), 3U, 0x90);
        patch::setRawThroughJump(g_dlSAMP.getAddress(0x66D06), raw, 6, 5U, true);

        ///

        g_pOrigDirect9CreateFontA = _Direct9CreateFontA(patch::SetCallHook(g_dlSAMP.getAddress(0x6B230), &Direct9CreateFontA__Detour));
    }

    /*
        Делаем ID цвета my cum
    */
    if (g_pConfig->getConfig()->m_samp.m_bIsWhiteID)
    {
        const char* formatPlayerTag = "%s {FFFFFF}(%d)";
        static char s_arrBuffPlayerTag[128 + 8];

        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_RENDERPLAYERTAG_FORMAT), reinterpret_cast<uint32_t>(formatPlayerTag));
        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_RENDERPLAYERTAG_PUSHBUFF_1), reinterpret_cast<uint32_t>(&s_arrBuffPlayerTag));

        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_RENDERPLAYERTAG_PUSHBUFF_2), reinterpret_cast<uint32_t>(&s_arrBuffPlayerTag));
    }

    /*
        Проверяем на обновления
    */
    g_pGameLoopDetour = new PLH::x86Detour(
        reinterpret_cast<PCHAR>(OFFSETS::GTASA_GAMELOOP), reinterpret_cast<PCHAR>(&gameLoopDetourFNC), &g_ui64GameLoopJumpTrampline, g_dis
    );
    g_pGameLoopDetour->hook();

    /*
        Парсим скриншоты по датам
    */
    if (g_pConfig->getConfig()->m_samp.m_bIsSortingScreenshots)
    {
        static char s_fullPathScreenshot[22] = "%s"; // '%s\\screens\\xx-xx-xxxx'
        static char s_pathScreenshot[35]; // '\\screens\\xx-xx-xxxx\\sa-mp-%03i.png'

        /*
        auto t = time(0);
        auto pLocalTime = localtime(&t);
        
        SYSTEMTIME - быстрее в ~7 раз
        */

        SYSTEMTIME timeInfo;
        GetLocalTime(&timeInfo);

        sprintf_s(s_pathScreenshot, "\\screens\\%02d-%02d-%d", timeInfo.wDay, timeInfo.wMonth, timeInfo.wYear);

        strcat(s_fullPathScreenshot, s_pathScreenshot);
        strcat(s_pathScreenshot, "\\sa-mp-%03i.png");

        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_FORMATPATHSCREENSHOT), reinterpret_cast<uint32_t>(s_pathScreenshot));
        patch::setPushOffset(g_dlSAMP.getAddress(OFFSETS::SAMP_FORMATFULLPATHSCREENSHOT), reinterpret_cast<uint32_t>(s_fullPathScreenshot));
    }

    g_pLog->Log("All patching succesful!!");
}

///////////////////////////////

#include <subauth.h>

struct ldrrModuleDLL
{
    uint32_t pad_0[6];
    HANDLE hModule;
};

typedef PDWORD(__fastcall* _LdrpDereferenceModule)(ldrrModuleDLL* a1, PVOID a2);

_LdrpDereferenceModule g_pLdrpDereferenceModule__Jump = nullptr;
PDWORD __fastcall LdrpDereferenceModule__Detour(ldrrModuleDLL* p_this, void* trash)
{
    static bool s_bIsLoaded{};
    if (!s_bIsLoaded && DWORD(p_this->hModule) == g_gtarpclientside.getAddress())
    {
        MainFunction();

        s_bIsLoaded = true;
    }

    return g_pLdrpDereferenceModule__Jump(p_this, trash);
}

///////////////////////////////

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        if (hModule != GetModuleHandleA("!000patchGTARPClientByTim4ukys.ASI")) {
            TerminateProcess(GetCurrentProcess(), -1);
        }

        g_pLog = new CLog(L"!000patchGTARPClientByTim4ukys.log");
        g_pConfig = new CConfig(L"!000patchGTARPClientByTim4ukys.json");
        g_pSAMP = new CSAMP();

        /*
            Давайте расмотрим функцию loadLibraryA. Заходим на MSDN и смотрим(url - https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya).

            Теперь мы знаем, что эта функция находится в Kernel32.dll. Не трате время, а сразу открывайте KernelBase, т.к. с Windows 7(вроде бы) эта не DLL, а затычка, которая
            ссылается на KernelBase.

            Открываем и видим, что теперь мы в LoadLibraryExW. Почему? Потому, что ядро винды использует Юникод(PUNICODE_STRING). Да чё ты делать то будешь, теперь она ссылается на 
            LdrLoadDll (ntdll.dll). 

            Судя по этой теме(url - https://www.cyberforum.ru/blogs/172954/blog5934.html) мы видим, что она принимает четыре аргумента: Flags, Reserved, ModuleFileName, ModuleHandle.
            Отлично! Мы нашли то, что нам нужно! 
            
            HANDLE - это адрес начала адресного пространства DLL. Если он есть - значит DLL загружен. Делаем псевдокод(в ida pro просто по нажатию F5) и смотрим:

            v7 = v11;
            *p_ModuleHandle = *(_DWORD *)(v11 + 0x18);
            LdrpDereferenceModule(v7);

            v7 - Это указатель на v11 => это одно и тоже. Вот и наш Handle. Исходя из протатипа мы видим, что это выходной аргумент и это ед. место где он инициализируется =>
            Мы должны использовать его до того, как он будет где либо использован. В DLLMain как можно видеть он используется => мы воспользуемся им до того, как будет 
            вызван DLLMain. 
            
            0x18 - Явно это смещение => v11 это некая структура. Лично мне вообще похуй что это за структура, мне нужен только hednl, по этому делаем 
            псевдоструктуру. Полная она нам не нужна.

            Ставим callhook LdrpDereferenceModule и в ней будем чекать, та ли DLL. Если та, то ебашим ;)
        */
        auto handleNTDLL = GetModuleHandleA("ntdll.dll");
        auto nAddressLdrLoadDll = DWORD(GetProcAddress(handleNTDLL, "LdrLoadDll"));


        auto callDereferenceModule = patch::FindPattern("ntdll.dll", "\x8B\x4C\x24\x18\x8B\x54\x24\x1C\x8B\x41\x18\x89\x02\xE8", "x???x???x??xxx", nAddressLdrLoadDll);

        callDereferenceModule += 13;
        g_pLog->Log("call address: 0x%X", callDereferenceModule);

        g_pLdrpDereferenceModule__Jump = _LdrpDereferenceModule(patch::SetCallHook(callDereferenceModule, &LdrpDereferenceModule__Detour));
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        SAFE_UNHOOK(g_pDrawWantedDetour);
        SAFE_UNHOOK(g_pGameLoopDetour);
        SAFE_UNHOOK(g_pLoadTextureHudDetour);

        SAFE_DELETE(g_aServerSprite);

        SAFE_DELETE(g_pSAMP);
        SAFE_DELETE(g_pConfig);
        SAFE_DELETE(g_pLog);
    }

    return TRUE;
}