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

// Win-API: VirtualProtect, typedef BOOL
#include <Windows.h>
// C (Old-Library): strlen()
#include <cstring>
// C (Old-Library): uint8_t, uint16_t, uint32_t, uint64_t
#include <cstdint>

namespace patch 
{
	/*
	* @breaf Считывает участок памяти и переобразует его в HEX строку
	* @param address Адрес участка памяти с которого будет начинаться чтение
	* @param size Размер строки(массива байт)
	* @return HEX строка
	*/
	inline std::string getHEX(uint64_t address, size_t size)
	{
		std::stringstream sstream{};
		
		DWORD oldProtect;
		if (VirtualProtect(reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			for (size_t i = 0; i < size; i++)
			{
				sstream << std::uppercase << std::hex << static_cast<int>(*(reinterpret_cast<uint8_t*>(address) + i));
			}

			VirtualProtect(reinterpret_cast<LPVOID>(address), size, oldProtect, NULL);
		}

		return sstream.str();
	}

	/*
	* @breaf Меняет участок памяти согласно по массиву байтов в строке
	* @param address Адрес участка памяти с которого будет начинаться запись
	* @param raw Сам текст, из которового будет браться массив байт
	* @param size Размер строки(массива байт)
	* @return Состояние проделанной работы
	*/
	inline BOOL setRaw(uint64_t address, const char* raw, size_t size)
    {
        DWORD oldProtect;

        if (VirtualProtect(reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            for (size_t i = 0; i < size; i++) {
                *(reinterpret_cast<uint8_t*>(address) + i) = static_cast<uint8_t>(raw[i]);
            }

            VirtualProtect(reinterpret_cast<LPVOID>(address), size, oldProtect, NULL);
            return TRUE;
        }
        return FALSE;
    }
    /*
	* @breaf Меняет участок памяти согласно по массиву байтов в строке
	* @param address Адрес участка памяти с которого будет начинаться запись
	* @param raw Сам текст, из которового будет браться массив байт
	* @return Состояние проделанной работы
	*/
    inline BOOL setRaw(uint64_t address, std::string raw)
    {
        DWORD oldProtect;
        if (VirtualProtect(reinterpret_cast<LPVOID>(address), raw.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            for (size_t i = 0; i < raw.size(); i++) {
                *(reinterpret_cast<uint8_t*>(address) + i) = static_cast<uint8_t>(raw.at(i));
            }

            VirtualProtect(reinterpret_cast<LPVOID>(address), raw.size(), oldProtect, NULL);
            return TRUE;
        }
        return FALSE;
    }

    /*
	* @breaf Меняет определённый участок памяти по значению
	* @param address Адрес участка памяти в котором будет производиться перезапись
	* @param raw Сам текст, из которового будет браться массив байт
	* @return Состояние проделанной работы
	*/
	template<typename T>
	inline BOOL set(uint64_t address, T value)
	{
        DWORD oldProtect;
        if (VirtualProtect(reinterpret_cast<PVOID>(address), sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
			*reinterpret_cast<T*>(address) = value;
        	VirtualProtect(reinterpret_cast<PVOID>(address), sizeof(T), oldProtect, NULL);
            return TRUE;
        }
        return FALSE;
	}

	inline BOOL fill(uint64_t address, size_t size, uint32_t value)
	{
		DWORD oldProtect;
		if (VirtualProtect(reinterpret_cast<PVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			for (size_t i = 0; i < size; i++) {
				*(reinterpret_cast<uint8_t*>(address) + i) = value;
			}
			VirtualProtect(reinterpret_cast<PVOID>(address), size, oldProtect, NULL);
			return TRUE;
		}
		return FALSE;
	}
}

#endif