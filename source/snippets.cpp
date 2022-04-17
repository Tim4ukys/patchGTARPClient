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
#include "snippets.h"

snippets::DynamicLibrary::DynamicLibrary(const char* libName) {
    m_unDllAddress.pHandl = nullptr;

    m_pModuleName = new char[strlen(libName) + 1];
    strcpy(m_pModuleName, libName);
}
snippets::DynamicLibrary::~DynamicLibrary() {
    delete[] m_pModuleName;
}

DWORD snippets::DynamicLibrary::getAddress() noexcept {
    if (!m_unDllAddress.pHandl) {
        std::lock_guard<std::mutex> lock(m_lock);
        m_unDllAddress.pHandl = GetModuleHandleA(m_pModuleName);
    }

    return m_unDllAddress.dwHandl;
}
DWORD snippets::DynamicLibrary::getAddress(DWORD offset) noexcept {
    if (!m_unDllAddress.pHandl) {
        std::lock_guard<std::mutex> lock(m_lock);
        m_unDllAddress.pHandl = GetModuleHandleA(m_pModuleName);
    }

    return m_unDllAddress.dwHandl + offset;
}

// -------------------------

std::string snippets::UTF8_to_CP1251(std::string const& utf8) {
    if (!utf8.empty()) {
        int wchlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), NULL, 0);
        if (wchlen > 0 && wchlen != 0xFFFD) {
            std::vector<wchar_t> wbuf(wchlen);
            MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), &wbuf[0], wchlen);
            std::vector<char> buf(wchlen);
            WideCharToMultiByte(1251, 0, &wbuf[0], wchlen, &buf[0], wchlen, 0, 0);

            return std::string(&buf[0], wchlen);
        }
    }
    return std::string();
}

std::string snippets::ConvertWideToANSI(const wchar_t* wstr)
{
    int         count = WideCharToMultiByte(CP_ACP, 0, wstr, wcslen(wstr), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, &str[0], count, NULL, NULL);
    return str;
}

// -------------------------

WNDPROC snippets::WinProcHeader::s_pOrig;
PLH::CapstoneDisassembler g_dis{PLH::Mode::x86};

void snippets::WinProcHeader::Init() {
    s_pOrig = (WNDPROC)SetWindowLongW(**(HWND**)0xC17054, GWL_WNDPROC, (LONG)WndProcHandler);
}

PLH::x86Detour* snippets::WinProcHeader::regWinProc(WNDPROC pNewHeader, WNDPROC* pOldHeader) {
    static std::mutex s_lock;
    std::lock_guard<std::mutex> lock(s_lock);

    uint64_t jumpToBack;
    auto pWinProc = new PLH::x86Detour((const PCHAR)WndProcHandler, (const PCHAR)pNewHeader, &jumpToBack, g_dis);
    pWinProc->hook();
    *pOldHeader = WNDPROC(jumpToBack);
    return pWinProc;
}

LRESULT __stdcall snippets::WinProcHeader::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return CallWindowProcA(s_pOrig, hWnd, msg, wParam, lParam);
}

// -------------------------

std::array<int, 3> snippets::versionParse(const std::string& vers) {
    const std::regex r{R"(([\w]+)\.([\w]+)\.([\w]+))"};
    std::smatch      m;

    std::array<int, 3> resolve;
    if (std::regex_search(vers, m, r)) {
        for (size_t i{0}; i < 3;) {
            resolve[i++] = std::atoi(m[i + 1].str().c_str());
        }
    }
    return resolve;
}