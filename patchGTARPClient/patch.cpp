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
#include "patch.hpp"

namespace patch {
    BOOL setJumpThroughJump(std::uintptr_t address, std::uintptr_t detour, std::size_t saveByte, BOOL isSave) {
        DWORD       oldProtect;
        std::size_t size = saveByte < 5u ? 5u : saveByte;

        if (VirtualProtect((PVOID)address, size, PAGE_READWRITE, &oldProtect)) {
            // Копируем память, чтобы потом вставить её в конец
            std::uint8_t* aSaveByte = NULL;
            if (isSave) {
                aSaveByte = (std::uint8_t*)malloc(saveByte);
                memcpy(aSaveByte, (PVOID)address, saveByte);
            }

            // NOP'им память
            FillMemory((PVOID)address, size, 0x90);

            // Создаём островок, где будем вызывать detour
            const auto    szMemIsland = 5u * 2 + (isSave ? saveByte : 0U);
            std::uint8_t* memIsland = (std::uint8_t*)malloc(szMemIsland);
            // VirtualProtect(memIsland, szMemIsland, , NULL);

            // Делаем прыжок на островок
            *(std::uint8_t*)address = 0xE9; /* jump */
            *(std::uint32_t*)(address + 1) = (std::uint32_t)memIsland - address - 5u;

            // Вызываем detour
            *memIsland = 0xE8; /* call */
            *(uint32_t*)(memIsland + 1) = detour - (uint32_t)memIsland - 5u;
            memIsland += 5u;

            // Вставляем сохранёные байты
            if (isSave) {
                memcpy(memIsland, aSaveByte, saveByte);
                memIsland += saveByte;

                free(aSaveByte);
            }

            // Делаем прыжок назад
            *memIsland = 0xE9; /* jump */
            *(std::uint32_t*)((std::uint32_t)memIsland + 1) = address + 5u - (std::uint32_t)memIsland - 5u;

            VirtualProtect((PVOID)address, size, oldProtect, &oldProtect);
            return TRUE;
        }
        return FALSE;
    }

    BOOL setRawThroughJump(std::uintptr_t address, const char* raw,
                           std::size_t rawSize, std::size_t saveByte, BOOL isSave) {
        DWORD       oldProtect;
        std::size_t size = saveByte < 5u ? 5u : saveByte;

        if (VirtualProtect((PVOID)address, size, PAGE_READWRITE, &oldProtect)) {
            // Копируем память, чтобы потом вставить её в конец
            std::uint8_t* aSaveByte = NULL;
            if (isSave) {
                aSaveByte = (std::uint8_t*)malloc(saveByte);
                memcpy(aSaveByte, (PVOID)address, saveByte);
            }

            // NOP'им память
            FillMemory((PVOID)address, size, 0x90);

            // Создаём островок, где будем вызывать detour
            const auto    szMemIsland = 5u + rawSize + (isSave ? saveByte : 0U);
            std::uint8_t* memIsland = (std::uint8_t*)malloc(szMemIsland);
            // VirtualProtect(memIsland, szMemIsland, , NULL);

            // Делаем прыжок на островок
            *(std::uint8_t*)address = 0xE9; /* jump */
            *(std::uint32_t*)(address + 1) = (std::uint32_t)memIsland - address - 5u;

            // Переносим RAW на островок
            memcpy(memIsland, raw, rawSize);
            memIsland += rawSize;

            // Вставляем сохранёные байты
            if (isSave) {
                memcpy(memIsland, aSaveByte, saveByte);
                memIsland += saveByte;

                free(aSaveByte);
            }

            // Делаем прыжок назад
            *memIsland = 0xE9; /* jump */
            *(std::uint32_t*)((std::uint32_t)memIsland + 1) = address + 5u - (std::uint32_t)memIsland - 5u;

            VirtualProtect((PVOID)address, size, oldProtect, &oldProtect);
            return TRUE;
        }
        return FALSE;
    }

    BOOL setShellCodeThroughJump(std::uintptr_t address, Xbyak::CodeGenerator& code,
                                 std::size_t saveByte, BOOL isSave) {
        DWORD       oldProtect;
        std::size_t size = saveByte < 5u ? 5u : saveByte;

        if (VirtualProtect((PVOID)address, size, PAGE_READWRITE, &oldProtect)) {
            // Копируем код
            bool isHaveOldShell{};
            if (isSave) {
                // Проверяем, стоял ли этот патч тут раньше
                if (auto addr = reinterpret_cast<uint8_t*>(address);
                    isHaveOldShell = (*addr == 0xE9)) {
                    DWORD targ = *reinterpret_cast<PDWORD>(address + 1) + address + 5;
                    if (size - 5u > 0) {
                        code.db(addr + 5u, saveByte - 5u);
                    }
                    code.jmp(PVOID(targ));
                } else {
                    code.db(addr, saveByte);
                }
            }

            // NOP'им память
            FillMemory((PVOID)address, size, 0x90);

            // Делаем прыжок с островка назад
            if (!isHaveOldShell)
                code.jmp(PVOID(address + 5u));

            // Делаем прыжок на островок
            *(std::uint8_t*)address = 0xE9; /* jump */
            *(std::uint32_t*)(address + 1) = code.getCode<std::uint32_t>() - address - 5u;

            VirtualProtect((PVOID)address, size, oldProtect, &oldProtect);
            return TRUE;
        }
        return FALSE;
    }

    inline MODULEINFO GetModuleInfo(const char* szModule) {
        MODULEINFO modinfo{};
        HMODULE    hModule = GetModuleHandleA(szModule);
        if (!hModule)
            return modinfo;
        GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
        return modinfo;
    }

    DWORD FindPattern(const char* module, const char* pattern, const char* mask, DWORD startSearchAddr) {
        // Get all module related information
        MODULEINFO mInfo = GetModuleInfo(module);

        // Assign our base and module size
        // Having the values right is ESSENTIAL, this makes sure
        // that we don't scan unwanted memory and leading our game to crash
        DWORD base = startSearchAddr == NULL ? DWORD(mInfo.lpBaseOfDll) : startSearchAddr;
        DWORD size = mInfo.SizeOfImage;

        // Get length for our mask, this will allow us to loop through our array
        DWORD patternLength = (DWORD)strlen(mask);

        for (DWORD i = 0; i < size - patternLength; i++) {
            bool found = true;
            for (DWORD j = 0; j < patternLength; j++) {
                // if we have a ? in our mask then we have true by default,
                // or if the bytes match then we keep searching until finding it or not
                found &= mask[j] == '?' || pattern[j] == *(char*)(base + i + j);
            }

            // found = true, our entire pattern was found
            // return the memory addy so we can write to it
            if (found) {
                return base + i;
            }
        }

        return NULL;
    }
} // namespace patch