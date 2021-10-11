#ifndef _SNIPPETS_HPP_
#define _SNIPPETS_HPP_

#include <string>
#include <Windows.h>

class DynamicLibrary {
public:

    inline DynamicLibrary(std::string name) : m_moduleName(name), m_baseAddress(NULL)
    {

    }

    inline uint32_t getAddress(uint32_t offset = NULL)
    {
        if (!m_baseAddress)
        {
            m_baseAddress = reinterpret_cast<uint32_t>( GetModuleHandleA(m_moduleName.c_str()) );
        }

        return m_baseAddress + offset;
    }

    inline uint32_t getAddress(OFFSETS offset)
    {
        if (!m_baseAddress)
        {
            m_baseAddress = reinterpret_cast<uint32_t>(GetModuleHandleA(m_moduleName.c_str()));
        }

        return m_baseAddress + static_cast<int>(offset);
    }

private:

    uint32_t m_baseAddress;
    std::string m_moduleName;
};

#endif