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
#ifndef _PATCH_HPP_
#define _PATCH_HPP_

/*
 * @auther Tim4ukys
 * @date 01.01.2023
 * @copyright GNU GPLv3
 * @warning Работает только с x86 приложениями
 *
 * Модуль для патчинга памяти
 */

#include <cstdint>
#include <string>
#include <cstring>
#include <Windows.h>

namespace patch {

    template<typename T>
    void writeMemory(std::uintptr_t addr, T value) {
        DWORD oldProt;
        VirtualProtect((void*)addr, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProt);
        *reinterpret_cast<T*>(addr) = value;
        VirtualProtect((void*)addr, sizeof(T), oldProt, NULL);
    }

    class callHook {
        std::uintptr_t m_oldFnc{}, m_hookAddr{}, m_detFnc{};

    public:
        callHook() = default;
        callHook(std::uintptr_t hkAddr)
            : m_hookAddr(hkAddr){};
        ~callHook() {
            remove();
        }

        inline void setDet(std::uintptr_t det) noexcept { m_detFnc = det; }
        inline void setHookAddr(std::uintptr_t hkAddr) noexcept { m_hookAddr = hkAddr; }

        inline void install() {
            if (!m_detFnc || !m_hookAddr) return;

            if (m_oldFnc) remove();

            DWORD oldProt;
            VirtualProtect((void*)m_hookAddr, 5u, PAGE_EXECUTE_READWRITE, &oldProt);
            reinterpret_cast<std::uint8_t*>(m_hookAddr)[0] = '\xE8'; // call ...
            m_oldFnc = *reinterpret_cast<std::uint32_t*>(m_hookAddr + 1);
            *reinterpret_cast<std::uint32_t*>(m_hookAddr + 1) = m_detFnc;
            VirtualProtect((void*)m_hookAddr, 5u, oldProt, NULL);
        }
        inline void remove() {
            if (!m_oldFnc) return;
            writeMemory<std::uint32_t>(m_hookAddr + 1, m_oldFnc);
            m_oldFnc = 0;
        }

        template<typename fnc, typename... Args>
        inline auto call(Args&&... args) const noexcept {
            return ((fnc*)m_oldFnc)(args...);
        }
    };

    inline void getHEX(std::uintptr_t addr, std::size_t count, std::string& textOut) {
        DWORD oldProt;
        VirtualProtect((void*)addr, count, PAGE_EXECUTE_READWRITE, &oldProt);
        textOut.resize(count*2);
        for (std::size_t i{}; i < count; ++i)
            sprintf(textOut.data() + i*2, "%02X", *(std::uint8_t*)(addr + i));
        VirtualProtect((void*)addr, count, oldProt, NULL);
    }

    inline std::string getHEX(std::uintptr_t addr, std::size_t count) {
        std::string text;
        getHEX(addr, count, text);
        return text;
    }

    inline void setRaw(std::uintptr_t addr, const char* raw_str, std::size_t len) {
        DWORD oldProt;
        VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &oldProt);
        memcpy((void*)addr, raw_str, len);
        VirtualProtect((void*)addr, len, oldProt, NULL);
    }

    template<std::size_t N>
    inline void setRaw(std::uintptr_t addr, const char (&raw_str)[N]) {
        DWORD oldProt;
        VirtualProtect((void*)addr, N - 1, PAGE_EXECUTE_READWRITE, &oldProt);
        memcpy((void*)addr, (void*)raw_str, N - 1);
        VirtualProtect((void*)addr, N - 1, oldProt, NULL);
    }

    inline void setBytes(std::uintptr_t addr, const std::vector<std::uint8_t>& bytes) {
        const auto len = bytes.size();
        DWORD      oldProt;
        VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &oldProt);
        memcpy((void*)addr, bytes.data(), len);
        VirtualProtect((void*)addr, len, oldProt, NULL);
    }

    inline void setBytes(std::uintptr_t addr, const std::uint8_t* bytes, std::size_t len_bytes) {
        DWORD oldProt;
        VirtualProtect((void*)addr, len_bytes, PAGE_EXECUTE_READWRITE, &oldProt);
        memcpy((void*)addr, bytes, len_bytes);
        VirtualProtect((void*)addr, len_bytes, oldProt, NULL);
    }

    template<std::size_t N>
    void setBytes(std::uintptr_t addr, const std::uint8_t(&bytes)[N]) {
        DWORD oldProt;
        VirtualProtect((void*)addr, N, PAGE_EXECUTE_READWRITE, &oldProt);
        memcpy((void*)addr, (void*)bytes, N);
        VirtualProtect((void*)addr, N, oldProt, NULL);
    }

    inline void getBytes(std::uintptr_t addr, std::vector<std::uint8_t>& outBytes, std::size_t count) {
        DWORD oldProt;
        VirtualProtect((void*)addr, count, PAGE_EXECUTE_READWRITE, &oldProt);
        outBytes.resize(count);
        memcpy(outBytes.data(), (void*)addr, count);
        VirtualProtect((void*)addr, count, oldProt, NULL);
    }

    template<std::size_t N>
    void getBytes(std::uintptr_t addr, std::uint8_t (&outBytes)[N]) {
        DWORD oldProt;
        VirtualProtect((void*)addr, N, PAGE_EXECUTE_READWRITE, &oldProt);
        memcpy((void*)outBytes, (void*)addr, N);
        VirtualProtect((void*)addr, N, oldProt, NULL);
    }

    enum class TYPE_JMP { sml, lrg };
    template<TYPE_JMP typeJMP>
    void setJump(std::uintptr_t addr, std::uintptr_t count) {
        if constexpr (typeJMP == TYPE_JMP::sml) {
            setBytes(addr, {0xEB, static_cast<std::uint8_t>(count - 2U)});
        } else if constexpr (typeJMP == TYPE_JMP::lrg) {
            std::uint8_t raw[5]{0xE9};
            *(std::uintptr_t*)((std::uint8_t*)raw + 1) = count - 5U;
            setBytes(addr, raw);
        }
    }

    BOOL setRawThroughJump(std::uintptr_t address, const char* raw,
                           std::size_t rawSize, std::size_t saveByte,
                           BOOL isSave);

    BOOL setJumpThroughJump(std::uintptr_t address, std::uintptr_t detour,
                            std::size_t saveByte, BOOL isSave);

    inline void fill(std::uintptr_t addr, std::size_t sz, unsigned int value) {
        DWORD oldProt;
        VirtualProtect((void*)addr, sz, PAGE_EXECUTE_READWRITE, &oldProt);
        FillMemory((void*)addr, sz, value);
        VirtualProtect((void*)addr, sz, oldProt, NULL);
    }

} // namespace patch

#endif