/*******************************************************
*                       LICENSE: MIT.                  *
*                 LAST UPDATE: 19.09.2021              *
*                                                      *
*                   DEVELOPER: TIM4UKYS                *
*  Contact: vk.com/tim4ukys or tim4ukys.dev@yandex.ru  *
*******************************************************/
#ifndef _CLOG_HPP_
#define _CLOG_HPP_

#include <fstream>
#include <Windows.h>

#define MODULE_NAME ("patchGTARPClientByTim4ukys")
#define CONTACT ("My contact: vk.com/tim4ukys or tim4ukys.dev@yandex.ru")

class CLog {
public: 
	
	inline CLog(std::wstring fileName)
	{
		fullPathToLogFile = getFullPath() + fileName;
		std::ofstream outFileStream{ fullPathToLogFile.c_str() };

		if (outFileStream.fail()) {
			MessageBeep(MB_ICONERROR);
			PostQuitMessage(-1);
		}
		else {
			outFileStream << MODULE_NAME << " started!" << std::endl 
				<< CONTACT << std::endl << std::endl;

			outFileStream.close();
		}
	}
	inline ~CLog()
	{
		Log(" ");
		Log("Destory...");
	}

	inline void Log(const char* fmt, ...)
	{
		std::ofstream outFileStream{ fullPathToLogFile.c_str(), std::ios_base::app };
		if (!outFileStream.fail())
		{
			char buff[512]{};
			
			va_list arrg{};
			va_start(arrg, fmt);
			vsprintf_s(buff, fmt, arrg);
			va_end(arrg);

			// [00:00:00.000] // 14 символов + 1 для нулевого символа
			char timeBuff[15]{};
			SYSTEMTIME time{};
			GetLocalTime(&time);
			sprintf(timeBuff, "[%02d:%02d:%02d.%03d]", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

			outFileStream << timeBuff << ": " << buff << std::endl;

			outFileStream.close();
		}
	}


private:

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

};

#endif