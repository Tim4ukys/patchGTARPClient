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

D3D9Hook *g_pD3D9Hook = nullptr;
Log g_Log("!000patchGTARPClientByTim4ukys.log");
Config g_Config("!000patchGTARPClientByTim4ukys.json");

snippets::DynamicLibrary g_gtarpclientBase{"gtarp_clientside.asi"};
snippets::DynamicLibrary g_sampBase{ "samp.dll" };