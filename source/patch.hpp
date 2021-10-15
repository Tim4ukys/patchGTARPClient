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
	* @breaf Конвертирует HEX строку в массив с байтами
	* @param hex Строка в которой записаны числа в HEX формате
	* @return Контейнер vector с байтами внутри
	*/
	inline std::vector<uint8_t> HEXtoRaw(std::string hex)
	{
		std::vector<uint8_t> byte{};
		
		for (size_t i = 0; i + 1 < hex.size(); i += 2)
		{
			std::string h{}; h.push_back(hex[i]); h.push_back(hex[i + 1]);
			byte.push_back(std::stoi(h, NULL, 16));
		}
		return byte;
	}

	/*
	* @breaf Пихает offset переменной в стек
	* @param address Адрес участка памяти в котором будет это происходить
	* @param offsetAddress Адрес переменной
	* @return Состояние проделанной работы
	*/
	inline BOOL setPushOffset(uint32_t address, uint32_t offsetAddress)
	{
		std::stringstream sstream{};
		sstream << std::uppercase << std::hex << offsetAddress;
		auto raw = HEXtoRaw(sstream.str());
		if (!raw.size()) return FALSE;

		//g_pLog->Log("offsetAddress: 0x%X", offsetAddress);

		DWORD oldProtect;
		if (VirtualProtect(reinterpret_cast<LPVOID>(address), 5, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			uint8_t* pAddr = reinterpret_cast<uint8_t*>(address);
			*pAddr = 0x68;
			for	(size_t i = 1; i < 5; i++)
			{
				size_t vecPos = 4 - i;
				*(pAddr + i) = raw.size() > vecPos ? raw[vecPos] : 0;
			}
			VirtualProtect(reinterpret_cast<LPVOID>(address), 5, oldProtect, NULL);
			return TRUE;
		}

		return FALSE;
	}

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