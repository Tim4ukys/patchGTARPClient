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

SAMP*     g_pSAMP = nullptr;
D3D9Hook* g_pD3D9Hook = nullptr;
Log       g_Log("!000patchGTARPClientByTim4ukys.log");
Config    g_Config("!000patchGTARPClientByTim4ukys.json");
HMODULE   g_DLLModule;

snippets::DynamicLibrary g_gtarpclientBase{"gtarp_clientside.asi"};
snippets::DynamicLibrary g_sampBase{"samp.dll"};
snippets::DynamicLibrary g_D3DX9_25{"d3dx9_25.dll"};
snippets::DynamicLibrary g_SF{"SAMPFUNCS.asi"};
