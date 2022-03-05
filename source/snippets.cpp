#include "pch.h"
#include "snippets.h"
#include "offsets.hpp"

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
        m_unDllAddress.pHandl = GetModuleHandleA(m_pModuleName);
    }

    return m_unDllAddress.dwHandl;
}
DWORD snippets::DynamicLibrary::getAddress(DWORD offset) noexcept {
    if (!m_unDllAddress.pHandl) {
        m_unDllAddress.pHandl = GetModuleHandleA(m_pModuleName);
    }

    return m_unDllAddress.dwHandl + offset;
}

template<typename T>
DWORD snippets::DynamicLibrary::getAddress(T offset) noexcept {
    if (!m_unDllAddress.pHandl) {
        m_unDllAddress.pHandl = GetModuleHandleA(m_pModuleName);
    }

    return m_unDllAddress.dwHandl + reinterpret_cast<DWORD>(offset);
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