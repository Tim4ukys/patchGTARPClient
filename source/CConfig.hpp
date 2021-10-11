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
		struct stServerIcon {
			bool m_state;
			float m_x, m_y;
		} m_serverIcon;
	};

	inline CConfig(std::wstring fileName)
	{
		fullPathToLogFile = getFullPath() + fileName;
		std::ifstream inFileStream{ fullPathToLogFile.c_str() };
		
		if (!inFileStream.fail()) {
			json j;
			inFileStream >> j;
			inFileStream.close();
			config = JSON_TO_CONFIG(j);
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

	inline stConfig JSON_TO_CONFIG(json& j)
	{
		return {
			{
				j["serverIcon"]["state"].get<bool>(),
				j["serverIcon"]["x"].get<float>(),
				j["serverIcon"]["y"].get<float>(),
			}
		};
	}
	inline json CONFIG_TO_JSON(stConfig& conf)
	{
		json j;
		
		// stServerIcon
		j["serverIcon"]["state"] = conf.m_serverIcon.m_state;
		j["serverIcon"]["x"] = conf.m_serverIcon.m_x;
		j["serverIcon"]["y"] = conf.m_serverIcon.m_y;
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

	stConfig config{
		{
			false,
			656,
			28
		}
	};
};

#endif