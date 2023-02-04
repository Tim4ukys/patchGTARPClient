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
#pragma once
class Log {
public:
    Log(std::string_view fileName);
    ~Log();
    
    void Write(const char* fmt, ...);
    Log& operator<<(const char* r);

private:
    std::mutex  m_lock;
    std::string m_fileName;

    void(*m_fncSFLog)(PVOID, const char*, ...) = nullptr;
};
