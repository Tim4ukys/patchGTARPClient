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

    BOOL setRawThroughJump(std::uintptr_t address, const char* raw,
                           std::size_t rawSize, std::size_t saveByte, BOOL isSave) {
        DWORD oldProtect;

        std::size_t size = saveByte < 5u ? 5u : saveByte;

        if (VirtualProtect((PVOID)address, size, PAGE_READWRITE, &oldProtect)) {
            // Копируем память, чтобы потом вставить её в конец
            std::uint8_t* aSaveByte = NULL;
            if (isSave) {
                aSaveByte = (std::uint8_t*)malloc(size);
                memcpy(aSaveByte, (PVOID)address, size);
            }

            // NOP'им память
            FillMemory((PVOID)address, size, 0x90);

            // Создаём островок, где будем вызывать detour
            const auto    szMemIsland = 5u + rawSize + (isSave ? size : 0U);
            std::uint8_t* memIsland = (std::uint8_t*)malloc(szMemIsland);
            //VirtualProtect(memIsland, szMemIsland, , NULL);

            // Делаем прыжок на островок
            *(std::uint8_t*)address = 0xE9; /* jump */
            *(std::uint32_t*)(address + 1) = (std::uint32_t)memIsland - address - 5u;

            // Переносим RAW на островок
            memcpy(memIsland, raw, rawSize);
            memIsland += rawSize;

            // Вставляем сохранёные байты
            if (isSave) {
                memcpy(memIsland, aSaveByte, size);
                memIsland += size;

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

}