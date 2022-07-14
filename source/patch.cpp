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
#include "patch.h"

// ---------------------------

patch::callHook::callHook(uintptr_t HookAddress)
    : m_nHookAddress(HookAddress),
      m_nOldCallFunc(NULL) {
}

patch::callHook::~callHook() {
    uninstallHook();
}

BOOL patch::callHook::set(uintptr_t HookAddress, void* DetourFunction, uintptr_t* oldCallFnc) {
    if (oldCallFnc)
        *oldCallFnc = *reinterpret_cast<uintptr_t*>(HookAddress + 1) + HookAddress + 5;

    if (DWORD oldProt; VirtualProtect(reinterpret_cast<void*>(HookAddress + 1), sizeof(uintptr_t), PAGE_READWRITE, &oldProt)) {
        *reinterpret_cast<uintptr_t*>(HookAddress + 1) = reinterpret_cast<uintptr_t>(DetourFunction) - HookAddress - (sizeof(uintptr_t) + 1U);
        VirtualProtect(reinterpret_cast<void*>(HookAddress + 1), sizeof(uintptr_t), oldProt, &oldProt);
        return TRUE;
    }
    return FALSE;
}

BOOL patch::callHook::installHook(void* DetourFunction, bool oldDelete) {
    if (oldDelete && m_nOldCallFunc)
        uninstallHook();

    return set(m_nHookAddress, DetourFunction, &m_nOldCallFunc);
}

BOOL patch::callHook::uninstallHook(uintptr_t oldAddrFunc) {
    auto rr = set(m_nHookAddress, reinterpret_cast<PVOID>(!oldAddrFunc ? m_nOldCallFunc : oldAddrFunc));
    m_nOldCallFunc = NULL;
    return rr;
}

DWORD patch::callHook::getOriginal() const noexcept {
    return m_nOldCallFunc;
}

// ---------------------------

extern "C" {

static void GetModuleInfo(const char* szModule, MODULEINFO* moduleInfo) {
    ZeroMemory(moduleInfo, sizeof(MODULEINFO));
    HMODULE hModule = GetModuleHandleA(szModule);
    if (hModule) {
        GetModuleInformation(GetCurrentProcess(), hModule, moduleInfo, sizeof(MODULEINFO));
    }
}

uint32_t patch__FindPattern(_In_ const char* szModule, _In_ const char* szPattern, _In_ const char* mask,
                            _In_opt_ uint32_t startSearchAddr) {
    // Get all module related information
    MODULEINFO mInfo;
    GetModuleInfo(szModule, &mInfo);

    // Assign our base and module size
    // Having the values right is ESSENTIAL, this makes sure
    // that we don't scan unwanted memory and leading our game to crash
    const uint32_t base = startSearchAddr == NULL ? (uint32_t)mInfo.lpBaseOfDll : startSearchAddr;
    const uint32_t size = mInfo.SizeOfImage;

    // Get length for our mask, this will allow us to loop through our array
    const DWORD patternLength = (DWORD)strlen(mask);

    for (uintptr_t i = 0; i < size - (uintptr_t)patternLength; i++) {
        char found = '\x1';
        DWORD j;
        for (j = 0; j < patternLength; j++) {
            // if we have a ? in our mask then we have true by default,
            // or if the bytes match then we keep searching until finding it or not
            found &= mask[j] == '?' || szPattern[j] == *(char*)(base + i + j);
        }

        // found = true, our entire pattern was found
        // return the memory addy so we can write to it
        if (found) {
            return base + i;
        }
    }

    return NULL;
}

BOOL patch__setRawThroughJump(_In_ uint32_t address, _In_ const char* raw, _In_ size_t rawSize,
                              _In_opt_ size_t saveByte, _In_opt_ BOOL isSave) {
    DWORD oldProtect;

    size_t size = saveByte < 5u ? 5u : saveByte;

    if (VirtualProtect((PVOID)address, size, PAGE_READWRITE, &oldProtect)) {
        // Копируем память, чтобы потом вставить её в конец
        uint8_t* aSaveByte = NULL;
        if (isSave) {
            aSaveByte = (uint8_t*)malloc(size);
            memcpy(aSaveByte, (PVOID)address, size);
        }

        // NOP'им память
        FillMemory((PVOID)address, size, 0x90);

        // Создаём островок, где будем вызывать detour
        uint8_t* memIsland = (uint8_t*)malloc(5u + rawSize + (isSave ? size : 0U));

        // Делаем прыжок на островок
        *(uint8_t*)address = 0xE9; /* jump */
        *(uint32_t*)(address + 1) = (uint32_t)memIsland - address - 5u;

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
        *(uint32_t*)((uint32_t)memIsland + 1) = address + 5u - (uint32_t)memIsland - 5u;

        VirtualProtect((PVOID)address, size, oldProtect, NULL);
        return TRUE;
    }

    return FALSE;
}

BOOL patch__setJump(_In_ uint32_t address, _In_ uint32_t detour,
                    _In_opt_ size_t saveByte, _In_opt_ BOOL isSave) {
    DWORD oldProtect;

    const size_t size = saveByte < 5u ? 5u : saveByte;

    if (VirtualProtect((PVOID)address, size, PAGE_READWRITE, &oldProtect)) {
        // Копируем память, чтобы потом вставить её в конец
        uint8_t* aSaveByte = NULL;
        if (isSave) {
            aSaveByte = (uint8_t*)malloc(size);
            memcpy(aSaveByte, (PVOID)address, size);
        }

        // NOP'им память
        FillMemory((PVOID)address, size, 0x90);

        // Создаём островок, где будем вызывать detour
        uint8_t* memIsland = (uint8_t*)malloc(5u * 2 + (isSave ? size : 0U));

        // Делаем прыжок на островок
        *(uint8_t*)address = 0xE9; /* jump */
        *(uint32_t*)(address + 1) = (uint32_t)memIsland - address - 5u;

        // Вызываем detour
        *memIsland = 0xE8; /* call */
        *(uint32_t*)(memIsland + 1) = detour - (uint32_t)memIsland - 5u;
        memIsland += 5u;

        // Вставляем сохранёные байты
        if (isSave) {
            memcpy(memIsland, aSaveByte, size);
            memIsland += size;

            free(aSaveByte);
        }

        // Делаем прыжок назад
        *memIsland = 0xE9; /* jump */
        *(uint32_t*)((uint32_t)memIsland + 1) = address + 5u - uint32_t(memIsland) - 5u;

        VirtualProtect((PVOID)address, size, oldProtect, NULL);
        return TRUE;
    }

    return FALSE;
}

BOOL patch__setPushOffset(_In_ uint32_t address, _In_ uint32_t offsetAddress) {
    DWORD oldProtect;
    if (VirtualProtect((PVOID)address, 5u, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        uint8_t* pAddr = (uint8_t*)address;

        *pAddr = 0x68;
        memcpy(++pAddr, &offsetAddress, 4u);

        VirtualProtect((PVOID)address, 5u, oldProtect, NULL);
        return TRUE;
    }
    return FALSE;
}

BOOL patch__getHEX(_In_ uint32_t address, _Out_ char* outBuff, _In_ size_t size) {
    DWORD oldProtect;
    if (VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        size_t i;
        for (i = 0; i < size; ++i) {
            sprintf(outBuff, "%02X", *(uint8_t*)(address++));
            outBuff += 2;
        }
        VirtualProtect((PVOID)address, size, oldProtect, NULL);
        return TRUE;
    }
    return FALSE;
}

BOOL patch__setRaw(_In_ uint32_t address, _In_ const char* raw, _In_ size_t size) {
    DWORD oldProtect;
    if (VirtualProtect((PVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy((PVOID)address, raw, size);
        VirtualProtect((LPVOID)address, size, oldProtect, NULL);
        return TRUE;
    }
    return FALSE;
}

BOOL patch__fill(_In_ uint32_t address, _In_ size_t size, _In_ uint32_t value) {
    DWORD oldProtect;
    if (VirtualProtect((PVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        FillMemory((PVOID)address, size, value);
        VirtualProtect((PVOID)address, size, oldProtect, NULL);
        return TRUE;
    }
    return FALSE;
}

}