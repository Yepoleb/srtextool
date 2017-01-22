cmake_minimum_required (VERSION 3.1)
project (srtextool)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

set (SOURCES
    src/cli/cmd_add.cpp
    src/cli/cmd_check.cpp
    src/cli/cmd_delete.cpp
    src/cli/cmd_extract.cpp
    src/cli/cmd_list.cpp
    src/cli/main.cpp
    src/cli/shared.cpp
    src/ddsfile.cpp
    src/headerfile.cpp
    src/byteio.cpp
)

if (MINGW)
    message (STATUS "MinGW detected")
    set (CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif (MINGW)

add_executable (${PROJECT_NAME} ${SOURCES})
target_include_directories (${PROJECT_NAME} PRIVATE external)
