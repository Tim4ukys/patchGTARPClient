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
#include "Config.h"

Config::Config(std::string fileName)
    : m_fileName(std::filesystem::current_path().string() + '\\' + fileName) 
{
    std::filesystem::path cfile{m_fileName};
    if (std::filesystem::exists(cfile)) {
        std::ifstream fconf{m_fileName};
        fconf >> j;

        if (isAllKeysCorrrect()) return;
    }
    loadDefaultConfig();
}
Config::~Config() {
    saveFile();
}

void Config::saveFile() {
    std::ofstream oFile{m_fileName};
    oFile << j.dump(4);
}

void Config::loadDefaultConfig() {
    j = R"(
    {
        "clock": {
            "fixTimeZone": true
        },
        "samp": {
            "fontFaceName": "Comic Sans MS",
            "isCustomFont": true,
            "isSortingScreenshots": true,
            "isWhiteID": true
        },
        "serverIcon": {
            "state": false,
            "x": 656.0,
            "y": 28.0
        },
        "vehicleHud": {
            "isDrawHelpTablet": false
        }
    }
    )"_json;
}

bool Config::isAllKeysCorrrect() {
    if (
        j["serverIcon"].is_structured() ||
        j["serverIcon"]["state"].is_boolean() || j["serverIcon"]["x"].is_number_float() || j["serverIcon"]["y"].is_number_float() ||

        j["vehicleHud"].is_structured() ||
        j["vehicleHud"]["isDrawHelpTablet"].is_boolean() ||

        j["samp"].is_structured() ||
        j["samp"]["isWhiteID"].is_boolean() || j["samp"]["isCustomFont"].is_boolean() || j["samp"]["fontFaceName"].is_string() ||
        j["samp"]["isSortingScreenshots"].is_boolean() ||

        j["clock"].is_structured() ||
        j["clock"]["fixTimeZone"].is_boolean())
        return true;
    else
        return false;
}

nlohmann::json& Config::getJSON() {
    return j;
}
nlohmann::json& Config::operator[](const char* key) {
    return j[key];
}