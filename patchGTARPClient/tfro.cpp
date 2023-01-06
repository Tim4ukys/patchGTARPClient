/* Original name: Turn Fucking Radio Off; Author: NarutoUA (blast.hk/members/2504) */
#include "pch.h"
#include "tfro.h"

void TFRO::turnOn() {
    // mov dl, 11
    // nop
    patch::setRaw(0x4EB63D, "\xb2\x0d\x90");
}

void TFRO::turnOff() {
    patch::setBytes(0x4EB63D, m_oldByte);
}

void TFRO::init() {
    patch::getBytes(0x4EB63D, m_oldByte);

    // ----

    if (m_bState = g_Config["gtasa"]["tfro"].get<bool>())
        turnOn();
}
TFRO::~TFRO() {
    if (m_bState)
        turnOff();
}