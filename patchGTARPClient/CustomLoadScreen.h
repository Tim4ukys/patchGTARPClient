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
#pragma once
#include "MLoad.h"

class CustomLoadScreen : public MLoad {
    CSprite2d m_txtSplash;
    Xbyak::CodeGenerator m_code;
    CRect                m_saveRect;

public:

    void init() override;
};
