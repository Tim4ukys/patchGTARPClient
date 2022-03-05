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
#ifndef PCH_H
#define PCH_H

// fix compilation
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

// STL
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <functional>
#include <array>
#include <regex>
#include <filesystem>
#include <fstream>


// C (Old-Library)
#include <cstring>
#include <cstdint>


// WIN-API
#include <Windows.h>
#include <Psapi.h>
#include <memory.h>


// BOOST
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>


// my library
#include "patch.h" 
#include "client.h"

#endif //PCH_H
