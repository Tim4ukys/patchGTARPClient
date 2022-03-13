#include "pch.h"
#include "FastScreenshot.h"

// fix https://forum.gtarp.ru/threads/skrinshoty-utechka-pamjati.973479/

bool g_bIsSortScreenshot;
void saveTexture(std::string szFileName, LPDIRECT3DTEXTURE9 frontBuff, LPDIRECT3DSURFACE9 pTemp, RECT r) {
    ((HRESULT(__stdcall*)(LPCSTR, D3DXIMAGE_FILEFORMAT, LPDIRECT3DSURFACE9, CONST PALETTEENTRY*, CONST RECT*))g_sampBase.getAddress(0xC653A))(
        szFileName.c_str(), D3DXIFF_PNG, pTemp, NULL, &r);
    frontBuff->Release();
}

int GetScreenshotFileName(std::string& FileName) {
    FileName = ((PCHAR(*)())g_sampBase.getAddress(0xC4350))();

    char             Buf[MAX_PATH]{};
    SYSTEMTIME       t;
    FileName += "\\screens\\";
    if (g_bIsSortScreenshot) {
        GetLocalTime(&t);
        char buff[12];
        sprintf(buff, "%d-%02d-%02d\\", t.wYear, t.wMonth, t.wDay);
        FileName += buff;
    }
    int i{};
    do {
        sprintf(Buf, (FileName + "sa-mp-%03i.png").c_str(), i);
        std::filesystem::path p(Buf);
        if (!std::filesystem::exists(p))
            break;
    } while (++i < 1000);
    FileName = Buf;
    return i;
}

void TakeScreenshot() {
    auto            pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(RwD3D9GetCurrentD3DDevice());
    std::string sFileName;

    int iCount = GetScreenshotFileName(sFileName); 
    g_Log.Write("sFileName: %s", sFileName.c_str());

    LPDIRECT3DTEXTURE9 pFrontBuff;
    LPDIRECT3DSURFACE9 pTemp;
    D3DXCreateTexture(pDevice, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1, NULL, D3DFMT_A8R8G8B8,
                      D3DPOOL_SCRATCH, &pFrontBuff);
    pFrontBuff->GetSurfaceLevel(0, &pTemp);

    if (SUCCEEDED(pDevice->GetFrontBufferData(0, pTemp))) {

        if (iCount == 999) {
            g_pSAMP->addChatMessage(0x88'AA'62, "��������� ����� ���-�� ����������! ����������, {FFA500}���������� �����{88AA62}.");
        }

        POINT point{};
        ClientToScreen(**(HWND**)0xC17054, &point);
        RECT rect;
        GetClientRect(**(HWND**)0xC17054, &rect);
        rect.left += point.x; rect.right += point.x;
        rect.top += point.y; rect.bottom += point.y;

        std::ofstream oufstr{sFileName};
        oufstr << "temp";
        oufstr.close();
        std::thread thr(saveTexture, sFileName, pFrontBuff, pTemp, rect);
        thr.detach();
        g_pSAMP->addChatMessage(0x88'AA'62,
                                "�������� �������� {FFA500}sa-mp-%03i.png {88AA62}(�������  {FFA500}��� -> ��������� {88AA62}�� ������ �������� � ����)",
                                iCount);
    } else {
        g_pSAMP->addChatMessage(0x88'AA'62, "�� ������� ��������� ��������.");
    }

    *LPBOOL(g_sampBase.getAddress(0x12DD3C)) = FALSE; // g_bTakeScreenshot
}

void FastScreenshot::Process() {
    g_bIsSortScreenshot = g_Config["samp"]["isSortingScreenshots"].get<bool>();
    plugin::patch::ReplaceFunction(g_sampBase.getAddress(0x74EB0), &TakeScreenshot);
}