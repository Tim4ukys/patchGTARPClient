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

Config::Config(std::string_view fileName)
    : m_fileName(std::filesystem::current_path().string() + '\\' + fileName.data()) 
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

template<typename T>
inline void SET_DEFAULT_PROP(nlohmann::json& js, std::string_view key, T&& param) {
    bool r{};
    if constexpr (std::is_same_v<T, bool>)
        r = !js[key.data()].is_boolean();
    else if constexpr (std::is_same_v<T, float>)
        r = !js[key.data()].is_number_float();
    else if constexpr (std::is_same_v<T, int>)
        r = !js[key.data()].is_number_integer();
    else
        static_assert(true && "Unkown type");

    if (r)
        js[key.data()] = param;
    }
template<std::size_t N>
inline void SET_DEFAULT_PROP(nlohmann::json& js, std::string_view key, const char (&param)[N]) {
    if (!js[key.data()].is_string()) {
        js[key.data()] = param;
    }
    }

inline void SET_DEFAULT_ARR(nlohmann::json& js, std::string_view key, const nlohmann::json& arr, nlohmann::json::value_t type) {
    if (!js[key.data()].is_array()) {
        js[key.data()] = arr;
    } else {
        for (size_t i{}; i < js[key.data()].size(); ++i) {
            if (js[key.data()][i].type() != type) {
                js[key.data()] = arr;
                break;
            }
        }
    }
}

void Config::restoreAndCheckKeysCorrect() {
    auto safeLoadStruct = [](nlohmann::json& jch, std::string_view key, std::function<void(nlohmann::json&)> fnc) -> void {
        if (!jch[key.data()].is_structured() && !jch[key.data()].is_null()) {
            //g_Log.Write("type: %s", jch.type_name());
            jch.erase(key.data());
        }
        fnc(jch[key.data()]);
    };
    auto safeLoadArray = [](nlohmann::json& jch, std::string_view key, std::function<void(nlohmann::json&)> fnc,
                            std::function<void(nlohmann::json&)> def_load) -> void {
        if (!jch[key.data()].is_array() && !jch[key.data()].is_null()) {
            jch.erase(key.data());
        }
        const auto len = jch[key.data()].size();
        if (len != 0) {
            for (size_t i{}; i < len; ++i) {
                fnc(jch[key.data()][i]);
            }
        } else {
            def_load(jch[key.data()]);
        }
    };
    //SET_DEFAULT_STR(j, "version", g_szCurrentVersion);
    safeLoadStruct(j, "gtasa", [](nlohmann::json& jn) {
        SET_DEFAULT_PROP(jn, "tfro", true);
    });
    safeLoadStruct(j, "oldHud", [](nlohmann::json &jn) {
        SET_DEFAULT_PROP(jn, "radar", false);
        SET_DEFAULT_PROP(jn, "hud", true);
        SET_DEFAULT_PROP(jn, "pathToTXDhud", "NONE");
        SET_DEFAULT_PROP(jn, "radarScaleFix", false);
    });
    safeLoadStruct(j, "clock", [](nlohmann::json& jn) {
        SET_DEFAULT_PROP(jn, "fixTimeZone", true);
    });
    safeLoadStruct(j, "samp", [](nlohmann::json& jn) {
        SET_DEFAULT_PROP(jn, "fontFaceName", "Comic Sans MS");
        SET_DEFAULT_PROP(jn, "isCustomFont", false);
        SET_DEFAULT_PROP(jn, "customFontWeight", 400);
        if (jn["customFontWeight"].get<int>() != 400 && jn["customFontWeight"].get<int>() != 700) {
            jn["customFontWeight"] = 400;
        }
        SET_DEFAULT_PROP(jn, "customFontHeight", 0);
        if (jn["customFontHeight"].get<int>() >= -3 && jn["customFontHeight"].get<int>() <= 5) {
            jn["customFontHeight"] = 0;
        }
        SET_DEFAULT_PROP(jn, "isSortingScreenshots", true);
        SET_DEFAULT_PROP(jn, "isWhiteID", true);
        SET_DEFAULT_PROP(jn, "isCustomF1", true);
        SET_DEFAULT_PROP(jn, "isMakeQuickScreenshot", true);
        SET_DEFAULT_PROP(jn, "isPlaySoundAfterMakeScreenshot", true);
        SET_DEFAULT_PROP(jn, "formatScreenshotIMG", "PNG");
    });
    safeLoadStruct(j, "serverIcon", [](nlohmann::json& jn) {
        SET_DEFAULT_PROP(jn, "state", false);
        SET_DEFAULT_PROP(jn, "x", 656.0);
        SET_DEFAULT_PROP(jn, "y", 28.0);
        SET_DEFAULT_PROP(jn, "width", 366.0);
        SET_DEFAULT_PROP(jn, "height", 144.0);
    });
    safeLoadStruct(j, "vehicleHud", [](nlohmann::json& jn) {
        SET_DEFAULT_PROP(jn, "isDrawHelpTablet", false);
        SET_DEFAULT_PROP(jn, "isDisableSnowWindow", true);
    });
    safeLoadStruct(j, "customScreen", [safeLoadArray](nlohmann::json& jn) {
        SET_DEFAULT_PROP(jn, "state", true);
        safeLoadArray(
            jn, "screens",
            [](nlohmann::json& jn) {
                SET_DEFAULT_ARR(jn, "camera", {835.89148f, -94.770203f, 40.035099f}, json::value_t::number_float);
                SET_DEFAULT_ARR(jn, "point", {659.96997f, -90.027298f, 6.7947001f}, json::value_t::number_float);
                SET_DEFAULT_ARR(jn, "time", {2}, json::value_t::number_unsigned);
                SET_DEFAULT_ARR(jn, "weather", {0}, json::value_t::number_unsigned);
                if (jn["time"].size() != jn["weather"].size()) {
                    g_Log.Write(R"(jn["time"].size() != jn["weather"].size())");
                    jn["time"] = {2};
                    jn["weather"] = {0};
                }
            },
            [](nlohmann::json& jn) {
                jn = R"([
                {
                    "camera": [ 2669.3742675781, -1318.3228759766, 92.652923583984 ],
                    "point": [ 2668.7307128906, -1319.0095214844, 92.314979553223 ],
                    "time": [ 2 ],
                    "weather": [ 0 ]
                },
                {
                    "camera": [ 2252.419921875, -2244.7614746094, 58.224502563477 ],
                    "point": [ 2251.6882324219, -2244.181640625, 57.866134643555 ],
                    "time": [ 12 ],
                    "weather": [ 21 ]
                },
                {
                    "camera": [ 2472.2346191406, -1436.2554931641, 94.704544067383 ],
                    "point": [ 2473.0703125, -1435.7905273438, 94.412178039551 ],
                    "time": [ 3 ],
                    "weather": [ 21 ]
                },
                {
                    "camera": [ 771.84381103516, -2374.1923828125, 65.668548583984 ],
                    "point": [ 771.12982177734, -2373.5791015625, 65.330642700195 ],
                    "time": [ 0 ],
                    "weather": [ 21 ]
                },
                {
                    "camera": [ 174.1121673584, -2454.3811035156, 64.906684875488 ],
                    "point": [ 173.82850646973, -2453.6262207031, 64.315361022949 ],
                    "time": [ 0 ],
                    "weather": [ 1 ]
                },
                {
                    "camera": [ 47.522113800049, -3009.2365722656, 74.479293823242 ],
                    "point": [ 46.951133728027, -3008.4533691406, 74.233139038086 ],
                    "time": [ 12, 0 ],
                    "weather": [ 8, 9 ]
                },
                {
                    "camera": [ 942.20391845703, -2668.2263183594, 116.60377502441 ],
                    "point": [ 941.546875, -2667.7329101563, 116.03379058838 ],
                    "time": [ 2 ],
                    "weather": [ 20 ]
                },
                {
                    "camera": [ 2510.9289550781, -776.1796875, 25.263519287109 ],
                    "point": [ 2510.4526367188, -775.3056640625, 25.167667388916 ],
                    "time": [ 5 ],
                    "weather": [ 11 ]
                }
            ])"_json;
            });
    });
}

nlohmann::json& Config::getJSON() {
    return j;
}
nlohmann::json& Config::operator[](const char* key) {
    return j[key];
}