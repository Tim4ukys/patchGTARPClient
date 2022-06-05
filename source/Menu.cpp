#include "pch.h"
#include "Menu.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"

BlurEffect* g_pBlurEffect{};

struct stMenuData {
    bool m_bOpen;

public:
    stMenuData()
        : m_bOpen(false) {
    }
} g_menuData;

static WNDPROC g_pWindowProc;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT __stdcall WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_KEYUP:
    case WM_SYSKEYUP:
        switch (wParam) {
        case VK_F4:
            g_menuData.m_bOpen ^= true;
            break;
        }
        break;
    }

    if (static bool popen = false; g_menuData.m_bOpen) {
        if (!popen) {
            Menu::show_cursor(true);
        }

        popen = true;
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    } else {
        if (popen) {
            popen = false;
            Menu::show_cursor(false);
        }
    }
    return g_pWindowProc(hWnd, msg, wParam, lParam);
}

void Menu::Process() {
    g_pD3D9Hook->onInitDevice += [](LPDIRECT3DDEVICE9 pDevice) {
        //g_Log.Write("hooked device=0x%X; RwD3D9=0x%X", pDevice, RwD3D9GetCurrentD3DDevice());
        g_pBlurEffect = new BlurEffect(pDevice);
        snippets::WinProcHeader::regWinProc(&WndProcHandler, &g_pWindowProc);

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        set_style();

        ImGui_ImplWin32_Init(**(HWND**)0xC17054);
        ImGui_ImplDX9_Init(pDevice);
    };
    g_pD3D9Hook->onLostDevice += [](LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
        g_pBlurEffect->onLostDevice();
    };
    g_pD3D9Hook->onResetDevice += [](LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
        ImGui_ImplDX9_CreateDeviceObjects();
        g_Log.Write("hooked device=0x%X; RwD3D9=0x%X", pDevice, RwD3D9GetCurrentD3DDevice());
        g_pBlurEffect->onResetDevice(pPresentParams);
    };
    g_pD3D9Hook->onPresentEvent += [](IDirect3DDevice9* pDevice, const RECT*, const RECT*, HWND, const RGNDATA*) {
        if (g_menuData.m_bOpen) {
            g_pBlurEffect->Render({0, 0, RsGlobal.maximumWidth, RsGlobal.maximumHeight}, 75.f);
        }
        //ImGui_ImplDX9_NewFrame();
        //ImGui_ImplWin32_NewFrame();
        //ImGui::NewFrame();

        //if (g_menuData.m_bOpen) {
            //ImGui::Begin("test_title1", &g_menuData.m_bOpen);
            //ImGui::Text(u8"Залупа");
            //ImGui::Separator();
            //ImGui::ShowStyleEditor();
            //background();
            //ImGui::End();
        //}

        //ImGui::EndFrame();
        //ImGui::Render();
        //ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    };
    g_onDetachPlugin += []() {
        delete g_pBlurEffect;
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    };
}

void Menu::background() {
    auto         wsize = ImGui::GetWindowSize();
    auto         wpos = ImGui::GetWindowPos();
    //static float s_blurValue{};
    //ImGui::SliderFloat("Blur value:", &s_blurValue, 0.0f, 100.0f);
    g_pBlurEffect->Render({long(wpos.x), long(wpos.y), long(wpos.x + wsize.x), long(wpos.y + wsize.y)}, 75.f);
}

// thx imring
void Menu::show_cursor(bool show) {
    if (show) {
        plugin::patch::Nop(0x541DF5, 5);                        // don't call CControllerConfigManager::AffectPadFromKeyBoard
        plugin::patch::Nop(0x53F417, 5);                        // don't call CPad__getMouseState
        plugin::patch::SetRaw(0x53F41F, "\x33\xC0\x0F\x84", 4); // test eax, eax -> xor eax, eax
                                                                // jl loc_53F526 -> jz loc_53F526
        plugin::patch::PutRetn(0x6194A0);                       // disable RsMouseSetPos (ret)

        ImGui::GetIO().MouseDrawCursor = true;
    } else {
        plugin::patch::SetRaw(0x541DF5, "\xE8\x46\xF3\xFE\xFF", 5); // call CControllerConfigManager::AffectPadFromKeyBoard
        plugin::patch::SetRaw(0x53F417, "\xE8\xB4\x7A\x20\x00", 5); // call CPad__getMouseState
        plugin::patch::SetRaw(0x53F41F, "\x85\xC0\x0F\x8C", 4);     // xor eax, eax -> test eax, eax
                                                                    // jz loc_53F526 -> jl loc_53F526
        plugin::patch::SetUChar(0x6194A0, 0xE9);                    // jmp setup

        ImGui::GetIO().MouseDrawCursor = false;
    }

    (*reinterpret_cast<CMouseControllerState*>(0xB73418)).X = 0.0f;
    (*reinterpret_cast<CMouseControllerState*>(0xB73418)).Y = 0.0f;
    ((void(__cdecl*)())(0x541BD0))(); // CPad::ClearMouseHistory
    ((void(__cdecl*)())(0x541DD0))(); // CPad::UpdatePads
}

void Menu::set_style() {
    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.Fonts->AddFontFromFileTTF(snippets::GetSystemFontPath("Segoe UI Semibold").c_str(), 18.f, 0, io.Fonts->GetGlyphRangesCyrillic());

    auto& style = ImGui::GetStyle();
    auto colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
    colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
    colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
    colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

    //style.Colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.f, 0.f, 0.f);

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
}