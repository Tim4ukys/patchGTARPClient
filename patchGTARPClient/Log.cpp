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
#include "Log.h"

Log::Log(std::string_view fileName)
    : m_fileName(std::filesystem::current_path().string() + '\\' + fileName.data()) 
{
    std::filesystem::path file{m_fileName};
    if (std::filesystem::exists(file)) {
        std::filesystem::remove(file);
    }

    OSVERSIONINFOA vers;
    ZeroMemory(&vers, sizeof(OSVERSIONINFOA));
    vers.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

    if (GetVersionExA(&vers)) {
        Write("[PC INFO]: Windows version: %s %u.%u build: %u | PlatformID - %u", vers.szCSDVersion, vers.dwMajorVersion,
              vers.dwMinorVersion, vers.dwBuildNumber, vers.dwPlatformId);
    }
}
Log::~Log() {}

void Log::Write(const char* fmt, ...) {
    std::lock_guard<std::mutex> lock(m_lock);
    char buff[256];
    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(buff, fmt, arg);
    va_end(arg);

    char       timeBuff[15];
    SYSTEMTIME time;
    GetLocalTime(&time);
    sprintf(timeBuff, "[%02d:%02d:%02d.%03d]", 
        time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

    std::ofstream fstr(m_fileName, std::ios_base::app);
    fstr << timeBuff << ": " << buff << "\n";

    if (m_fncSFLog || g_SF.getAddr<std::uintptr_t>()) {
        if (!m_fncSFLog) {
            m_fncSFLog = g_SF.getFnc<void(PVOID, const char*, ...)>("?LogConsole@SAMPFUNCS@@QAAXPBDZZ");
        }
        m_fncSFLog(nullptr, "{FFAA00}[patchGTARPClient]: {FFFFFF}%s", buff);
    }
}

Log& Log::operator<<(const char* r) {
    Write(r);
    return *this;
}