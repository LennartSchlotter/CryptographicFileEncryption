cmake_minimum_required(VERSION 3.28)
project(CFE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set(SANITIZE_FLAGS -fsanitize=address,undefined -fno-sanitize=leak)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    >
    $<$<CONFIG:Debug>:
        -g -O0
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:${SANITIZE_FLAGS}>
    >
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:
        ${SANITIZE_FLAGS}
    >
)

target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

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