set(EXE_NAME CFE)
cmake_minimum_required(VERSION 3.28)
project(${EXE_NAME} LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(sodium REQUIRED IMPORTED_TARGET libsodium)
pkg_check_modules(argon2 REQUIRED IMPORTED_TARGET libargon2)
pkg_check_modules(fmt REQUIRED IMPORTED_TARGET fmt)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${EXE_NAME})

set_target_properties(${EXE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set(SANITIZE_FLAGS -fsanitize=address,undefined -fno-sanitize=leak)

target_compile_options(${EXE_NAME} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    >
    $<$<CONFIG:Debug>:
        -g -O0
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:${SANITIZE_FLAGS}>
    >
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)

target_link_options(${EXE_NAME} PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:
        ${SANITIZE_FLAGS}
    >
)

target_sources(${EXE_NAME} PRIVATE
    src/main.cpp
    src/handler.cpp
    src/logger.cpp
    src/helper/password.cpp
    src/helper/secure_allocator.cpp
    src/helper/cryptography.cpp
    src/modules/encrypt.cpp
    src/modules/decrypt.cpp
)

target_include_directories(${EXE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${EXE_NAME} PRIVATE
    PkgConfig::sodium
    PkgConfig::argon2
    PkgConfig::fmt
)

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