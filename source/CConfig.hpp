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
#ifndef _CCONFIG_HPP_
#define _CCONFIG_HPP_

#include <fstream>
#include <Windows.h>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class CConfig {
public:

	struct stConfig {
		struct ServerIcon {
			bool m_bState;

			float m_fX;
			float m_fY;
		} m_serverIcon;

		struct VehicleHUD {
			bool m_bIsDrawHelpTablet;
		} m_vehHud;

		struct Samp {
			bool m_bIsWhiteID;

			bool m_bIsCustomFont;
			std::string m_FontFaceName;

			bool m_bIsSortingScreenshots;
		} m_samp;
	};

	inline CConfig(std::wstring fileName)
	{
		fullPathToLogFile = getFullPath() + fileName;
		std::ifstream inFileStream{ fullPathToLogFile.c_str() };
		
		if (!inFileStream.fail()) {
			json j;
			inFileStream >> j;
			inFileStream.close();

			if (isAllKeysCorrrect(j))
			{
				config = JSON_TO_CONFIG(j);
			}
			else
			{
				loadDefaultConfig();
			}
		}
		else
		{
			loadDefaultConfig();
		}
	}
	inline ~CConfig()
	{
		saveSettings();
	}

	inline void saveSettings() {
		std::ofstream outFileStream{ fullPathToLogFile.c_str() };
		if (!outFileStream.fail()) {
			json j = CONFIG_TO_JSON(config);
			outFileStream << j.dump(4);
			outFileStream.close();
		}
	}

	inline stConfig* getConfig()
	{
		return &config;
	}

private:

	inline BOOL isAllKeysCorrrect(json& j)
	{
		if (j["serverIcon"].is_null() ||
			j["serverIcon"]["state"].is_null() || j["serverIcon"]["x"].is_null() || j["serverIcon"]["y"].is_null() ||

			j["vehicleHud"].is_null() ||
			j["vehicleHud"]["isDrawHelpTablet"].is_null() ||

			j["samp"].is_null() ||
			j["samp"]["isWhiteID"].is_null() || j["samp"]["isCustomFont"].is_null() || j["samp"]["fontFaceName"].is_null() ||
			j["samp"]["isSortingScreenshots"].is_null())
		{
			return FALSE;
		}

		return TRUE;
	}

	inline void loadDefaultConfig()
	{
		config = {
			/*ServerIcon*/
			{
				false,
				656,
				28
			},
			/*VehicleHUD*/
			{
				false
			},
			/*Samp*/
			{
				true,
				false,
				"Segoe UI",
				false
			}
		};
	}

	inline stConfig JSON_TO_CONFIG(json& j)
	{
		return {
			{
				j["serverIcon"]["state"].get<bool>(),
				j["serverIcon"]["x"].get<float>(),
				j["serverIcon"]["y"].get<float>(),
			},
			{
				j["vehicleHud"]["isDrawHelpTablet"].get<bool>()
			},
			{
				j["samp"]["isWhiteID"].get<bool>(),
				j["samp"]["isCustomFont"].get<bool>(),
				j["samp"]["fontFaceName"].get<std::string>(),
				j["samp"]["isSortingScreenshots"].get<bool>()
			}
		};
	}
	inline json CONFIG_TO_JSON(stConfig& conf)
	{
		json j;
		
		// ServerIcon
		j["serverIcon"]["state"] = conf.m_serverIcon.m_bState;
		j["serverIcon"]["x"] = conf.m_serverIcon.m_fX;
		j["serverIcon"]["y"] = conf.m_serverIcon.m_fY;

		// VehicleHud
		j["vehicleHud"]["isDrawHelpTablet"] = conf.m_vehHud.m_bIsDrawHelpTablet;

		// Samp
		j["samp"]["isWhiteID"] = conf.m_samp.m_bIsWhiteID;
		j["samp"]["isCustomFont"] = conf.m_samp.m_bIsCustomFont;
		j["samp"]["fontFaceName"] = conf.m_samp.m_FontFaceName;
		j["samp"]["isSortingScreenshots"] = conf.m_samp.m_bIsSortingScreenshots;

		return j;
	}

	inline std::wstring getFullPath()
	{
		const int buffSize{ 512 };
		wchar_t buff[buffSize]{};

		GetModuleFileNameW(NULL, buff, static_cast<size_t>(buffSize));

		for (int i = wcslen(buff); i >= 0; i--)
		{
			if (buff[i] == '\\' || buff[i] == '/') break;
			else buff[i] = '\0';
		}

		return std::wstring(buff, wcslen(buff));
	}

	std::wstring fullPathToLogFile{};

	stConfig config{};
};

#endif