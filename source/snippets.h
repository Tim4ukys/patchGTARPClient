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

    std::string UTF8_to_CP1251(std::string const& utf8);
};