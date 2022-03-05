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
    : m_nHookAddress(HookAddress), m_nOldCallFunc(NULL) {}

patch::callHook::~callHook() {
    uninstallHook();
}

BOOL patch::callHook::set(uintptr_t HookAddress, void* DetourFunction, uintptr_t *oldCallFnc) {
    if (oldCallFnc)
        *oldCallFnc = *reinterpret_cast<uintptr_t*>(HookAddress + 1) + HookAddress + 5;

    if (DWORD oldProt; VirtualProtect(reinterpret_cast<void*>(HookAddress + 1), sizeof(uintptr_t), PAGE_READWRITE, &oldProt)) 
    {
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

inline MODULEINFO GetModuleInfo(char* szModule) {
    MODULEINFO modinfo{};
    HMODULE    hModule = GetModuleHandleA(szModule);
    if (!hModule)
        return modinfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
    return modinfo;
}

uintptr_t patch::FindPattern(char* module, char* pattern, char* mask, uintptr_t startSearchAddr) {
    //Get all module related information
    const MODULEINFO mInfo = GetModuleInfo(module);

    //Assign our base and module size
    //Having the values right is ESSENTIAL, this makes sure
    //that we don't scan unwanted memory and leading our game to crash
    const uintptr_t base = startSearchAddr == NULL ? uintptr_t(mInfo.lpBaseOfDll) : startSearchAddr;
    const uintptr_t size = mInfo.SizeOfImage;

    //Get length for our mask, this will allow us to loop through our array
    const DWORD patternLength = (DWORD)strlen(mask);

    for (uintptr_t i = 0; i < size - (uintptr_t)patternLength; i++) {
        bool found = true;
        for (DWORD j = 0; j < patternLength; j++) {
            //if we have a ? in our mask then we have true by default,
            //or if the bytes match then we keep searching until finding it or not
            found &= mask[j] == '?' || pattern[j] == *(char*)(base + i + j);
        }

        //found = true, our entire pattern was found
        //return the memory addy so we can write to it
        if (found) {
            return base + i;
        }
    }

    return NULL;
}

void patch::HEXtoRaw(std::string& hex, std::vector<uint8_t>* raw) {
    raw->resize(hex.size() / 2);

    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        char buff[3];
        memcpy(buff, &hex[i], 2);
        buff[2] = '\0';

        (*raw)[i / 2] = static_cast<uint8_t>(strtol(buff, nullptr, 16));
    }
}

BOOL patch::setRawThroughJump(uintptr_t address, const char* raw, size_t rawSize, size_t saveByte, bool isSave) {
    DWORD oldProtect;

    size_t size = saveByte < (sizeof(uintptr_t) + 1) ? (sizeof(uintptr_t) + 1) : saveByte;

    if (VirtualProtect(PVOID(address), size, PAGE_READWRITE, &oldProtect)) {
        // Копируем память, чтобы потом вставить её в конец
        uint8_t* aSaveByte = nullptr;
        if (isSave) {
            aSaveByte = new uint8_t[size];
            memcpy(aSaveByte, PVOID(address), size);
        }

        // NOP'им память
        FillMemory(PVOID(address), size, 0x90);

        // Создаём островок, где будем вызывать detour
        uint8_t* memIsland = reinterpret_cast<uint8_t*>(malloc((sizeof(uintptr_t) + 1) * 2 + (isSave ? size : 0U)));

        // Делаем прыжок на островок
        *(uint8_t*)address = 0xE9; /* jump */
        *reinterpret_cast<DWORD*>(address + 1) = DWORD(memIsland) - address - (sizeof(uintptr_t) + 1);

        // Ставим raw
        memcpy(memIsland, raw, rawSize);
        memIsland += rawSize;

        // Вставляем сохранёные байты
        if (isSave) {
            memcpy(memIsland, aSaveByte, size);
            memIsland += size;

            delete[] aSaveByte;
        }

        // Делаем прыжок назад
        *memIsland = 0xE9; /* jump */
        *(DWORD*)(DWORD(memIsland) + 1) = (address + (sizeof(uintptr_t) + 1)) - DWORD(memIsland) - (sizeof(uintptr_t) + 1);

        VirtualProtect(PVOID(address), size, oldProtect, NULL);
        return TRUE;
    }

    return FALSE;
}

BOOL patch::setPushOffset(uintptr_t address, uintptr_t offsetAddress) {
    DWORD oldProtect;
    if (VirtualProtect(reinterpret_cast<LPVOID>(address), (sizeof(uintptr_t) + 1), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        uint8_t* pAddr = reinterpret_cast<uint8_t*>(address);
        *pAddr = 0x68;

        memcpy(++pAddr, &offsetAddress, sizeof(uintptr_t));

        VirtualProtect(reinterpret_cast<LPVOID>(address), (sizeof(uintptr_t) + 1), oldProtect, NULL);
        return TRUE;
    }

    return FALSE;
}

std::string patch::getHEX(uintptr_t address, size_t size) {
    std::stringstream sstream{};

    DWORD oldProtect;
    if (VirtualProtect(reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        for (size_t i = 0; i < size; i++) {
            sstream << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << static_cast<int>(reinterpret_cast<uint8_t*>(address)[i]);
        }

        VirtualProtect(reinterpret_cast<LPVOID>(address), size, oldProtect, NULL);
    }

    return sstream.str();
}

BOOL patch::setRaw(uintptr_t address, std::vector<uint8_t>& raw) {
    DWORD oldProtect;

    if (VirtualProtect(reinterpret_cast<LPVOID>(address), raw.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(PVOID(address), &raw[0], raw.size());

        VirtualProtect(reinterpret_cast<LPVOID>(address), raw.size(), oldProtect, NULL);
        return TRUE;
    }
    return FALSE;
}

BOOL patch::setRaw(uintptr_t address, const char* raw, size_t size) {
    DWORD oldProtect;

    if (VirtualProtect(reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(PVOID(address), raw, size);

        VirtualProtect(reinterpret_cast<LPVOID>(address), size, oldProtect, NULL);
        return TRUE;
    }
    return FALSE;
}

BOOL patch::setRaw(uintptr_t address, std::string& raw) {
    DWORD oldProtect;
    if (VirtualProtect(reinterpret_cast<LPVOID>(address), raw.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(PVOID(address), &raw[0], raw.size());

        VirtualProtect(reinterpret_cast<LPVOID>(address), raw.size(), oldProtect, NULL);
        return TRUE;
    }
    return FALSE;
}

template<typename T>
BOOL patch::set(uintptr_t address, T value) {
    DWORD oldProtect;
    if (VirtualProtect(reinterpret_cast<PVOID>(address), sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *reinterpret_cast<T*>(address) = value;
        VirtualProtect(reinterpret_cast<PVOID>(address), sizeof(T), oldProtect, NULL);
        return TRUE;
    }
    return FALSE;
}

BOOL patch::fill(uintptr_t address, size_t size, uint32_t value) {
    DWORD oldProtect;
    if (VirtualProtect(reinterpret_cast<PVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        FillMemory(PVOID(address), size, value);
        VirtualProtect(reinterpret_cast<PVOID>(address), size, oldProtect, NULL);
        return TRUE;
    }
    return FALSE;
}
