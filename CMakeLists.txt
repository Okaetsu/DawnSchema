cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(deps)

set(TARGET DawnSchema)
project(${TARGET})

file(GLOB SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dllmain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Generator/JsonSchema/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Loader/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SDK/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SDK/Classes/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SDK/Classes/Property/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SDK/Helper/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SDK/Structs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SDK/Structs/Reflected/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SDK/Structs/Custom/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Tools/EnumSchemaDefinitionGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Utility/Config.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Utility/JsonHelpers.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Utility/EnumHelpers.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Misc/FileWatchWrapper.cpp"
)

add_library(${TARGET} SHARED ${SRC_FILES} version.rc)
target_include_directories(${TARGET} PRIVATE 
    include/
)
target_link_libraries(${TARGET} PRIVATE Zydis Zycore safetyhook nlohmann_json::nlohmann_json glaze::glaze UE4SS efsw)

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET}> "D:/SteamLibrary/steamapps/common/The Blood of Dawnwalker/Dawnwalker/Binaries/Win64/ue4ss/Mods/${TARGET}/dlls/"
)