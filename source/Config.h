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
#pragma once
class Config {
public:

	Config(std::string fileName);
    ~Config();

	void saveFile();
    nlohmann::json& getJSON();
    nlohmann::json& operator[](const char* key);

private:
	using json = nlohmann::json;

	json        j;
    std::string m_fileName;

	bool isAllKeysCorrrect();
    void loadDefaultConfig();
};
