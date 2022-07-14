/* Original name: Turn Fucking Radio Off; Author: NarutoUA (blast.hk/members/2504) */
#include "pch.h"
#include "tfro.h"

void TFRO::Process() {
    if (g_Config["gtasa"]["tfro"].get<bool>()) {
        // mov dl, 11
        // nop
        patch__setRaw(0x4EB63D, "\xb2\x0d\x90", 3U);
    }
}
