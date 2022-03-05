#pragma once

namespace snippets 
{
    class DynamicLibrary {
    public:

        DynamicLibrary(const char* libName);
        ~DynamicLibrary();

        DWORD getAddress() noexcept;
        DWORD getAddress(DWORD offset) noexcept;

        template<typename T>
        DWORD getAddress(T offset) noexcept;
    
    private:
        
        char* m_pModuleName;
        union {
            HANDLE pHandl;
            DWORD  dwHandl;
        } m_unDllAddress;
    };

    /*
    * @brief ��������� UTF8 � CP1251 ���������
    * @param utf8 ������ UTF8
    * @return ������ � ��������� CP1251
    */
    std::string UTF8_to_CP1251(std::string const& utf8);

    /**
    * @brief ������������ wchar_t � string
    * @param wstr unicode ������
    * @return char ������
    */
    std::string ConvertWideToANSI(const wchar_t* wstr);
};