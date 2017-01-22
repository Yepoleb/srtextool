cmake_minimum_required (VERSION 3.1)
project (srtextool)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif (NOT CMAKE_BUILD_TYPE)

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

set (STATIC_BUILD OFF CACHE BOOL "Enable static linking for release builds")
if (STATIC_BUILD)
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif (STATIC_BUILD)

add_executable (${PROJECT_NAME} ${SOURCES})
target_include_directories (${PROJECT_NAME} PRIVATE external)
