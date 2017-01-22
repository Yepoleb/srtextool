# This file is only needed when cross compiling for a different architecture
# You don't need it when building on the target system

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Linux)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER gcc)
SET(CMAKE_CXX_COMPILER g++)

# set 32bit flags
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32" CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -m32" CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_C_FLAGS} -m32" CACHE STRING "" FORCE)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
