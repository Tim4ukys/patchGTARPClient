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
    /**
    * Use: static_assert(all_same<Args...>::value, "Params must have same types");
    */
    template<typename... args>
    struct all_same : public std::false_type {};

    template<typename T>
    struct all_same<T> : public std::true_type {};

    template<typename T, typename... args>
    struct all_same<T, T, args...> : public all_same<T, args...> {};

    inline int randomInteger(int min, int max) {
        static std::random_device          rd;
        static std::default_random_engine  e1(rd());
        std::uniform_int_distribution<int> uniform_dist(min, max);
        return uniform_dist(e1);
    }

    template<UINT64 toWait>
    class Timer {
        UINT64 nextTick{};

    public:
        Timer() {
            reset();
        }

        inline bool isEnd() const {
            return GetTickCount64() >= nextTick;
        }
        operator bool() const {
            return isEnd();
        }

        inline void reset() {
            nextTick = GetTickCount64() + toWait;
        }
    };

    class DynamicLibrary {
        std::uintptr_t m_baseAddr;
        std::mutex     m_lock;
        std::string    m_moduleName;

    public:
        DynamicLibrary() = delete;

        inline void updateBase(std::uintptr_t addr) { m_baseAddr = addr; }
        inline void updateBase() { m_baseAddr = (std::uintptr_t)GetModuleHandleA(m_moduleName.c_str()); }

        explicit DynamicLibrary(const std::string& libName)
            : m_moduleName(libName) {
            updateBase();
        }

        template<typename T>
        T getAddr() const noexcept {
            return T(m_baseAddr);
        }
        template<typename T>
        T getAddr(std::uintptr_t offset) const noexcept {
            return reinterpret_cast<T>(m_baseAddr + offset);
        }
        template<typename T, typename T2>
        T getAddr(T2 offset) const noexcept {
            return T(m_baseAddr + std::uintptr_t(offset));
        }

        template<typename fnc>
        auto getFnc(const char* procName) noexcept {
            std::lock_guard<std::mutex> lock{m_lock};
            return reinterpret_cast<fnc*>(GetProcAddress((HMODULE)m_baseAddr, procName));
        }

        template<typename fnc, typename... Args> 
        auto callFnc(const char* procName, Args&&... args) noexcept {
            return getFnc<fnc>(procName)(args...);
        }

        inline PIMAGE_NT_HEADERS getNTHeader() const noexcept {
            return getAddr<PIMAGE_NT_HEADERS>(getAddr<PIMAGE_DOS_HEADER>()->e_lfanew);
        }
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