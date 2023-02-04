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
#include "Updater.h"

void Updater::sync() {
    if (m_bWasSync)
        return;
    m_bWasSync = true;

    auto        j = nlohmann::json::parse(client::downloadStringFromURL(R"(https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json)"));
    std::string s;
    j["vers"].get_to(s);
    auto [vMaj, vMin, vPatch] = snippets::versionParse(s);
    m_sync.maj = vMaj;
    m_sync.min = vMin;
    m_sync.patch = vPatch;
    g_Log.Write("Git version: vMaj: %d | vMid: %d | vPatch: %d", vMaj, vMin, vPatch);
}

