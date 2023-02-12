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

UINT64 snippets::WinProcHeader::s_sampWndProcJump;
PLH::x86Detour snippets::WinProcHeader::s_sampWndProc{g_sampBase.getAddr<UINT64>(0x60EE0), UINT64(WndProcHandler), &s_sampWndProcJump};

void snippets::WinProcHeader::Init() {
    s_sampWndProc.hook();
}

//void snippets::WinProcHeader::regWinProc(WNDPROC newProc, WNDPROC& jumpDetour) {
//    jumpDetour = reinterpret_cast<WNDPROC>(s_sampWndProcJump);
//    s_sampWndProcJump = reinterpret_cast<UINT64>(newProc);
//}

LRESULT __stdcall snippets::WinProcHeader::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return WNDPROC(s_sampWndProcJump)(hWnd, msg, wParam, lParam);
}

std::shared_ptr<PLH::x86Detour> snippets::WinProcHeader::regWndProc(WNDPROC newProc, UINT64& jmp) {
    static std::mutex           s_lock;
    std::lock_guard<std::mutex> lock(s_lock);

    auto r = std::make_shared<PLH::x86Detour>(UINT64(WndProcHandler),
                                              UINT64(newProc),
                                              &jmp);
    r->hook();
    return r;
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

#pragma warning(push)
#pragma warning(disable : 4244)
std::string snippets::GetSystemFontPath(const std::string &faceName) {
    static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    HKEY                hKey;
    LONG                result;
    std::wstring        wsFaceName{faceName.begin(), faceName.end()};
    std::wstring        wsFontFile;

    // Open Windows font registry key
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fontRegistryPath, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        goto chk_local;
    }

    DWORD maxValueNameSize, maxValueDataSize;
    result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
    if (result != ERROR_SUCCESS) {
        goto chk_local;
    }

    DWORD        valueIndex = 0;
    LPWSTR       valueName = new WCHAR[maxValueNameSize];
    LPBYTE       valueData = new BYTE[maxValueDataSize];
    DWORD        valueNameSize, valueDataSize, valueType;

    // Look for a matching font name
    do {

        wsFontFile.clear();
        valueDataSize = maxValueDataSize;
        valueNameSize = maxValueNameSize;

        result = RegEnumValue(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);

        valueIndex++;

        if (result != ERROR_SUCCESS || valueType != REG_SZ) {
            continue;
        }

        std::wstring wsValueName(valueName, valueNameSize);

        // Found a match
        if (_wcsnicmp(wsFaceName.c_str(), wsValueName.c_str(), wsFaceName.length()) == 0 &&
            wsFaceName.length() + ARRAYSIZE(" (TrueType)") - 1 == wsValueName.length()) {

            wsFontFile.assign((LPWSTR)valueData, valueDataSize);
            break;
        }
    } while (result != ERROR_NO_MORE_ITEMS);

    delete[] valueName;
    delete[] valueData;

    RegCloseKey(hKey);

    if (wsFontFile.empty()) {
        goto chk_local;
    }

    // Build full font file path
    wchar_t winDir[MAX_PATH];
    GetWindowsDirectoryW(winDir, MAX_PATH);

    {
        std::wstringstream ss{};
        ss << winDir << "\\Fonts\\" << wsFontFile;
        wsFontFile = ss.str();
    }

    return std::string(wsFontFile.begin(), wsFontFile.end());

 chk_local:

    result = RegOpenKeyEx(HKEY_CURRENT_USER, fontRegistryPath, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        return "";
    }

    result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
    if (result != ERROR_SUCCESS) {
        return "";
    }

    valueIndex = 0;
    valueName = new WCHAR[maxValueNameSize];
    valueData = new BYTE[maxValueDataSize];

    // Look for a matching font name
    do {

        wsFontFile.clear();
        valueDataSize = maxValueDataSize;
        valueNameSize = maxValueNameSize;

        result = RegEnumValue(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);

        valueIndex++;

        if (result != ERROR_SUCCESS || valueType != REG_SZ) {
            continue;
        }

        std::wstring wsValueName(valueName, valueNameSize);

        // Found a match
        if (_wcsnicmp(wsFaceName.c_str(), wsValueName.c_str(), wsFaceName.length()) == 0 && 
            wsFaceName.length() + ARRAYSIZE(" (TrueType)") - 1 == wsValueName.length()) {

            wsFontFile.assign((LPWSTR)valueData, valueDataSize);
            break;
        }
    } while (result != ERROR_NO_MORE_ITEMS);

    delete[] valueName;
    delete[] valueData;

    RegCloseKey(hKey);

    if (wsFontFile.empty()) {
        return "";
    }

    return std::string(wsFontFile.begin(), wsFontFile.end());
}
#pragma warning(pop)