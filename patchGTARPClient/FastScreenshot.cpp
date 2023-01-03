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
#include "pch.h"
#include "FastScreenshot.h"
#include "resource.h"
#include "offsets.hpp"

// fix https://forum.gtarp.ru/threads/skrinshoty-utechka-pamjati.973479/

//#define DEBUG_SOUND

DWORD FastScreenshot::s_hTakeSound;

struct stImgFormat {
    D3DXIMAGE_FILEFORMAT m_d3dFormat;
    const char*          m_pFormat;
} g_imgFormat;

static decltype(D3DXSaveSurfaceToFileA)* s_fncSaveSurfaceToFileA;
static decltype(D3DXCreateTexture)*      s_fncCreateTexture;

void saveTexture(std::shared_ptr<std::string> szFileName, LPDIRECT3DTEXTURE9 frontBuff,
                 LPDIRECT3DSURFACE9 pTemp, std::shared_ptr<RECT> r) {
    static std::mutex           s_log{};
    std::lock_guard<std::mutex> lock{s_log};

    s_fncSaveSurfaceToFileA(szFileName->c_str(), g_imgFormat.m_d3dFormat, pTemp, NULL, r.get());
    pTemp->Release();
    frontBuff->Release();
}

int GetScreenshotFileName(std::string& FileName) {
    FileName = g_sampBase.getAddr<PCHAR (*)()>(0xC4350)();

    char              Buf[MAX_PATH]{};
    static SYSTEMTIME s_time;
    if (static bool s_bIsInit{};
        !s_bIsInit) {
        GetLocalTime(&s_time);
        s_bIsInit = true;
    }
    
    FileName += "\\screens\\";
    if (g_Config["samp"]["isSortingScreenshots"].get<bool>()) {
        //GetLocalTime(&t);
        char buff[12];
        sprintf(buff, "%d-%02d-%02d\\", s_time.wYear, s_time.wMonth, s_time.wDay);
        FileName += buff;
    }
    FileName += "sa-mp-%03i.%s";
    int i{};
    do {
        sprintf(Buf, FileName.c_str(), i, g_imgFormat.m_pFormat);
        std::filesystem::path p(Buf);
        if (!std::filesystem::exists(p))
            break;
    } while (++i < 1000);
    FileName = Buf;
    return i;
}

inline std::pair<UINT, UINT> getResolution() {
    RECT r;
    GetClientRect(*(HWND*)0xC97C1C, &r);
    return {r.right - r.left, r.bottom - r.top};
}

void FastScreenshot::hkTakeScreenshot() {
#ifdef DEBUG_SOUND
    BASS_ChannelSetAttribute(g_hTakeSound, BASS_ATTRIB_VOL, 1.0f);
    BASS_ChannelPlay(g_hTakeSound, TRUE);
    *LPBOOL(g_sampBase.getAddress(0x12DD3C)) = FALSE; // g_bTakeScreenshot
#else
    auto                         pDevice = *g_sampBase.getAddr<LPDIRECT3DDEVICE9*>(OFFSETS::SAMP::D3D9DEVICE);
    std::shared_ptr<std::string> sFileName = std::make_shared<std::string>();
    int                          iCount = GetScreenshotFileName(*sFileName);

    LPDIRECT3DTEXTURE9 pFrontBuff{};
    LPDIRECT3DSURFACE9 pSurfFrontTexture{};
    auto [resX, resY] = getResolution();
    s_fncCreateTexture(pDevice,
                       UINT(std::roundf((float)resX / 4.0f)) * 4, UINT(std::roundf((float)resY / 4.0f)) * 4,
                       1, NULL, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &pFrontBuff);
    pDevice->CreateOffscreenPlainSurface(resX, resY, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH,
                                         &pSurfFrontTexture, nullptr);

    try {
        if (!pFrontBuff)
            throw std::string("pFrontBuff == nullptr");
        pFrontBuff->GetSurfaceLevel(0, &pSurfFrontTexture);
        if (!pSurfFrontTexture)
            throw std::string("pSurfFrontTexture == nullptr");

        if (SUCCEEDED(pDevice->GetFrontBufferData(0, pSurfFrontTexture))) {

            if (iCount == 999) {
                g_pSAMP->addChatMessage(0x88'AA'62, "Достигнут лимит кол-ва скриншотов! Пожалуйста, {FFA500}освободите папку{88AA62}.");
            }

            POINT point{};
            std::shared_ptr<RECT> rect = std::make_shared<RECT>();
            ClientToScreen(**(HWND**)0xC17054, &point);
            GetClientRect(**(HWND**)0xC17054, rect.get());
            rect->left += point.x;
            rect->right += point.x;
            rect->top += point.y;
            rect->bottom += point.y;

            std::ofstream oufstr{*sFileName};
            oufstr << "temp";
            oufstr.close();
            std::thread thr(saveTexture, sFileName, pFrontBuff, pSurfFrontTexture, rect);
            thr.detach();
            g_pSAMP->addChatMessage(0x88'AA'62,
                                    "Скриншот сохранен {FFA500}sa-mp-%03i.%s {88AA62}(нажмите  {FFA500}ПКМ -> Скриншоты {88AA62}на иконке лаунчера в трее)",
                                    iCount, g_imgFormat.m_pFormat);
            if (g_Config["samp"]["isPlaySoundAfterMakeScreenshot"].get<bool>()) {
                BASS_ChannelSetAttribute(s_hTakeSound, BASS_ATTRIB_VOL, 1.0f);
                BASS_ChannelPlay(s_hTakeSound, TRUE);
            }
        } else {
            throw std::string("GetFrontBufferData != SUCCEEDED");
        }

        *g_sampBase.getAddr<LPBOOL>(OFFSETS::SAMP::TAKESCREENSHOT) = FALSE; // g_bTakeScreenshot
    } catch (const std::string& text) {
        g_pSAMP->addChatMessage(0x88'AA'62, "Не удалось сохранить скриншот: {808080}%s", text.c_str());
        SAFE_RELEASE(pSurfFrontTexture);
        SAFE_RELEASE(pFrontBuff);
    }
#endif
}


void FastScreenshot::turnOn() {
    //std::uint8_t bytes[5]{ 0xE9 };
    //*reinterpret_cast<std::uint32_t*>(bytes + 1) = (std::uint32_t)&hkTakeScreenshot;
    //patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FNCTAKESCREENSHOT), bytes);
    plugin::patch::ReplaceFunction(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FNCTAKESCREENSHOT), &hkTakeScreenshot);
}

void FastScreenshot::turnOff() {
    patch::setBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FNCTAKESCREENSHOT), m_byteOrig);
}

void FastScreenshot::init() {
    // Load config
    if (auto ext = g_Config["samp"]["formatScreenshotIMG"].get<std::string>();
        ext == "JPEG") {
        g_imgFormat.m_d3dFormat = D3DXIFF_JPG;
        g_imgFormat.m_pFormat = "jpg";
    } else if (ext == "TGA") {
        g_imgFormat.m_d3dFormat = D3DXIFF_TGA;
        g_imgFormat.m_pFormat = "tga";
    } else {
        g_imgFormat.m_d3dFormat = D3DXIFF_PNG;
        g_imgFormat.m_pFormat = "png";
    }

    g_onInitAudio += []() {
        struct {
            PCHAR  m_pData;
            size_t m_szLength;
        } g_takeScreenshot;

        HRSRC   rc = FindResourceA(g_DLLModule, MAKEINTRESOURCEA(IDR_WAVE1), MAKEINTRESOURCEA(RT_RCDATA));
        HGLOBAL rcData = LoadResource(g_DLLModule, rc);
        g_takeScreenshot.m_szLength = SizeofResource(g_DLLModule, rc);
        g_takeScreenshot.m_pData = static_cast<char*>(LockResource(rcData));
        s_hTakeSound = BASS_StreamCreateFile(TRUE, g_takeScreenshot.m_pData, 0, g_takeScreenshot.m_szLength, 0);
#ifdef DEBUG_SOUND
        g_Log << "Sound is init, epta";
#endif
    };

    s_fncSaveSurfaceToFileA = g_D3DX9_25.getFnc<decltype(D3DXSaveSurfaceToFileA)>("D3DXSaveSurfaceToFileA");
    s_fncCreateTexture = g_D3DX9_25.getFnc<decltype(D3DXCreateTexture)>("D3DXCreateTexture");

    // ----

    patch::getBytes(g_sampBase.getAddr<std::uintptr_t>(OFFSETS::SAMP::FNCTAKESCREENSHOT), m_byteOrig);

    // ----

    if (m_bState = g_Config["samp"]["isMakeQuickScreenshot"].get<bool>())
        turnOn();
}
FastScreenshot::~FastScreenshot() {
    BASS_StreamFree(s_hTakeSound);
    if (m_bState)
        turnOff();
}