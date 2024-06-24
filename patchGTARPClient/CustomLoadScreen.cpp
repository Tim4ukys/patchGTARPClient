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
#include "pch.h"
#include "CustomLoadScreen.h"
#include <CTxdStore.h>

void CustomLoadScreen::init() {
    if (g_Config["customLoadScreen"]["path"].get<std::string>() == "NONE" || !g_Config["customLoadScreen"]["splashs"].size())
        return;

    plugin::Events::initRwEvent += [&]() {
        int   txd = CTxdStore::AddTxdSlot("CUSTOMLOADSCREEN_MY");
        auto& conf = g_Config["customLoadScreen"]["splashs"][snippets::randomInteger(0, g_Config["customLoadScreen"]["splashs"].size() - 1)];
        CTxdStore::LoadTxd(txd, g_Config["customLoadScreen"]["path"].get<std::string>().c_str());
        CTxdStore::AddRef(txd);
        CTxdStore::PushCurrentTxd();
        CTxdStore::SetCurrentTxd(txd);
        m_txtSplash.SetTexture((char*)conf["texture"].get<std::string>().c_str());
        CTxdStore::PopCurrentTxd();



        /*
        ”∆≈ œ–Œ¬≈–»À, –¿¡Œ“¿≈“ Õ¿ ¬—≈’ ¿ƒ–≈—¿’: 0x58FFFB 0x59001B 0x590059

        TO-DO: —‰ÂÎ‡Ú¸ ˆËÍÎÓÏ ‰Îˇ ‚ÒÂı ‡‰ÂÒÓ‚
        */
        using namespace Xbyak::util;

        constexpr DWORD addr = 0x58FFFB;
        auto            funcCSpriteDrawRect = patch::getDetour<DWORD>(addr);

        m_code.push(eax);
        m_code.mov(eax, dword[esp + 4 + 16 + 0]);
        m_code.mov(dword[&m_saveRect.left], eax);
        m_code.mov(eax, dword[esp + 4 + 16 + 4]);
        m_code.mov(dword[&m_saveRect.bottom], eax);
        m_code.mov(eax, dword[esp + 4 + 16 + 8]);
        m_code.mov(dword[&m_saveRect.right], eax);
        m_code.mov(eax, dword[esp + 4 + 16 + 12]);
        m_code.mov(dword[&m_saveRect.top], eax);
        m_code.pop(eax);

        m_code.call((PVOID)funcCSpriteDrawRect);
        
        m_code.sub(esp, 8);
        
        m_code.lea(eax, dword[esp + 16]);
        m_code.push(DWORD(sizeof(CRect)));
        m_code.push(DWORD(&m_saveRect));
        m_code.push(eax);
        m_code.call(PVOID(&memcpy));
        m_code.add(esp, 4 * 3);

        m_code.mov(ecx, (DWORD)&m_txtSplash);
        m_code.call(PVOID(0x728350));
        m_code.jmp(PVOID(addr + 5u));

        auto replace = [&](Xbyak::CodeGenerator& cd) {
            cd.jmp(m_code.getCode<PVOID>());
        };

        patch::writeCode(addr, replace, 5u);




        auto replaceColor = [&](Xbyak::CodeGenerator& cd) {
            cd.mov(dword[esp + 20], (DWORD)(conf["color"].get<int>() | 0xFF'00'00'00));
        };
        patch::writeCode(g_gtarpclientBase.getAddr<uintptr_t>(0x21067), replaceColor, 8u);
    };
}
