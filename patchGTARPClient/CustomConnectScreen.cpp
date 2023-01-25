#include "pch.h"
#include "CustomConnectScreen.h"
#include "offsets.hpp"
#include "CWeather.h"

void CustomConnectScreen::init() {
    if (!g_Config["customScreen"]["state"].get<bool>()) return;

    auto& conf = g_Config["customScreen"]["screens"][snippets::randomInteger(0, g_Config["customScreen"]["screens"].size() - 1)];
    
    {
        size_t val = snippets::randomInteger(0, conf["weather"].size() - 1);
        auto   weather = (short)conf["weather"][val].get<int>();
        g_onInitSamp += [weather]() {
            CWeather::OldWeatherType = weather;
        };
        patch::writeMemory<unsigned char>(g_gtarpclientBase.getAddr<std::uintptr_t>(OFFSETS::GTARP::CONSCREEN_TIME),
                                          conf["time"][val].get<int>());
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
}