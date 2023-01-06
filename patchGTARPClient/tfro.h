/* Original name: Turn Fucking Radio Off; Author: NarutoUA (blast.hk/members/2504) */
#pragma once

class TFRO : public MLoad {
    std::uint8_t m_oldByte[3];

    void turnOn() override;
    void turnOff() override;

public:
    void init() override;
    ~TFRO() override;
};