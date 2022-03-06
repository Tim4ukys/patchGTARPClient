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
* @date 25.02.2022
* @copyright GNU GPLv3
* @warning Работает только с x86 и x64 приложениями
* 
* Модуль для патчинга памяти
*/
namespace patch {

    /**
    * @brief Небольшой класс для установки "Крючка" на вызов функции.
    */
    class callHook {
    public:
        /**
        * @param HookAddress адресс, где начинает "CALL" инструкция.
        */
        callHook(uintptr_t HookAddress);
        ~callHook();

        /**
        * @brief Устанавливает хук
        * @param DetourFunction адрес на функцию, которая будет вызываться вместо той
        * @param oldDelet удалять ли старый хук?
        * @return успех операции
        */
        BOOL installHook(void* DetourFunction, bool oldDelete = true);
        /**
        * @param oldAddrFunc адресс старой функции
        */
        BOOL uninstallHook(uintptr_t oldAddrFunc = NULL);

        DWORD getOriginal() const noexcept;

    private:

        BOOL set(uintptr_t HookAddress, void* DetourFunction, uintptr_t* oldCallFnc = nullptr);

        uintptr_t m_nOldCallFunc, m_nHookAddress;
    };

    /*
	* @breaf Ищет адрес по патерну(куску памяти)
	* @param module Название модуля
	* @param pattern Патерн в виде RAW строки
	* @param mask Макска. X - будут читаться, а ? - игнорироваться
	* @param startSearchAddr Адресс с которого нужно начинать поиск.
	* @return Адрес патерна
	* @auther fredaikis unknowncheats
    * @warning В случае неудачи адрес будет равен нулю
	*/
    uintptr_t FindPattern(char* module, char* pattern, char* mask, uintptr_t startSearchAddr = NULL);

    /*
	* @breaf Конвертирует HEX строку в массив с байтами
	* @param hex Строка в которой записаны числа в HEX формате
	* @return Контейнер vector с байтами внутри
	*/
    void HEXtoRaw(std::string& hex, std::vector<uint8_t>* raw);

    /**
    * @brief помещает содержимое буфера в участок кода
    * @param address Адрес, куда будет встраиваться код
    * @param raw буфер с ASM кодом
    * @param rawSize размер буфера 
    * @param saveByte размер "сохраняемых" байтов
    * @param isSave сохранять ли байты
    * @warning В случае если 'saveByte != NULL', а 'isSave = false', то в address NOP'яться столько байтов, сколько указано в saveByte.
    */
    BOOL setRawThroughJump(uintptr_t address, const char* raw, size_t rawSize, size_t saveByte = NULL, bool isSave = false);

    /**
    * @brief встраивает код путём создания Jump на detour
    * @param address Адрес, куда будет встраиваться код
    * @param detour адрес, куда будет сделан прыжок
    * @param saveByte размер "сохраняемых" байтов
    * @param isSave сохранять ли байты
    * @warning В случае если 'saveByte != NULL', а 'isSave = false', то в address NOP'яться столько байтов, сколько указано в saveByte.
    */
    BOOL setJump(uintptr_t address, uintptr_t detour, size_t saveByte = NULL, bool isSave = false);

	/*
	* @breaf Пихает offset переменной в стек
	* @param address Адрес участка памяти в котором будет это происходить
	* @param offsetAddress Адрес переменной
	* @return Состояние проделанной работы
	*/
    BOOL setPushOffset(uintptr_t address, uintptr_t offsetAddress);

    /*
	* @breaf Считывает участок памяти и переобразует его в HEX строку
	* @param address Адрес участка памяти с которого будет начинаться чтение
	* @param size Размер строки(массива байт)
	* @return HEX строка
	*/
    std::string getHEX(uintptr_t address, size_t size);

    /*
	* @breaf Меняет участок памяти согласно по массиву байтов в строке
	* @param address Адрес участка памяти с которого будет начинаться запись
	* @param raw Массив байт
	* @param size Размер строки(массива байт)
	* @return Состояние проделанной работы
	*/
    BOOL setRaw(uintptr_t address, std::vector<uint8_t>& raw);
    
    /*
	* @breaf Меняет участок памяти согласно по массиву байтов в строке
	* @param address Адрес участка памяти с которого будет начинаться запись
	* @param raw Сам текст, из которового будет браться массив байт
	* @param size Размер строки(массива байт)
	* @return Состояние проделанной работы
	*/
    BOOL setRaw(uintptr_t address, const char* raw, size_t size);

    /*
	* @breaf Меняет участок памяти согласно по массиву байтов в строке
	* @param address Адрес участка памяти с которого будет начинаться запись
	* @param raw Сам текст, из которового будет браться массив байт
	* @return Состояние проделанной работы
	*/
    BOOL setRaw(uintptr_t address, std::string& raw);

    /*
	* @breaf Меняет определённый участок памяти по значению
	* @param address Адрес участка памяти в котором будет производиться перезапись
	* @param raw Сам текст, из которового будет браться массив байт
	* @return Состояние проделанной работы
	*/
    template<typename T>
    BOOL set(uintptr_t address, T value);

    /*
	* @breaf Заполняет участок памяти одним значением
	* @param address Адрес участка памяти в котором будет производиться перезапись
	* @param size Размер(кол-во) памяти, которой будет присвоенно значение value
	* @param value Значение которое будет присвоенно
	* @return Состояние проделанной работы
	*/
    BOOL fill(uintptr_t address, size_t size, uint32_t value);
    
} // namespace patch

#endif