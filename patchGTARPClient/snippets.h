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
    template<typename tKey, typename tVar>
    class FastMap {
        std::vector<tKey> keys;
        std::vector<tVar> vars;

        std::size_t binSearch(const tKey& k) {
            size_t low{}, high{keys.size() - 1};
            while (low <= high) {
                size_t mid = (low + high) / 2;
                auto&  guess = keys[mid];
                if (guess > k) {
                    high = mid - 1;
                } else if (guess < k) {
                    low = mid + 1;
                } else {
                    return mid;
                }
            }
            return -1;
        }

        void qsort(std::size_t low, std::size_t high) {
            std::size_t i = low;
            std::size_t j = high;
            auto pivot = keys[(i + j) / 2];

            while (i <= j) {
                while (keys[i] < pivot)
                    i++;
                while (keys[j] > pivot)
                    j--;
                if (i <= j) {
                    std::swap(keys[i], keys[j]);
                    std::swap(vars[i], vars[j]);
                    i++;
                    j--;
                }
            }
            if (j > low)
                qsort(low, j);
            if (i < high)
                qsort(i, high);
        }

    public:
        FastMap() = default;
        ~FastMap() = default;

        template<std::size_t arrLen>
        explicit FastMap(const std::pair<tKey, tVar> (&arr)[arrLen]) {
            keys.resize(arrLen);
            vars.resize(arrLen);
            for (size_t i{}; i < arrLen; ++i) {
                keys[i] = std::move(arr[i].first);
                vars[i] = std::move(arr[i].second);
            }
            qsort(0, arrLen - 1);
        }

        inline std::vector<tVar>& getVars() noexcept {
            return vars;
        }

        tVar& operator[](const tKey& k) {
            #ifndef _NDEBUG
            const auto pos = binSearch(k);
            if (pos == -1) {
                throw std::exception("Ты ебала??? Этого нет в списке, *facepalm moment*");
            }
            return vars[pos];
            #else
            return vars[binSearch(k)];
            #endif
        }

    };

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