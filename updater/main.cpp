#include <windows.h>
#include <wininet.h>
#include <CommCtrl.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <thread>
#include <filesystem>

#include <zip.h>

#include "json.hpp"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WNDCLASSEX g_wc{ sizeof(WNDCLASSEX) };
HWND g_hWnd{};
std::string g_sUpText = "Статус:";
HWND g_hProgress;

class DownloadProgress : public IBindStatusCallback {
public:
    HRESULT __stdcall QueryInterface(const IID&, void**) {
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef(void) {
        return 1;
    }
    ULONG STDMETHODCALLTYPE Release(void) {
        return 1;
    }
    HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD dwReserved, IBinding* pib) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE GetPriority(LONG* pnPriority) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE OnLowResource(DWORD reserved) {
        return S_OK;
    }
    virtual HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT hresult, LPCWSTR szError) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD* grfBINDF, BINDINFO* pbindinfo) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC* pformatetc, STGMEDIUM* pstgmed) {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID riid, IUnknown* punk) {
        return E_NOTIMPL;
    }

    virtual HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
    {
        SendMessage(g_hProgress, PBM_SETPOS, ULONG(ulProgressMax > 0 ? ulProgress / ulProgressMax * 100 : 0), 0);
        return S_OK;
    }
};

#include <tlhelp32.h>

bool ProcessRunning(const wchar_t* name)
{
    HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (SnapShot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32 procEntry;
    procEntry.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(SnapShot, &procEntry))
        return false;

    do
    {
        if (wcscmp(procEntry.szExeFile, name) == NULL)
            return true;
    } while (Process32Next(SnapShot, &procEntry));

    return false;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    while (ProcessRunning(L"gta_sa.exe")) {}

    HINTERNET interwebs = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
    HINTERNET urlFile = InternetOpenUrlA(interwebs, "https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json",
        NULL, NULL, NULL, NULL);

    std::string resolve;
    char* buff = new char[512];
    DWORD bytesRead;
    do {
        InternetReadFile(urlFile, buff, 512, &bytesRead);
        resolve.append(buff, bytesRead);
    } while (bytesRead);
    delete[] buff;

    InternetCloseHandle(interwebs);
    InternetCloseHandle(urlFile);

    //auto it = resolve.find("\r\n\r\n");
    //if (it == resolve.npos)
        //return EXIT_FAILURE;
    //resolve.erase(0, it + 4);
    nlohmann::json j_res = nlohmann::json::parse(resolve);
    static std::string fileURL = R"(https://github.com/Tim4ukys/patchGTARPClient/releases/download/v)" +
        j_res["vers"].get<std::string>() + R"(/patchGTARPClientByTim4ukys.zip)";

    MSG msg{};
    g_wc.cbClsExtra = 0;
    g_wc.cbWndExtra = 0;
    g_wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    g_wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    g_wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    g_wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    g_wc.hInstance = hInstance;
    g_wc.lpfnWndProc = &WndProc;
    g_wc.lpszClassName = TEXT("UpdateMyPatch");
    g_wc.lpszMenuName = nullptr;
    g_wc.style = CS_VREDRAW | CS_HREDRAW;

    if (!RegisterClassEx(&g_wc))
        return EXIT_FAILURE;

    g_hWnd = CreateWindow(g_wc.lpszClassName, TEXT("Updater patchGTARPClient"), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        0, 0, 355, 130, nullptr, nullptr, g_wc.hInstance, nullptr);
    if (g_hWnd == INVALID_HANDLE_VALUE)
        return EXIT_FAILURE;

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    auto thrProcess = []() {
        g_sUpText = "Статус: cкачивание архива";
        InvalidateRect(g_hWnd, NULL, FALSE);
        std::filesystem::path tempDir("./temp");
        if (!std::filesystem::exists(tempDir)) {
            std::filesystem::create_directory(tempDir);
        }
        else if (!std::filesystem::is_directory(tempDir)) {
            std::filesystem::remove(tempDir);
            std::filesystem::create_directory(tempDir);
        }

        DownloadProgress progress;
        URLDownloadToFileA(NULL, fileURL.c_str(), "temp\\arhiv.zip", NULL, &progress);

        ////////////////////////////////

        g_sUpText = "Статус: разархивация плагина";
        InvalidateRect(g_hWnd, NULL, FALSE);
        int err{};
        zip* p = zip_open("temp\\arhiv.zip", 0, &err);
        SendMessage(g_hProgress, PBM_SETPOS, ULONG(0.5 * 100) + 100, 0);
        zip_stat_t st;
        zip_stat_init(&st);
        zip_stat(p, "!000patchGTARPClientByTim4ukys.ASI", 0, &st);
        SendMessage(g_hProgress, PBM_SETPOS, ULONG(0.25 * 100) + 100, 0);

        PCHAR cont = new char[st.size];
        zip_file* f = zip_fopen(p, "!000patchGTARPClientByTim4ukys.ASI", 0);
        zip_fread(f, cont, st.size);
        SendMessage(g_hProgress, PBM_SETPOS, ULONG(0.40 * 100) + 100, 0);
        zip_fclose(f);
        std::ofstream("!000patchGTARPClientByTim4ukys.ASI", std::ios_base::binary).write(cont, st.size);
        delete[] cont;
        SendMessage(g_hProgress, PBM_SETPOS, ULONG(0.50 * 100) + 100, 0);

        zip_stat_init(&st);
        zip_stat(p, "gtarp_clientside.asi", 0, &st);
        SendMessage(g_hProgress, PBM_SETPOS, ULONG(0.75 * 100) + 100, 0);
        cont = new char[st.size];
        f = zip_fopen(p, "gtarp_clientside.asi", 0);
        zip_fread(f, cont, st.size);
        SendMessage(g_hProgress, PBM_SETPOS, ULONG(0.90 * 100) + 100, 0);
        zip_fclose(f);
        std::ofstream("gtarp_clientside.asi", std::ios_base::binary).write(cont, st.size);
        delete[] cont;
        SendMessage(g_hProgress, PBM_SETPOS, ULONG(1.00 * 100) + 100, 0);

        zip_close(p);

        ////////////////////////////////

        g_sUpText = "Статус: удаление временных файлов";
        InvalidateRect(g_hWnd, NULL, FALSE);
        std::filesystem::remove_all(tempDir);
        SendMessage(g_hProgress, PBM_SETPOS, ULONG(1.00 * 100) + 200, 0);

        ////////////////////////////////

        g_sUpText = "Статус: обновление успешно завершено";
        InvalidateRect(g_hWnd, NULL, FALSE);
        Sleep(3000);
        TerminateProcess(GetCurrentProcess(), -1);
    };
    std::jthread thr{ thrProcess };
    thr.detach();

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //SendMessage(g_hProgress, PBM_STEPIT, 0, 0);
    switch (uMsg)
    {
    case WM_CREATE:
    {
        RECT r;
        GetClientRect(hWnd, &r);
        g_hProgress = CreateWindowEx(0, PROGRESS_CLASS, nullptr, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 5,
            r.bottom - 25, r.right - r.left - 10, 20, hWnd, NULL, g_wc.hInstance, NULL);
        SendMessage(g_hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 300));
        SendMessage(g_hProgress, PBM_SETSTEP, 1, 0);
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(EXIT_SUCCESS);
        return 0;
    }
    case WM_ERASEBKGND:
    {
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        auto hdc = BeginPaint(hWnd, &ps);
        TextOutA(hdc, 10, 20, g_sUpText.c_str(), g_sUpText.length());
        EndPaint(hWnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}