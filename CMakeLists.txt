cmake_minimum_required(VERSION 3.28)

# Compiler
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-18")
set(CMAKE_C_COMPILER "/usr/bin/clang-18")

set(EXE_NAME CFE)
project(${EXE_NAME} LANGUAGES CXX)

# C++ standards
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# libc++
add_compile_options(-stdlib=libc++)
add_link_options(-stdlib=libc++)

# External libraries
find_package(PkgConfig REQUIRED)
if(DEFINED ENV{PKG_CONFIG_PATH})
    set(ENV{PKG_CONFIG_PATH} "$ENV{HOME}/libsodium/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
else()
    set(ENV{PKG_CONFIG_PATH} "$ENV{HOME}/libsodium/lib/pkgconfig")
endif()
pkg_check_modules(sodium REQUIRED IMPORTED_TARGET libsodium)
pkg_check_modules(argon2 REQUIRED IMPORTED_TARGET libargon2)
pkg_check_modules(fmt REQUIRED IMPORTED_TARGET fmt)

# Google Test install
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SANITIZE_FLAGS -fsanitize=address,undefined -fno-sanitize=leak)

function(cfe_target_options TARGET)
    target_compile_options(${TARGET} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
            -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wswitch-enum
        >
        $<$<CONFIG:Debug>:
            -g -O0
            $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:${SANITIZE_FLAGS}>
        >
        $<$<CONFIG:Release>:-O2 -DNDEBUG>
    )
    target_link_options(${TARGET} PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:
            ${SANITIZE_FLAGS}
        >
    )
endfunction()

add_library(CFE_lib STATIC
    src/handler.cpp
    src/logger.cpp
    src/helper/password.cpp
    src/helper/cryptography.cpp
    src/helper/secure_memory.cpp
    src/modules/encrypt.cpp
    src/modules/decrypt.cpp
    src/modules/keygen.cpp
)

cfe_target_options(CFE_lib)
target_include_directories(CFE_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(CFE_lib PUBLIC
    PkgConfig::sodium
    PkgConfig::argon2
    PkgConfig::fmt
)

# Executable
add_executable(${EXE_NAME} src/main.cpp)
cfe_target_options(${EXE_NAME})
set_target_properties(${EXE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(${EXE_NAME} PRIVATE CFE_lib)

# Tests
enable_testing()
add_subdirectory(tests)

################# Libraries ##########################
#   Run: sudo apt install libfmt-dev
#   find_package(fmt REQUIRED)
#   target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
#
#   Run: vcpkg add port sqlite3
#   find_package(SQLite3 REQUIRED)
#   target_link_libraries(${PROJECT_NAME} PRIVATE SQLite::SQLite3)
#
#   vcpkg search <name>
#
#   Remember to update the CI dependency install!