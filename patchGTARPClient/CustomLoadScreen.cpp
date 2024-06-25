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
    m_bState = g_Config["customLoadScreen"]["path"].get<std::string>() != "NONE" && g_Config["customLoadScreen"]["splashs"].size();
    if (!m_bState)
        return;

    plugin::Events::initRwEvent += [&]() {
        int   txd = CTxdStore::AddTxdSlot(TEXTURESLOTNAME.data());
        auto& conf = g_Config["customLoadScreen"]["splashs"][snippets::randomInteger(0, g_Config["customLoadScreen"]["splashs"].size() - 1)];
        CTxdStore::LoadTxd(txd, g_Config["customLoadScreen"]["path"].get<std::string>().c_str());
        CTxdStore::AddRef(txd);
        CTxdStore::PushCurrentTxd();
        CTxdStore::SetCurrentTxd(txd);
        m_txtSplash.SetTexture((char*)conf["texture"].get<std::string>().c_str());
        CTxdStore::PopCurrentTxd();


        using namespace Xbyak::util;

        const DWORD addrs[] = {0x58FFFB, 0x59001B, 0x590059};
        for (size_t i{}; i < 3; i++) {
            const auto addr = addrs[i];
            const auto funcCSpriteDrawRect = patch::getDetour<DWORD>(addr);

            auto& code = m_code[i];

            code.push(eax);
            code.mov(eax, dword[esp + 4 + 16 + 0]);
            code.mov(dword[&m_saveRect.left], eax);
            code.mov(eax, dword[esp + 4 + 16 + 4]);
            code.mov(dword[&m_saveRect.bottom], eax);
            code.mov(eax, dword[esp + 4 + 16 + 8]);
            code.mov(dword[&m_saveRect.right], eax);
            code.mov(eax, dword[esp + 4 + 16 + 12]);
            code.mov(dword[&m_saveRect.top], eax); 
            code.pop(eax);

            code.call((PVOID)funcCSpriteDrawRect);

            code.sub(esp, 8);

            code.lea(eax, dword[esp + 16]);
            code.push(DWORD(sizeof(CRect)));
            code.push(DWORD(&m_saveRect));
            code.push(eax);
            code.call(PVOID(&memcpy));
            code.add(esp, 4 * 3);

            code.mov(ecx, (DWORD)&m_txtSplash);
            code.call(PVOID(0x728350));
            code.jmp(PVOID(addr + 5u));

            auto replace = [&](Xbyak::CodeGenerator& cd) {
                cd.jmp(code.getCode<PVOID>());
            };

            patch::writeCode(addr, replace, 5u);
        }


        auto replaceColor = [&](Xbyak::CodeGenerator& cd) {
            cd.mov(dword[esp + 20], (DWORD)(conf["color"].get<int>() | 0xFF'00'00'00));
        };
        patch::writeCode(g_gtarpclientBase.getAddr<uintptr_t>(0x21067), replaceColor, 8u);
    };

    plugin::Events::shutdownRwEvent += [&]() {
        if (m_bState) {
            m_txtSplash.Delete();
            CTxdStore::RemoveTxdSlot(CTxdStore::FindTxdSlot(TEXTURESLOTNAME.data()));
        }
    };
}
