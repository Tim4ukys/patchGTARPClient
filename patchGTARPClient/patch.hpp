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

    inline void getHEX(std::uintptr_t addr, std::size_t count, std::string& textOut) {
        DWORD oldProt;
        VirtualProtect((void*)addr, count, PAGE_EXECUTE_READWRITE, &oldProt);
        textOut.resize(count*2);
        for (std::size_t i{}; i < count; ++i)
            sprintf(textOut.data(), "%02X", *(std::uint8_t*)(addr++));
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
        memcpy((void*)addr, bytes, N);
        VirtualProtect((void*)addr, N, oldProt, NULL);
    }

    template<typename T>
    void writeMemory(std::uintptr_t addr, T value) {
        DWORD oldProt;
        VirtualProtect((void*)addr, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProt);
        *reinterpret_cast<T*>(addr) = value;
        VirtualProtect((void*)addr, sizeof(T), oldProt, NULL);
    }

    inline void setJump(std::uintptr_t addr, std::uintptr_t det) {
        auto jmpSz = (addr - det - 2U);
        if (jmpSz <= 0xFF) {
            setBytes(addr, {0xEB, static_cast<std::uint8_t>(jmpSz)});
        } else {
            jmpSz -= 3u;
            std::uint8_t raw[5]{0xE9};
            *(std::uintptr_t*)((std::uint8_t*)raw + 1) = addr;
            setBytes(addr, raw);
        }
    }

    BOOL setRawThroughJump(std::uintptr_t address, const char* raw,
                           std::size_t rawSize, std::size_t saveByte,
                           BOOL isSave);

    inline BOOL setJumpThrough(std::uintptr_t address, std::uintptr_t detour,
                        std::size_t saveByte, BOOL isSave) {
        std::uint8_t raw[5]{0xE9};
        *(std::uintptr_t*)((std::uint8_t*)raw + 1) = address;
        setBytes(address, raw);
        return setRawThroughJump(address, (char*)raw, 5, saveByte, isSave);
    }

    inline void fill(std::uintptr_t addr, unsigned int value, std::size_t sz) {
        DWORD oldProt;
        VirtualProtect((void*)addr, sz, PAGE_EXECUTE_READWRITE, &oldProt);
        FillMemory((void*)addr, sz, value);
        VirtualProtect((void*)addr, sz, oldProt, NULL);
    }

} // namespace patch

#endif