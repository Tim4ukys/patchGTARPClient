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

    /**
    * @brief Путь до шрифта
    * @param faceName Название шрифта
    * @return Полный путь до шрифта
    */
    std::string GetSystemFontPath(const std::string& faceName);

    class WinProcHeader {
    public:

        static void Init();
        static std::shared_ptr<PLH::x86Detour> regWinProc(WNDPROC pNewHeader, WNDPROC* pOldHeader);

    private:
        static WNDPROC s_pOrig;
        static LRESULT __stdcall WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };

    std::array<int, 3> versionParse(const std::string& vers);
};