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
#include "Menu.h"
#include "Updater.h"
extern Updater g_Updater;

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"

struct stServerIcon {
    CSprite2d m_Sprite;
    float     m_fIconPos[2];
    float     m_fIconSize[2];
    bool      m_bState;
};
extern stServerIcon g_serverIcon;

extern void FastScreenshot__updateFormat(const char* l);

//#define DEBUG_NEWS

struct stMenuData {
    bool        m_bOpen;
    int         m_iSelected;
    const char* m_pSelected;

    std::string                                                   m_sOldVersion;
    std::vector<std::pair<std::string, std::vector<std::string>>> m_arrNews;

public:
    stMenuData()
        : m_bOpen(false),
          m_iSelected(1),
          m_arrNews() {
    }
} g_menuData;

static WNDPROC g_pWindowProc;
static std::shared_ptr<PLH::x86Detour> g_wndProcHangler;
extern IMGUI_IMPL_API LRESULT          ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT __stdcall WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (static bool popen = false; g_menuData.m_bOpen) {
        if (!popen) {
            Menu::show_cursor(true);
        }

        switch (msg) {
        case WM_KEYDOWN:
            switch (wParam) {
            case VK_ESCAPE:
                if (!*PBYTE(0xB7CB49)) { // [byte] Menu show
                    g_menuData.m_bOpen = false;
                    return 0;
                }
                break;
            }
            break;
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

enum eTitles {
    eTitles_News = 0,
    eTitles_SAMP,
    eTitles_GTASA,
    eTitles_GTARP,

    eTitles_INFO
};

void Menu::title_menu() {
    std::string_view titles[]{
        "SA-MP", "GTA SA", "GTA RP"
    };
    // constexpr float height_title = 40.f;
    // ImGui::GetWindow
    auto len = ImGui::GetCurrentWindow()->Size.x / ARRAYSIZE(titles);
    if (ImGui::BeginTabBar("mainTitles")) {
        for (size_t i{}; i < ARRAYSIZE(titles); ++i) {
            ImGui::SetNextItemWidth(len);
            if (ImGui::TabItemButton(titles[i].data())) {
                g_menuData.m_iSelected = static_cast<int>(i + 1);
                g_menuData.m_pSelected = titles[i].data();
            }
        }
        ImGui::EndTabBar();
    }
}

void Menu::init() {
    g_onInitSamp += []() {
        g_pSAMP->cmdRect("patch_open",
                         [](const char* param) {
                             g_menuData.m_bOpen ^= true;
                         });
        g_pSAMP->addChatMessage(0x99'00'00, "[{FF9900}patchGTARPClient{990000}] {990000}Напоминаем{FF9900}, что команда открытия меню: {606060}/patch_open");
    };

    g_pD3D9Hook->onInitDevice += [](LPDIRECT3DDEVICE9 pDevice) {
        //g_Log.Write("hooked device=0x%X; RwD3D9=0x%X", pDevice, RwD3D9GetCurrentD3DDevice());
        g_wndProcHangler = snippets::WinProcHeader::regWinProc(&WndProcHandler, &g_pWindowProc);

        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        auto  conf = ImFontConfig();
        conf.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
        conf.SizePixels = 16;
        io.Fonts->AddFontDefault(&conf);

        set_style();

        auto fnc_downloadNews = []() {
            using json = nlohmann::json;
            auto j = json::parse(client::downloadStringFromURL("https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/news.json"));

            auto oldVers = snippets::versionParse(g_menuData.m_sOldVersion);
            //g_Log.Write("oldVers: %d.%d.%d", oldVers[0], oldVers[1], oldVers[2]);

            for (json::iterator i = j.begin(); i != j.end(); ++i) {;
                auto key = snippets::versionParse(i.key());
                if (key[0] <= Updater::MAJ::value && key[0] > oldVers[0] 
                    || ((key[0] <= Updater::MAJ::value && key[1] <= Updater::MIN::value) && (key[0] == oldVers[0] && key[1] > oldVers[1])) 
                    || ((key[0] <= Updater::MAJ::value && key[1] <= Updater::MIN::value && key[2] <= Updater::PATCH::value) 
                        && (key[0] == oldVers[0] && key[1] == oldVers[1] && key[2] > oldVers[2]))) 
                {
                    std::pair<std::string, std::vector<std::string>> t;
                    t.first = i.key();
                    auto& arr = i.value();
                    t.second.resize(arr.size());
                    //g_Log.Write("key=%s;arr.size()=%d", key.c_str(), j[i.key()].size());
                    for (size_t i{}; i < arr.size(); ++i) {
                        t.second[i] = arr[i].get<std::string>();
                    }
                    g_menuData.m_arrNews.push_back(t);
                }
            }

            g_menuData.m_iSelected = eTitles_News;
        };

        #ifndef DEBUG_NEWS
        if (!g_Config["vers"].is_string()) {
            if (!g_Config["vers"].is_null())
                g_Config.getJSON().erase("vers");
            g_Config["vers"] = Updater::VERSION;
            g_menuData.m_sOldVersion = "8.0.0";
            g_Config.saveFile();
            fnc_downloadNews();
        } else if (auto&& [mj, mn, pt] = snippets::versionParse(g_Config["vers"]);
                   g_Updater.check(mj, mn, pt))  {
            g_menuData.m_sOldVersion = g_Config["vers"];
            g_Config["vers"] = Updater::VERSION;
            g_Config.saveFile();
        #else 
        {
            g_menuData.m_sOldVersion = "8.0.0";
        #endif
            fnc_downloadNews();
        }
        #ifndef DEBUG_NEWS
        else {
            g_menuData.m_iSelected = eTitles_INFO;
        }
        #endif

        ImGui_ImplWin32_Init(**(HWND**)0xC17054);
        ImGui_ImplDX9_Init(pDevice);
    };
    g_pD3D9Hook->onLostDevice += [](LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    };
    g_pD3D9Hook->onResetDevice += [](LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
        ImGui_ImplDX9_CreateDeviceObjects();
        g_Log.Write("hooked device=0x%X; RwD3D9=0x%X", pDevice, RwD3D9GetCurrentD3DDevice());
    };
    g_pD3D9Hook->onPresentEvent += [](IDirect3DDevice9* pDevice, const RECT*, const RECT*, HWND, const RGNDATA*) {
        if (!g_menuData.m_bOpen)
            return;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize({670.f, 342.f}, ImGuiCond_Appearing);
        ImGui::SetNextWindowSizeConstraints({670.f, 342.f}, {FLT_MAX, FLT_MAX});
        ImGui::SetNextWindowPos({plugin::screen::GetScreenCenterX(), plugin::screen::GetScreenCenterY()}, ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::Begin("patchGTARPClient", &g_menuData.m_bOpen, ImGuiWindowFlags_NoSavedSettings);
        title_menu();

        auto checkbox = [](const char* label, nlohmann::json& j, const char* keyModule = nullptr,
                           void (*fnc)(bool) = nullptr, const char* desc = nullptr) {
            if (ImGui::Checkbox(label, j.get_ptr<bool*>())) {
                if (keyModule)
                    g_modules[std::string(keyModule)]->Toggle();
                if (fnc)
                    fnc(j.get<bool>());
                g_Config.saveFile();
            }
            if (desc && ImGui::IsItemHovered())
                ImGui::SetTooltip(desc);
        };
        auto textInput = [](const char* label, nlohmann::json& j, const char* desc = nullptr) {
            if (ImGui::InputText(label, j.get_ptr<std::string*>())) {
                g_Config.saveFile();
            }
            if (desc && ImGui::IsItemHovered())
                ImGui::SetTooltip(desc);
        };
        auto doubleInput = [](const char* label, nlohmann::json& j, void (*fnc)(float) = nullptr, const char* desc = nullptr) {
            if (ImGui::InputDouble(label, j.get_ptr<double*>())) {
                if (fnc)
                    fnc(j.get<float>());
                g_Config.saveFile();
            }
            if (desc && ImGui::IsItemHovered())
                ImGui::SetTooltip(desc);
        };
        auto combo = [](const char* label, nlohmann::json& j, const char const* params[], int count, void (*fnc)(const char*) = nullptr, const char* desc = nullptr) {
            static std::map<const char*, int> s_labels;
            if (s_labels.find(label) == s_labels.end()) {
                if (auto find_str = std::find_if(params, params + count, [&](const char* fstr) -> bool { return j.get<std::string>() == fstr; });
                    find_str != params + count) {
                    s_labels.emplace(label, int(DWORD(find_str) - DWORD(params)) / int(sizeof(sizeof(const char*))));
                } else {
                    s_labels.emplace(label, 0);
                }
            }
            if (ImGui::Combo(label, &s_labels[label], params, count)) {
                j = params[s_labels[label]];
                if (fnc)
                    fnc(params[s_labels[label]]);
                g_Config.saveFile();
            }
            if (desc && ImGui::IsItemHovered())
                ImGui::SetTooltip(desc);
        };

        constexpr auto TAB_SIZE = 20;

        if (g_menuData.m_pSelected)
            ImGui::BeginChild(g_menuData.m_pSelected, {-1, ImGui::GetWindowHeight() - 85.f}, false, ImGuiWindowFlags_NoBackground);
        switch (g_menuData.m_iSelected) {
        case eTitles_INFO:
            render_doska();
            ImGui::Separator();
            ImGui::Text(u8"Сайт проекта GitHub: github.com/Tim4ukys/patchGTARPClient\n"
                        u8"DonationAlerts: www.donationalerts.com/r/tim4ukys");
            /*ImGui::TextWrapped(u8"Донат нужен для аренды облака, с помощью которого работает tg-bot, "
                        u8"а в будущем он нужен будет для работы и самого плагина. "
                        u8"Цена аренды 270-320 рублей. Я не хочу отбирать деньги просто так, и ради этого "
                        u8"я создал цель сбора. Если нужная сумма набирается, то этот сбор закрывается, "
                        u8"и где-то через месяц я создаю новый. "
                        u8"Все прозрачно. Вы сами сможете видеть, сколько ещё нужно до цели. Главное, чтобы "
                        u8"при донате Вы не забывали выбрать \"цель\" доната. (Host: RuVDS)");
            ImGui::TextWrapped(u8"Буду рад и 10 рублям. Весь этот проект(я про патч) создавался не из-за "
                               u8"каких-то карыстных побуждений, а на голом энтузиазме. Спасибо за понимание.");*/
            break;
        case eTitles_News:
            ImGui::Text(u8"Все изменения, которые произошли пока Вы не обновляли плагин:");
            for (const auto& news : g_menuData.m_arrNews) {
                ImGui::Text("%s:", news.first.c_str());
                for (size_t i{}; i < news.second.size(); ++i) {
                    ImGui::Text("\t%d) %s", i + 1, news.second[i].c_str());
                }
            }
            break;
        case eTitles_SAMP:
            checkbox(u8"Новое меню на F1", g_Config["samp"]["isCustomF1"], "CustomHelp");
            checkbox(u8"Белые ID", g_Config["samp"]["isWhiteID"], "WhiteID", nullptr,
                     u8"ID легче читать. Очень полезно в тех случаях когда чел, на которого\n"
                     u8"нужно написать донос в репорт, одел маску.");

            checkbox(u8"Кастомный шрифт в чате", g_Config["samp"]["isCustomFont"], "CustomFont", 
                     [](bool) { g_pSAMP->redraw(); }, u8"Заменяет стандартный шрифт");
            if (g_Config["samp"]["isCustomFont"].get<bool>()) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE);
                textInput(u8"Название шрифта", g_Config["samp"]["fontFaceName"],
                          u8"Например: \tSegoe UI Light\n"
                          u8"\t\t\t\t\t\tComic Sans MS\n"
                          u8"\t\t\t\t\t\tJetBrains Mono");
            }
            checkbox(u8"Быстрые скриншоты", g_Config["samp"]["isMakeQuickScreenshot"], "FastScreenshot", nullptr,
                     u8"Ускоряет создание скриншотов в несколько раз.");
            if (g_Config["samp"]["isMakeQuickScreenshot"].get<bool>()) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE);
                checkbox(u8"Звук создания скриншота", g_Config["samp"]["isPlaySoundAfterMakeScreenshot"], nullptr, nullptr,
                         u8"После создания скриншота будет проигрываться мелодия(копипаста из STEAM).");

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE);

                const char* fmtIMG[]{"PNG", "JPEG", "TGA"};
                combo(u8"Формат изображения", g_Config["samp"]["formatScreenshotIMG"], fmtIMG, ARRAYSIZE(fmtIMG),
                      [](const char* f) { FastScreenshot__updateFormat(f); },
                      u8"Формат, в котором будут сохраняться скриншоты");
            }

            checkbox(u8"Сортировка скриншотов по папкам", g_Config["samp"]["isSortingScreenshots"], "SortScreenshot", nullptr,
                     u8"Скриншоты будут сохраняться в папку \"screens\\YYYY-MM-DD\"\n"
                     u8"Например:\t screens\\2005-11-29");

            break;
        case eTitles_GTASA:
            checkbox(u8"Turn Fucking Radio Off", g_Config["gtasa"]["tfro"], "TFRO", nullptr,
                     u8"При посадке в авто радио будет автоматически выключенно.\n"
                     u8"Автор оригинала: NarutoUA (blast.hk/members/2504)");
            break;
        case eTitles_GTARP:
            checkbox(u8"Car hotkey table", g_Config["vehicleHud"]["isDrawHelpTablet"], "DelCarTable", nullptr,
                     u8"Подсказка с клавишами, которая появляется, когда персонаж садится в авто.");
            checkbox(u8"Убрать наледь на окне при езде", g_Config["vehicleHud"]["isDisableSnowWindow"], "DisableSnowWindow", nullptr);
            checkbox(u8"Старый худ", g_Config["oldHud"]["hud"], "OldHUD", nullptr,
                     u8"Возвращает старый худ из GTA SA");
            if (g_Config["oldHud"]["hud"].get<bool>()) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE);
                checkbox(u8"Часовой пояс на часах МСК", g_Config["clock"]["fixTimeZone"], nullptr, nullptr,
                         u8"Подстраивает время на часах относительно московского часового пояса.");

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE);
                checkbox(u8"Иконка сервера", g_Config["serverIcon"]["state"], nullptr, nullptr /*[](bool a) { g_serverIcon.m_bState = a; }*/,
                         u8"Даёт возможность изменять иконку сервера(его позицию и ВКЛ/ВЫКЛ)");
                if (g_Config["serverIcon"]["state"].get<bool>()) {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE * 2);
                    doubleInput(u8"X", g_Config["serverIcon"]["x"], [](float a) { g_serverIcon.m_fIconPos[0] = a; });
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE * 2);
                    doubleInput(u8"Y", g_Config["serverIcon"]["y"], [](float a) { g_serverIcon.m_fIconPos[1] = a; });
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE * 2);
                    doubleInput(u8"Ширина", g_Config["serverIcon"]["width"], [](float a) { g_serverIcon.m_fIconSize[0] = a; });
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE * 2);
                    doubleInput(u8"Высота", g_Config["serverIcon"]["height"], [](float a) { g_serverIcon.m_fIconSize[1] = a; });
                }
            }
            checkbox(u8"Старый радар", g_Config["oldHud"]["radar"], nullptr, nullptr,
                     u8"Возвращает радар из GTA SA");
            textInput(u8"Путь до \"HUD.TXD\"", g_Config["oldHud"]["pathToTXDhud"],
                      u8"Путь относительно корневой папки игры.\n"
                      u8"Например(Как в настройках->Как в итоге):\n\t"
                      u8R"(%%s\CustomSAA2\hud.txd -> G:\gta rp\CustomSAA2\hud.txd)"
                      u8"\n\nЧтобы путь до файла был стандартным, следует написать: NONE");
            checkbox(u8"Свой экран загрузки на север", g_Config["customScreen"]["state"], nullptr, nullptr,
                     u8"Заменяет время, погоду, местоположение камеры и куда она смотрит\n"
                     u8"при коннекте к серверу.");
            /*
            if (g_Config["customScreen"]["state"].get<bool>()) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE);
                ImGui::BeginChild("customScreen", {0.f, 55.f});
                for (auto& it : g_Config["customScreen"]["screens"].items()) {
                    float cum[3];
                    cum[0] = it.value()["camera"][0].get<float>();
                    cum[1] = it.value()["camera"][1].get<float>();
                    cum[2] = it.value()["camera"][2].get<float>();
                    if (ImGui::InputFloat3("Camera", cum)) {
                        it.value()["camera"][0] = cum[0];
                        it.value()["camera"][1] = cum[1];
                        it.value()["camera"][2] = cum[2];
                        g_Config.saveFile();
                    }
                    
                }
                ImGui::EndChild();
            }
            */
            if (g_Config["customScreen"]["state"].get<bool>()) {
                //auto pCustConScreen = dynamic_cast<CustomConnectScreen*>(g_modules["CustomConnectScreen"].get());

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TAB_SIZE);
                ImGui::SetNextWindowSizeConstraints({670.f - TAB_SIZE - 16.f*2, 165.f}, {670.f - TAB_SIZE - 16.f*2, 342.f});
                ImGui::BeginChild("customScreen", ImVec2(0, 0), true);
                int i{1};
                for (auto& [key, value] : g_Config["customScreen"]["screens"].items()) {
                    char tmp[14]{};
                    float cum[3]{value["camera"][0].get<float>(), value["camera"][1].get<float>(), value["camera"][2].get<float>()};

                    sprintf(tmp, "Camera[%d]", i);
                    if (ImGui::InputFloat3(tmp, cum)) {
                        value["camera"][0] = cum[0]; value["camera"][1] = cum[1]; value["camera"][2] = cum[2];
                        g_Config.saveFile();
                    }

                    float point[3]{value["point"][0].get<float>(), value["point"][1].get<float>(), value["point"][2].get<float>()};
                    sprintf(tmp, "Point[%d]", i);
                    if (ImGui::InputFloat3(tmp, point)) {
                        value["point"][0] = point[0]; value["point"][1] = point[1]; value["point"][2] = point[2];
                        g_Config.saveFile();
                    }

                    ImGui::BeginChild(("customScreen_" + std::to_string(i)).c_str(), {0.f, 85.f}, true);
                    for (size_t i{1}; i <= value["time"].size(); ++i) {
                        char buff[15];
                        sprintf(buff, "time[%d]", i);
                        if (ImGui::SliderInt(buff, (int*)value["time"][i-1].get_ptr<long long*>(), 0, 23))
                            g_Config.saveFile();
                        sprintf(buff, "weather[%d]", i);
                        if (ImGui::SliderInt(buff, (int*)value["weather"][i-1].get_ptr<long long*>(), 0, 45))
                            g_Config.saveFile();
                        if (i > 1 && ImGui::Button((u8"Удалить time и weather " + std::to_string(i)).c_str())) {
                            value["time"].erase(i-1);
                            value["weather"].erase(i-1);
                            g_Config.saveFile();
                            break;
                        }
                        ImGui::Separator();
                    }
                    if (ImGui::Button(u8"Добавить time и weather")) {
                        value["time"].push_back(2);
                        value["weather"].push_back(0);
                        g_Config.saveFile();
                    }
                    ImGui::EndChild();
                    
                    if (ImGui::Button((u8"Удалить screen " + std::to_string(i)).c_str())) {
                        g_Config["customScreen"]["screens"].erase(i-1);
                        g_Config.saveFile();
                        break;     
                    }

                    ImGui::Separator(); ++i;
                }

                if (ImGui::Button(u8"Добавить screen")) {
                    g_Config["customScreen"]["screens"].push_back(R"({
                        "camera": [ 835.89148, -94.770203, 40.035099 ],
                        "point": [ 659.96997, -90.027298, 6.7947001 ],
                        "time": [ 2 ],
                        "weather": [ 0 ]
                    })"_json);
                    g_Config.saveFile();
                }

                ImGui::EndChild();
            }
            break;
        }
        if (g_menuData.m_pSelected)
            ImGui::EndChild();
        render_warning();

        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    };
}
void Menu::remove() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

ImVec2 operator+(ImVec2& a, ImVec2& b) {
    return {a.x + b.x, a.y + b.y};
}
ImVec2 operator-(ImVec2& a, ImVec2& b) {
    return {a.x - b.x, a.y - b.y};
}
ImVec2 operator/(ImVec2& a, int b) {
    return {a.x / b, a.y /*/ b*/};
}

void Menu::render_doska() {
    const char* brend = "~~~ D0SKA P040TA ~~~";
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(brend).x / 2);
    ImGui::Text(brend);
    ImGui::Text(u8"Автор: Tim4ukys(vk.com/tim4ukys)\n"
                u8"Ранняя помощь(тестирование): Шамиль Макаров, Дмитрий Макаров\n"
                u8"Топ донатеров:\n"
                u8"1. Salik_Alvarez\n");
}

void Menu::render_warning() {
    if (g_menuData.m_iSelected == eTitles_News || g_menuData.m_iSelected == eTitles_INFO) return;

    const char msg[] = u8"Внимание: некоторые настройки вступят в силу только после перезагрузки игры";
    auto&& size = ImGui::CalcTextSize(msg);
    auto&& pos = (ImGui::GetWindowPos() + ImGui::GetWindowSize() / 2) - size / 2;

    static D3DXCOLOR s_color = 0xFF'FF'FF'FF;
    static auto      a = 0.05f;
    if (static snippets::Timer<50> s_tick;
        s_tick) {
        if (s_color.a >= 1.0f || s_color.a <= 0.0f) {
            a *= -1;
        }
        s_color.a += a;
        s_tick.reset();
    }
    
    ImGui::GetWindowDrawList()->AddText(pos, s_color, msg);
}

void Menu::show_cursor(bool show) {
    if (show) {
        patch::fill(0x541DF5, 5u, 0x90);
        patch::fill(0x53F417, 5u, 0x90);
        patch::setRaw(0x53F41F, "\x33\xC0\xf\x84");
        *(DWORD*)0xB73424 = 0;
        *(DWORD*)0xB73428 = 0;
        ((void (*)())0x541BD0)();
        ((void (*)())0x541DD0)();
        patch::writeMemory<std::uint8_t>(0x6194A0, 0xC3);

        *(DWORD*)(*g_sampBase.getAddr<DWORD*>(0x26E8F4) + 0x61) = 2;

        ShowCursor(TRUE);
        if (!GetCursor()) {
            SetCursor(LoadCursorA(0, (const char*)0x7F00));
        }
    } else {
        ShowCursor(FALSE);
        SetCursor(0);
        g_pSAMP->setCursorMode(CURSOR_NONE, TRUE);
    }
}

void Menu::set_style() {
    auto &io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    //io.Fonts->AddFontFromFileTTF(snippets::GetSystemFontPath("Segoe UI Semibold").c_str(), 18.f, 0, io.Fonts->GetGlyphRangesCyrillic());

    auto& colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
    colors[ImGuiCol_MenuBarBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_Border] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
    colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.24f};
    colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
    colors[ImGuiCol_TextDisabled] = ImVec4{0.5f, 0.5f, 0.5f, 1.0f};
    colors[ImGuiCol_Header] = ImVec4{0.13f, 0.13f, 0.17f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_HeaderActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_Button] = ImVec4{0.13f, 0.13f, 0.17f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_ButtonActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_CheckMark] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};
    colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.1f, 0.13f, 0.92f};
    colors[ImGuiCol_SliderGrab] = ImVec4{0.44f, 0.37f, 0.61f, 0.54f};
    colors[ImGuiCol_SliderGrabActive] = ImVec4{0.74f, 0.58f, 0.98f, 0.54f};
    colors[ImGuiCol_FrameBg] = ImVec4{0.13f, 0.13f, 0.17f, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_FrameBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_Tab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TabHovered] = ImVec4{0.24f, 0.24f, 0.32f, 1.0f};
    colors[ImGuiCol_TabActive] = ImVec4{0.2f, 0.22f, 0.27f, 1.0f};
    colors[ImGuiCol_TabUnfocused] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_ScrollbarBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
    colors[ImGuiCol_ScrollbarGrab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0.24f, 0.24f, 0.32f, 1.0f};
    colors[ImGuiCol_Separator] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};
    colors[ImGuiCol_SeparatorHovered] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};
    colors[ImGuiCol_SeparatorActive] = ImVec4{0.84f, 0.58f, 1.0f, 1.0f};
    colors[ImGuiCol_ResizeGrip] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
    colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.74f, 0.58f, 0.98f, 0.29f};
    colors[ImGuiCol_ResizeGripActive] = ImVec4{0.84f, 0.58f, 1.0f, 0.29f};

    auto& style = ImGui::GetStyle();
    style.TabRounding = 4;
    style.ScrollbarRounding = 9;
    style.WindowRounding = 7;
    style.GrabRounding = 3;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ChildRounding = 4;
    style.WindowTitleAlign = {0.5f, 0.5f};
    style.WindowMenuButtonPosition = ImGuiDir_None;
}