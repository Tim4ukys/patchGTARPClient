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
* @warning �������� ������ � x86 � x64 ������������
* 
* ������ ��� �������� ������
*/
namespace patch {

    /**
    * @brief ��������� ����� ��� ��������� "������" �� ����� �������.
    */
    class callHook {
    public:
        /**
        * @param HookAddress ������, ��� �������� "CALL" ����������.
        */
        callHook(uintptr_t HookAddress);
        ~callHook();

        /**
        * @brief ������������� ���
        * @param DetourFunction ����� �� �������, ������� ����� ���������� ������ ���
        * @param oldDelet ������� �� ������ ���?
        * @return ����� ��������
        */
        BOOL installHook(void* DetourFunction, bool oldDelete = true);
        /**
        * @param oldAddrFunc ������ ������ �������
        */
        BOOL uninstallHook(uintptr_t oldAddrFunc = NULL);

        template<typename T>
        T getOriginal();

    private:

        BOOL set(uintptr_t HookAddress, void* DetourFunction, uintptr_t* oldCallFnc = nullptr);

        uintptr_t m_nOldCallFunc, m_nHookAddress;
    };

    /*
	* @breaf ���� ����� �� �������(����� ������)
	* @param module �������� ������
	* @param pattern ������ � ���� RAW ������
	* @param mask ������. X - ����� ��������, � ? - ��������������
	* @param startSearchAddr ������ � �������� ����� �������� �����.
	* @return ����� �������
	* @auther fredaikis unknowncheats
    * @warning � ������ ������� ����� ����� ����� ����
	*/
    uintptr_t FindPattern(char* module, char* pattern, char* mask, uintptr_t startSearchAddr = NULL);

    /*
	* @breaf ������������ HEX ������ � ������ � �������
	* @param hex ������ � ������� �������� ����� � HEX �������
	* @return ��������� vector � ������� ������
	*/
    void HEXtoRaw(std::string& hex, std::vector<uint8_t>* raw);

    /**
    * @brief �������� ���������� ������ � ������� ����
    * @param address �����, ���� ����� ������������ ���
    * @param raw ����� � ASM �����
    * @param rawSize ������ ������ 
    * @param saveByte ������ "�����������" ������
    * @param isSave ��������� �� �����
    * @warning � ������ ���� 'saveByte != NULL', � 'isSave = false', �� � address NOP'����� ������� ������, ������� ������� � saveByte.
    */
    BOOL setRawThroughJump(uintptr_t address, const char* raw, size_t rawSize, size_t saveByte = NULL, bool isSave = false);

	/*
	* @breaf ������ offset ���������� � ����
	* @param address ����� ������� ������ � ������� ����� ��� �����������
	* @param offsetAddress ����� ����������
	* @return ��������� ����������� ������
	*/
    BOOL setPushOffset(uintptr_t address, uintptr_t offsetAddress);

    /*
	* @breaf ��������� ������� ������ � ������������ ��� � HEX ������
	* @param address ����� ������� ������ � �������� ����� ���������� ������
	* @param size ������ ������(������� ����)
	* @return HEX ������
	*/
    std::string getHEX(uintptr_t address, size_t size);

    /*
	* @breaf ������ ������� ������ �������� �� ������� ������ � ������
	* @param address ����� ������� ������ � �������� ����� ���������� ������
	* @param raw ������ ����
	* @param size ������ ������(������� ����)
	* @return ��������� ����������� ������
	*/
    BOOL setRaw(uintptr_t address, std::vector<uint8_t>& raw);
    
    /*
	* @breaf ������ ������� ������ �������� �� ������� ������ � ������
	* @param address ����� ������� ������ � �������� ����� ���������� ������
	* @param raw ��� �����, �� ���������� ����� ������� ������ ����
	* @param size ������ ������(������� ����)
	* @return ��������� ����������� ������
	*/
    BOOL setRaw(uintptr_t address, const char* raw, size_t size);

    /*
	* @breaf ������ ������� ������ �������� �� ������� ������ � ������
	* @param address ����� ������� ������ � �������� ����� ���������� ������
	* @param raw ��� �����, �� ���������� ����� ������� ������ ����
	* @return ��������� ����������� ������
	*/
    BOOL setRaw(uintptr_t address, std::string& raw);

    /*
	* @breaf ������ ����������� ������� ������ �� ��������
	* @param address ����� ������� ������ � ������� ����� ������������� ����������
	* @param raw ��� �����, �� ���������� ����� ������� ������ ����
	* @return ��������� ����������� ������
	*/
    template<typename T>
    BOOL set(uintptr_t address, T value);

    /*
	* @breaf ��������� ������� ������ ����� ���������
	* @param address ����� ������� ������ � ������� ����� ������������� ����������
	* @param size ������(���-��) ������, ������� ����� ���������� �������� value
	* @param value �������� ������� ����� ����������
	* @return ��������� ����������� ������
	*/
    BOOL fill(uintptr_t address, size_t size, uint32_t value);
    
} // namespace patch

#endif