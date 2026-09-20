include(FetchContent)

if(UNIX AND NOT APPLE)
    # Native Linux UE4SS source tree.
    #
    # Example:
    # cmake -DUE4SS_SOURCE_DIR=/path/to/ue4ss-linux-palworld ...
    set(UE4SS_SOURCE_DIR "" CACHE PATH
        "Path to the native Linux UE4SS source tree")

    if(NOT UE4SS_SOURCE_DIR)
        message(FATAL_ERROR
            "Native Linux builds require UE4SS_SOURCE_DIR. "
            "Set -DUE4SS_SOURCE_DIR=/path/to/ue4ss-linux-palworld")
    endif()

    if(NOT EXISTS "${UE4SS_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "UE4SS_SOURCE_DIR does not contain a UE4SS CMakeLists.txt: "
            "${UE4SS_SOURCE_DIR}")
    endif()

    # PalSchema and UE4SS must use matching CppUserModBase ABI options.
    set(UE4SS_GUI_ENABLED ON)
    set(UE4SS_INPUT_ENABLED OFF)

    add_subdirectory(
        "${UE4SS_SOURCE_DIR}"
        "${CMAKE_BINARY_DIR}/native-ue4ss"
    )
else()
    # Preserve the existing Windows dependency behavior.
    add_subdirectory("RE-UE4SS")
endif()

add_subdirectory("json")
add_subdirectory("safetyhook")
add_subdirectory("glaze")
add_subdirectory("efsw")
