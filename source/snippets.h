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

namespace snippets 
{
    class DynamicLibrary {
    public:

        DynamicLibrary(const char* libName);
        ~DynamicLibrary();

        DWORD getAddress() noexcept;
        DWORD getAddress(DWORD offset) noexcept;
    
    private:
        
        std::mutex m_lock;

        char* m_pModuleName;
        union {
            HANDLE pHandl;
            DWORD  dwHandl;
        } m_unDllAddress;
    };

    /*
    * @brief Переводит UTF8 в CP1251 кодировку
    * @param utf8 строка UTF8
    * @return строка в кодировке CP1251
    */
    std::string UTF8_to_CP1251(std::string const& utf8);

    /**
    * @brief Конвертирует wchar_t в string
    * @param wstr unicode строка
    * @return char строка
    */
    std::string ConvertWideToANSI(const wchar_t* wstr);
};