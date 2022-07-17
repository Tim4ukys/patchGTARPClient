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

// Win-API: VirtualProtect, typedef BOOL, typedef DWORD
#include <Windows.h>
// Win-API: struct MODULEINFO, GetModuleHandleA
#include <Psapi.h>
// STL: std::stringstream
#include <sstream>
// STL: std::setfill, std::setw
#include <iomanip>
// STL: std::reverse
#include <algorithm>
// C (Old-Library): strlen
#include <cstring>
// C (Old-Library): uint8_t, uint16_t, uint32_t, uint64_t
#include <cstdint>
// C (Old-Library): memcpy, ZeroMemory, FillMemory
#include <memory.h>

/*
* @authors Tim4ukys
* @date 05.11.2021
* @copyright GNU GPLv3
* @warning Работает только с x86 приложениями
* 
* Модуль для патчинга памяти
*/
namespace patch
{
	inline BOOL setRawThroughJump(uint32_t address, const char* raw, size_t rawSize, size_t saveByte = NULL, bool isSave = false)
	{
		DWORD oldProtect;

		size_t size = saveByte < 5U ? 5 : saveByte;

		if (VirtualProtect(PVOID(address), size, PAGE_READWRITE, &oldProtect))
		{
			// Копируем память, чтобы потом вставить её в конец
			uint8_t* aSaveByte = nullptr;
			if (isSave)
			{
				aSaveByte = new uint8_t[size];
				memcpy(aSaveByte, PVOID(address), size);
			}

			// NOP'им память
			FillMemory(PVOID(address), size, 0x90);

			// Создаём островок, где будем вызывать detour
			uint8_t* memIsland = reinterpret_cast<uint8_t*>(malloc(5U + (isSave ? size : 0U) + 5U));

			// Делаем прыжок на островок 
			*(uint8_t*)address = 0xE9; /* jump */
			*reinterpret_cast<DWORD*>(address + 1) = DWORD(memIsland) - address - 5;

			// Ставим raw
			memcpy(memIsland, raw, rawSize);
			memIsland += rawSize;

			// Вставляем сохранёные байты
			if (isSave)
			{
				memcpy(memIsland, aSaveByte, size);
				memIsland += size;

				delete[] aSaveByte;
			}

			// Делаем прыжок назад
			*memIsland = 0xE9; /* jump */
			*(DWORD*)(DWORD(memIsland) + 1) = (address + 5U) - DWORD(memIsland) - 5;

			VirtualProtect(PVOID(address), size, oldProtect, NULL);
			return TRUE;
		}

		return FALSE;
	}

	inline BOOL setJump(uint32_t address, uint32_t detour, size_t saveByte = NULL, bool isSave = false)
	{
		DWORD oldProtect;

		size_t size = saveByte < 5U ? 5 : saveByte;

		if (VirtualProtect(PVOID(address), size, PAGE_READWRITE, &oldProtect))
		{
			// Копируем память, чтобы потом вставить её в конец
			uint8_t* aSaveByte = nullptr;
			if (isSave)
			{
				aSaveByte = new uint8_t[size];
				memcpy(aSaveByte, PVOID(address), size);
			}

			// NOP'им память
			FillMemory(PVOID(address), size, 0x90);

			// Создаём островок, где будем вызывать detour
			uint8_t* memIsland = reinterpret_cast<uint8_t*>(malloc(5U + (isSave ? size : 0U) + 5U));

			// Делаем прыжок на островок 
			*(uint8_t*)address = 0xE9; /* jump */
			*reinterpret_cast<DWORD*>(address + 1) = DWORD(memIsland) - address - 5;

			// Вызываем detour
			*memIsland = 0xE8; /* call */
			*reinterpret_cast<DWORD*>(memIsland + 1) = detour - DWORD(memIsland) - 5;
			memIsland += 5;

			// Вставляем сохранёные байты
			if (isSave)
			{
				memcpy(memIsland, aSaveByte, size);
				memIsland += size;

				delete[] aSaveByte;
			}

			// Делаем прыжок назад
			*memIsland = 0xE9; /* jump */
			*(DWORD*)(DWORD(memIsland) + 1) = (address + 5U) - DWORD(memIsland) - 5;

			VirtualProtect(PVOID(address), size, oldProtect, NULL);
			return TRUE;
		}

		return FALSE;
	}

	/*
	* @breaf Возрващает информацию о модуле по его названию
	* @param szModule Название модуля
	* @return Информацию о модуле
	* @auther fredaikis unknowncheats
	*/
	inline MODULEINFO GetModuleInfo(char* szModule)
	{
		MODULEINFO modinfo{};
		HMODULE hModule = GetModuleHandleA(szModule);
		if (!hModule)
			return modinfo;
		GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
		return modinfo;
	}

	/*
	* @breaf Ищет патерн и возвращает его адрес
	* @param module Название модуля
	* @param pattern Патерн в виде RAW строки
	* @param mask Макска. X - будут читаться, а ? - игнорироваться
	* @param startSearchAddr Адресс с которого нужно начинать поиск.
	* @return Адрес патерна
	* @auther fredaikis unknowncheats
	*/
	inline DWORD FindPattern(char* module, char* pattern, char* mask, DWORD startSearchAddr = NULL)
	{
		//Get all module related information
		MODULEINFO mInfo = GetModuleInfo(module);


		//Assign our base and module size
		//Having the values right is ESSENTIAL, this makes sure
		//that we don't scan unwanted memory and leading our game to crash
		DWORD base = startSearchAddr == NULL ? DWORD(mInfo.lpBaseOfDll) : startSearchAddr;
		DWORD size = mInfo.SizeOfImage;

		//Get length for our mask, this will allow us to loop through our array
		DWORD patternLength = (DWORD)strlen(mask);

		for (DWORD i = 0; i < size - patternLength; i++)
		{
			bool found = true;
			for (DWORD j = 0; j < patternLength; j++)
			{
				//if we have a ? in our mask then we have true by default, 
				//or if the bytes match then we keep searching until finding it or not
				found &= mask[j] == '?' || pattern[j] == *(char*)(base + i + j);
			}

			//found = true, our entire pattern was found
			//return the memory addy so we can write to it
			if (found)
			{
				return base + i;
			}
		}

		return NULL;
	}

	/*
	* @breaf Ставит hook на вызов функции
	* @param HookAddress Адрес call функции
	* @param DetourFunction Обработчик функции
	* @return Адресс на оригинальну функцию
	* @auther kin4stat
	*/
	inline void* SetCallHook(uint32_t HookAddress, void* DetourFunction) {
		uint32_t OriginalFunction = *reinterpret_cast<uint32_t*>(HookAddress + 1) + HookAddress + 5;

		DWORD oldProt;
		VirtualProtect(reinterpret_cast<void*>(HookAddress + 1), sizeof(uint32_t), PAGE_READWRITE, &oldProt);

		*reinterpret_cast<uint32_t*>(HookAddress + 1) = reinterpret_cast<uint32_t>(DetourFunction) - HookAddress - 5;

		VirtualProtect(reinterpret_cast<void*>(HookAddress + 1), sizeof(uint32_t), oldProt, &oldProt);

		return reinterpret_cast<void*>(OriginalFunction);
	}

	/*
	* @breaf Конвертирует HEX строку в массив с байтами
	* @param hex Строка в которой записаны числа в HEX формате
	* @return Контейнер vector с байтами внутри
	*/
	inline void HEXtoRaw(std::string &hex, std::vector<uint8_t> *raw)
	{
		raw->resize(hex.size() / 2);

		for (size_t i = 0; i + 1 < hex.size(); i += 2)
		{
			char buff[3];
			memcpy(buff, &hex[i], 2);
			buff[2] = '\0';

			(*raw)[i / 2] = static_cast<uint8_t>(strtol(buff, nullptr, 16));
		}
	}

	/*
	* @breaf Пихает offset переменной в стек
	* @param address Адрес участка памяти в котором будет это происходить
	* @param offsetAddress Адрес переменной
	* @return Состояние проделанной работы
	*/
	inline BOOL setPushOffset(uint32_t address, uint32_t offsetAddress)
	{
		DWORD oldProtect;
		if (VirtualProtect(reinterpret_cast<LPVOID>(address), 5, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			uint8_t* pAddr = reinterpret_cast<uint8_t*>(address);
			*pAddr = 0x68;

			memcpy(++pAddr, &offsetAddress, 4U);

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
	inline std::string getHEX(uint32_t address, size_t size)
	{
		std::stringstream sstream{};

		DWORD oldProtect;
		if (VirtualProtect(reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			for (size_t i = 0; i < size; i++)
			{
				sstream << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << static_cast<int>(reinterpret_cast<uint8_t*>(address)[i]);
			}

			VirtualProtect(reinterpret_cast<LPVOID>(address), size, oldProtect, NULL);
		}

		return sstream.str();
	}

	/*
	* @breaf Меняет участок памяти согласно по массиву байтов в строке
	* @param address Адрес участка памяти с которого будет начинаться запись
	* @param raw Массив байт
	* @param size Размер строки(массива байт)
	* @return Состояние проделанной работы
	*/
	inline BOOL setRaw(uint32_t address, std::vector<uint8_t> &raw)
	{
		DWORD oldProtect;

		if (VirtualProtect(reinterpret_cast<LPVOID>(address), raw.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			memcpy(PVOID(address), &raw[0], raw.size());

			VirtualProtect(reinterpret_cast<LPVOID>(address), raw.size(), oldProtect, NULL);
			return TRUE;
		}
		return FALSE;
	}
	/*
	* @breaf Меняет участок памяти согласно по массиву байтов в строке
	* @param address Адрес участка памяти с которого будет начинаться запись
	* @param raw Сам текст, из которового будет браться массив байт
	* @param size Размер строки(массива байт)
	* @return Состояние проделанной работы
	*/
	inline BOOL setRaw(uint32_t address, const char* raw, size_t size)
	{
		DWORD oldProtect;

		if (VirtualProtect(reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			memcpy(PVOID(address), raw, size);

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
	inline BOOL setRaw(uint32_t address, std::string &raw)
	{
		DWORD oldProtect;
		if (VirtualProtect(reinterpret_cast<LPVOID>(address), raw.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			memcpy(PVOID(address), &raw[0], raw.size());

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
	inline BOOL set(uint32_t address, T value)
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

	/*
	* @breaf Заполняет участок памяти одним значением
	* @param address Адрес участка памяти в котором будет производиться перезапись
	* @param size Размер(кол-во) памяти, которой будет присвоенно значение value
	* @param value Значение которое будет присвоенно
	* @return Состояние проделанной работы
	*/
	inline BOOL fill(uint32_t address, size_t size, uint32_t value)
	{
		DWORD oldProtect;
		if (VirtualProtect(reinterpret_cast<PVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			FillMemory(PVOID(address), size, value);
			VirtualProtect(reinterpret_cast<PVOID>(address), size, oldProtect, NULL);
			return TRUE;
		}
		return FALSE;
	}
}

#endif