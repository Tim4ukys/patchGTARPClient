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
// STL: std::stringstream
#include <sstream>
// STL: std:setfill, std::setw
#include <iomanip>
// C (Old-Library): strlen()
#include <cstring>
// C (Old-Library): uint8_t, uint16_t, uint32_t, uint64_t
#include <cstdint>
// C (Old-Library): memcpy, ZeroMemory, FillMemory


namespace patch
{
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
		std::stringstream sstream{};
		sstream << std::hex << offsetAddress;

		std::vector<uint8_t> raw;
		HEXtoRaw(sstream.str(), &raw);
		if (!raw.size()) return FALSE;

		DWORD oldProtect;
		if (VirtualProtect(reinterpret_cast<LPVOID>(address), 5, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			uint8_t* pAddr = reinterpret_cast<uint8_t*>(address);
			*pAddr = 0x68;
			for (size_t i = 1; i < 5; i++)
			{
				size_t vecPos = 4 - i;
				pAddr[i] = raw.size() > vecPos ? raw[vecPos] : 0;
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