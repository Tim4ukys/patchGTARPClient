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
* @warning Работает только с x86 приложениями
* 
* Модуль для патчинга памяти
*/

namespace patch {

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

}

#define PATCH_EXTR_C extern "C"

/*
* @breaf Ищет адрес по патерну(куску памяти)
* @param module Название модуля
* @param pattern Патерн в виде RAW строки
* @param mask Макска. X - будут читаться, а ? - игнорироваться
* @param startSearchAddr Адресс с которого нужно начинать поиск.
* @return Адрес патерна
* @warning В случае неудачи адрес будет равен нулю
* @auther fredaikis unknowncheats
*/
PATCH_EXTR_C uint32_t patch__FindPattern(_In_ const char*  szModule,
                                         _In_ const char*  szPattern,
                                         _In_ const char*  mask,
                                         _In_opt_ uint32_t startSearchAddr);

/*
* @brief Помещает содержимое RAW буффера в код, путём создания копии буффера и jump прыжка на него и из него
* @param address Адрес, где будет совершён jump
* @param raw буффер с ASM кодом
* @param rawSize размер буффера
* @param saveByte кол-во "сохраняемых" байтов
* @param isSave сохранять ли байты, где будет вставлен jump
* @warning В случае если 'saveByte != NULL', а 'isSave = false', то в address NOP'яться столько байтов, сколько указано в saveByte.
*/
PATCH_EXTR_C BOOL patch__setRawThroughJump(_In_ uint32_t    address,
                                           _In_ const char* raw,
                                           _In_ size_t      rawSize,
                                           _In_opt_ size_t  saveByte,
                                           _In_opt_ BOOL    isSave);

/*
* @brief Создаёт jump на участок памяти, в котором будет call detour
* @param address Адрес, где будет вставлен jump
* @param detour адрес функции, которая будет вызываться
* @param saveByte кол-во "сохраняемых" байтов
* @param isSave сохранять ли байты, где будет вставлен jump
* @warning В случае если 'saveByte != NULL', а 'isSave = false', то в address NOP'яться столько байтов, сколько указано в saveByte.
*/
PATCH_EXTR_C BOOL patch__setJump(_In_ uint32_t   address,
                                 _In_ uint32_t   detour,
                                 _In_opt_ size_t saveByte,
                                 _In_opt_ BOOL   isSave);

/*
* @breaf Создаёт "push *адрес*" в коде
* @param address Адрес участка памяти в котором будет это происходить
* @param offsetAddress Адрес адреса, который будет использоваться в push
* @return Результат проделанной работы
*/
PATCH_EXTR_C BOOL patch__setPushOffset(_In_ uint32_t address,
                                       _In_ uint32_t offsetAddress);

/*
* @breaf Считывает участок памяти и переобразует его в HEX строку
* @param address Адрес участка памяти с которого будет начинаться чтение
* @param outBuff Буффер, в который будет записываться строка
* @param size Кол-во байт, которые нужно будет переобразовать в строку
* @warning Буффер для строки должен быть больше "size" в два раза + 1(для \0)
* @return Результат проделанной работы
*/
PATCH_EXTR_C BOOL patch__getHEX(_In_ uint32_t address,
                                _Out_ char*   outBuff,
                                _In_ size_t   size);

/*
* @breaf Меняет участок памяти согласно по массиву байтов в строке
* @param address Адрес участка памяти с которого будет начинаться запись
* @param raw Сам текст, из которового будет браться массив байт
* @param size Размер строки(массива байт)
* @return Результат проделанной работы
*/
PATCH_EXTR_C BOOL patch__setRaw(_In_ uint32_t    address,
                                _In_ const char* raw,
                                _In_ size_t      size);

/*
* @breaf Заполняет участок памяти одним значением
* @param address Адрес участка памяти в котором будет производиться перезапись
* @param size Размер(кол-во) памяти, которой будет присвоенно значение value
* @param value Значение которое будет присвоенно
* @return Результат проделанной работы
*/
PATCH_EXTR_C BOOL patch__fill(_In_ uint32_t address,
                              _In_ size_t   size,
                              _In_ uint32_t value);

#undef PATCH_EXTR_C
#endif