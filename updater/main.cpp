#include <windows.h>
#include <wininet.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <thread>
#include <tlhelp32.h>
#include <filesystem>
#include "resource.h"
#include "CRender.h"

#include <zip.h>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

#include "json.hpp"

class DownloadProgress;
bool ProcessRunning(const wchar_t* name);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void setThemeImgui();

std::string getLastVersion();

WNDCLASSEXW g_wc{ sizeof(WNDCLASSEXW) };
HWND g_hWnd{};
CRender* g_pRender;

ImRect g_titleRect;

struct stProgressbarData {
    bool m_bState; // true - render, false - no render ;(
    float m_fProgress; // 0.0 .. 3.0
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    g_pRender = new CRender();

#ifdef NDEBUG
    while (ProcessRunning(L"gta_sa.exe")) {}

    static std::string fileURL = R"(https://github.com/Tim4ukys/patchGTARPClient/releases/download/v)" +
        getLastVersion() + R"(/patchGTARPClientByTim4ukys.zip)";
#endif

    ////////////////////

    g_wc.cbClsExtra = 0;
    g_wc.cbWndExtra = 0;
    g_wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    g_wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    g_wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    g_wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    g_wc.hInstance = hInstance;
    g_wc.lpfnWndProc = &WndProc;
    g_wc.lpszClassName = L"updater";
    g_wc.lpszMenuName = nullptr;
    g_wc.style = CS_VREDRAW | CS_HREDRAW;

    if (!RegisterClassExW(&g_wc))
        return EXIT_FAILURE;

    g_hWnd = CreateWindowW(g_wc.lpszClassName, L"Updater",
        WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 455, 190, nullptr, nullptr, g_wc.hInstance, nullptr);
    if (g_hWnd == INVALID_HANDLE_VALUE)
        return EXIT_FAILURE;

    g_pRender->m_eventInitD3D += [](LPDIRECT3DDEVICE9 pDevice) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        setThemeImgui();

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("vga-8x16.ttf", 16.5f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

        ImGui_ImplWin32_Init(g_hWnd);
        ImGui_ImplDX9_Init(pDevice);
    };
    g_pRender->m_eventDraw += [](LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pRParams) {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        static bool s_bIsClose = true;
        ImGui::Begin("Updater patchGTARPClient | DirectX9 version", &s_bIsClose, flags);
        auto window = ImGui::GetCurrentWindow();
        g_titleRect = window->TitleBarRect();

        

        ImGui::Text(u8"Hello ¸ïòà =))");

        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        if (!s_bIsClose) PostQuitMessage(EXIT_SUCCESS);
    };
    g_pRender->m_eventOnLostDevice += [](LPDIRECT3DDEVICE9 pDevice) { ImGui_ImplDX9_InvalidateDeviceObjects(); };
    g_pRender->m_eventOnResetDevice += [](LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pRPParams) { ImGui_ImplDX9_CreateDeviceObjects(); };

    g_pRender->initD3D(g_hWnd);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
     
    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    delete g_pRender;

    return static_cast<int>(msg.wParam);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg)
    {
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
        g_pRender->draw();
        return 0;
    case WM_SIZE:
        g_pRender->onResize(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_NCHITTEST:
    {
        if (RECT r; GetWindowRect(hWnd, &r)) {
            auto curX = LOWORD(lParam) - r.left;
            auto curY = HIWORD(lParam) - r.top;
            ImVec2 close_button;
            {
                auto& style = ImGui::GetStyle();
                float pad_r = style.FramePadding.x;
                float button_sz = ImGui::GetCurrentContext()->FontSize;

                pad_r += button_sz;
                close_button = ImVec2(g_titleRect.Max.x - pad_r - style.FramePadding.x, g_titleRect.Min.y);
            }
            if (curY < g_titleRect.GetHeight() && curX < g_titleRect.GetWidth()
                && curX < close_button.x)
                return HTCAPTION;
        }
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void setThemeImgui() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    //style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    //style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    style.GrabRounding = style.FrameRounding = 2.3f;
}

std::string getLastVersion() {
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

    return nlohmann::json::parse(resolve)["vers"].get<std::string>();
}

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
        //SendMessage(g_hProgress, PBM_SETPOS, ULONG(ulProgressMax > 0 ? ulProgress / ulProgressMax * 100 : 0), 0);
        return S_OK;
    }
};