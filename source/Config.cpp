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
    auto safeLoadStruct = [](nlohmann::json* jch, std::function<void()> fnc) -> void {
        if (!jch->is_structured()) {
            jch->clear();
        }
        fnc();
    };
    nlohmann::json* chkJs = &j["oldHud"];
    safeLoadStruct(chkJs, [chkJs]() {
        SET_DEFAULT_BOOL(*chkJs, "radar", false)
        SET_DEFAULT_BOOL(*chkJs, "hud", true)
        SET_DEFAULT_STR(*chkJs, "pathToTXDhud", "NONE")
    });
    chkJs = &j["clock"];
    safeLoadStruct(chkJs, [chkJs]() {
        SET_DEFAULT_BOOL(*chkJs, "fixTimeZone", true)
    });
    chkJs = &j["samp"];
    safeLoadStruct(chkJs, [chkJs]() {
        SET_DEFAULT_STR(*chkJs, "fontFaceName", "Comic Sans MS")
        SET_DEFAULT_BOOL(*chkJs, "isCustomFont", false)
        SET_DEFAULT_BOOL(*chkJs, "isSortingScreenshots", true)
        SET_DEFAULT_BOOL(*chkJs, "isWhiteID", true)
        SET_DEFAULT_BOOL(*chkJs, "isCustomF1", true)
        SET_DEFAULT_BOOL(*chkJs, "isMakeQuickScreenshot", true)
        SET_DEFAULT_BOOL(*chkJs, "isPlaySoundAfterMakeScreenshot", true)
    });
    chkJs = &j["serverIcon"];
    safeLoadStruct(chkJs, [chkJs]() {
        SET_DEFAULT_BOOL(*chkJs, "state", false)
        SET_DEFAULT_FLT(*chkJs, "x", 656.0)
        SET_DEFAULT_FLT(*chkJs, "y", 28.0)
    });
    chkJs = &j["vehicleHud"];
    safeLoadStruct(chkJs, [chkJs]() {
        SET_DEFAULT_BOOL(*chkJs, "isDrawHelpTablet", false)
        SET_DEFAULT_BOOL(*chkJs, "isDisableSnowWindow", true)
    });
}

nlohmann::json& Config::getJSON() {
    return j;
}
nlohmann::json& Config::operator[](const char* key) {
    return j[key];
}