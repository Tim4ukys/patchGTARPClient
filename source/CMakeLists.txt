cmake_minimum_required(VERSION 3.16)
project(patchGTARPClient)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
include_directories(nlohmann)

# DIRECTX
include_directories("$ENV{DXSDK_DIR}/Include")
link_directories("$ENV{DXSDK_DIR}/Lib/x86")
# PLUGIN SDK
ADD_DEFINITIONS(-DGTASA)
ADD_DEFINITIONS(-DGTAGAME_NAME="San Andreas")
ADD_DEFINITIONS(-DGTAGAME_ABBR="SA")
ADD_DEFINITIONS(-DGTAGAME_ABBRLOW="sa")
ADD_DEFINITIONS(-DGTAGAME_PROTAGONISTNAME="CJ")
ADD_DEFINITIONS(-DGTAGAME_CITYNAME="San Andreas")
ADD_DEFINITIONS(-D_LA_SUPPORT)
ADD_DEFINITIONS(-D_DX9_SDK_INSTALLED)
ADD_DEFINITIONS(-DPLUGIN_SGV_10US)

include_directories("$ENV{PLUGIN_SDK_DIR}/plugin_sa")
include_directories("$ENV{PLUGIN_SDK_DIR}/plugin_sa/game_sa")
include_directories("$ENV{PLUGIN_SDK_DIR}/shared")
include_directories("$ENV{PLUGIN_SDK_DIR}/shared/game")
link_directories("$ENV{PLUGIN_SDK_DIR}/output/lib")
# POLYHOOK 2.0
include_directories("G:/GitHub/PolyHook_2_0/include")
link_directories("G:/GitHub/PolyHook_2_0/lib/Release")
# BOOST
include_directories("G:/cpp/boost_1_77_0")
link_directories("G:/cpp/boost_1_77_0/libs")
link_directories("G:/cpp/boost_1_77_0/boost")
link_directories("G:/cpp/boost_1_77_0/stage/lib")
# OpenSSL
include_directories("G:/cpp/OpenSSL/out/Lib/x32/Release/include")
link_directories("G:/cpp/OpenSSL/out/Lib/x32/Release/lib")

add_library(${PROJECT_NAME} SHARED
        nlohmann/json.hpp
        CConfig.hpp
        Client.hpp
        CLog.hpp
        CSAMP.cpp
        CSAMP.h
        dllmain.cpp
        dllmain.h
        patch.hpp
        snippets.hpp offsets.hpp)

#######################################################

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "000patchGTARPClientByTim4ukys")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".ASI")

IF(EXISTS "000patchGTARPClientByTim4ukys.ASI")
    IF (EXISTS "!000patchGTARPClientByTim4ukys.ASI")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND del !000patchGTARPClientByTim4ukys.ASI)
    ENDIF()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ren 000patchGTARPClientByTim4ukys.ASI !000patchGTARPClientByTim4ukys.ASI)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND copy \"!000patchGTARPClientByTim4ukys.ASI\" \"D:/Games/GTA RP\" /y)
ENDIF()