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
#include "CustomConnectScreen.h"
#include "offsets.hpp"
#include "CWeather.h"

CustomConnectScreen::sub_1F270_t CustomConnectScreen::sub_1F270_orig = nullptr;
unsigned char                    CustomConnectScreen::s_nHour;
bool __fastcall CustomConnectScreen::sub_1F270(DWORD* pthis, void* trash, void* a2, int a3, char a4) {
    auto r = sub_1F270_orig(pthis, trash, a2, a3, a4);
    g_pSAMP->setHour(s_nHour);
    return r;
}

void CustomConnectScreen::init() {
    if (!g_Config["customScreen"]["state"].get<bool>() || !g_Config["customScreen"]["screens"].size())
        return;

    auto& conf = g_Config["customScreen"]["screens"][snippets::randomInteger(0, g_Config["customScreen"]["screens"].size() - 1)];
    
    {
        size_t val = snippets::randomInteger(0, conf["weather"].size() - 1);
        auto   weather = (short)conf["weather"][val].get<int>();
        g_onInitSamp += [weather]() {
            CWeather::OldWeatherType = weather;
        };
        s_nHour = conf["time"][val].get<int>();
        patch::writeMemory<unsigned char>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::CONSCREEN_TIME),
                                          s_nHour);
    }
    
    patch::writeMemory<float>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::CONSCREEN_POS_X),
                              conf["camera"][0].get<float>());
    patch::writeMemory<float>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::CONSCREEN_POS_Y),
                              conf["camera"][1].get<float>());
    patch::writeMemory<float>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::CONSCREEN_POS_Z),
                              conf["camera"][2].get<float>());
    
    patch::writeMemory<float>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::CONSCREEN_POINT_X),
                              conf["point"][0].get<float>());
    patch::writeMemory<float>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::CONSCREEN_POINT_Y),
                              conf["point"][1].get<float>());
    patch::writeMemory<float>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::CONSCREEN_POINT_Z),
                              conf["point"][2].get<float>());

    // fix change time on connect server
    plugin::patch::ReplaceFunctionCall(g_sampBase.getAddr<int>(0x104F0), &sub_1F270);
    sub_1F270_orig = g_sampBase.getAddr<sub_1F270_t>(0x1F270);

    static int s_nCountWait{};
    rakhook::on_send_rpc +=
        [](int& id, RakNet::BitStream* bs, PacketPriority& priority, PacketReliability& reliability, char& ord_channel, bool& sh_timestamp) -> bool {
        if (id == 25) {
            s_nCountWait += 3;
        }
        return true;
    };
    rakhook::on_receive_rpc +=
        [](unsigned char& id, RakNet::BitStream* bs) -> bool {
        if ((id == 157 || id == 158) && s_nCountWait > 0) {
            --s_nCountWait;
            return false;
        } else if (id == 129 && s_nCountWait > 0) {
            --s_nCountWait;
            CWeather::OldWeatherType = 0;
            //g_pSAMP->addChatMessage(-1, "reset weather");
        }
        return true;
    };
}