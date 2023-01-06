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

class MLoad {
protected:
    bool m_bState{};

    virtual void turnOn(){};
    virtual void turnOff(){};

public:
    virtual ~MLoad(){};
    virtual void init() = NULL;

    void Toggle(){
        if (m_bState) turnOff();
        else turnOn();
        m_bState = !m_bState; 
    };
};
