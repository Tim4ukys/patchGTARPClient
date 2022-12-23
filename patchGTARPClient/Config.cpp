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
    } else
        g_Log << "[Config]: file don't exists. Recreate..";

    restoreAndCheckKeysCorrect();
    saveFile();
}
Config::~Config() {
    saveFile();
}

void Config::saveFile() {
    std::ofstream oFile{m_fileName};
    oFile << j.dump(4);
}

#define SET_DEFAULT_STR(js, str_key, str_def) \
    if (!(js)[str_key].is_string()) { \
        (js)[str_key] = str_def; \
    }

#define SET_DEFAULT_INT(js, str_key, int_def) \
    if (!(js)[str_key].is_number_integer()) { \
        (js)[str_key] = int_def; \
    }

#define SET_DEFAULT_FLT(js, str_key, flt_def) \
    if (!(js)[str_key].is_number_float()) { \
        (js)[str_key] = flt_def; \
    }

#define SET_DEFAULT_BOOL(js, str_key, bool_def) \
    if (!(js)[str_key].is_boolean()) { \
        (js)[str_key] = bool_def; \
    }

void Config::restoreAndCheckKeysCorrect() {
    auto safeLoadStruct = [](nlohmann::json& jch, const char* key, std::function<void(nlohmann::json&)> fnc) -> void {
        if (!jch[key].is_structured() && !jch[key].is_null()) {
            //g_Log.Write("type: %s", jch.type_name());
            jch.erase(key);
        }
        fnc(jch[key]);
    };
    //SET_DEFAULT_STR(j, "version", g_szCurrentVersion);
    safeLoadStruct(j, "gtasa", [](nlohmann::json& jn) {
        SET_DEFAULT_BOOL(jn, "tfro", true);
    });
    safeLoadStruct(j, "oldHud", [](nlohmann::json &jn) {
        SET_DEFAULT_BOOL(jn, "radar", false)
        SET_DEFAULT_BOOL(jn, "hud", true)
        SET_DEFAULT_STR(jn, "pathToTXDhud", "NONE")
        SET_DEFAULT_BOOL(jn, "radarScaleFix", false)
    });
    safeLoadStruct(j, "clock", [](nlohmann::json& jn) {
        SET_DEFAULT_BOOL(jn, "fixTimeZone", true)
    });
    safeLoadStruct(j, "samp", [](nlohmann::json& jn) {
        SET_DEFAULT_STR(jn, "fontFaceName", "Comic Sans MS")
        SET_DEFAULT_BOOL(jn, "isCustomFont", false)
        SET_DEFAULT_BOOL(jn, "isSortingScreenshots", true)
        SET_DEFAULT_BOOL(jn, "isWhiteID", true)
        SET_DEFAULT_BOOL(jn, "isCustomF1", true)
        SET_DEFAULT_BOOL(jn, "isMakeQuickScreenshot", true)
        SET_DEFAULT_BOOL(jn, "isPlaySoundAfterMakeScreenshot", true)
        SET_DEFAULT_STR(jn, "formatScreenshotIMG", "PNG");
    });
    safeLoadStruct(j, "serverIcon", [](nlohmann::json& jn) {
        SET_DEFAULT_BOOL(jn, "state", false)
        SET_DEFAULT_FLT(jn, "x", 656.0)
        SET_DEFAULT_FLT(jn, "y", 28.0)
        SET_DEFAULT_FLT(jn, "width", 366.0)
        SET_DEFAULT_FLT(jn, "height", 144.0)
    });
    safeLoadStruct(j, "vehicleHud", [](nlohmann::json& jn) {
        SET_DEFAULT_BOOL(jn, "isDrawHelpTablet", false)
        SET_DEFAULT_BOOL(jn, "isDisableSnowWindow", true)
    });
}

nlohmann::json& Config::getJSON() {
    return j;
}
nlohmann::json& Config::operator[](const char* key) {
    return j[key];
}