cmake_minimum_required(VERSION 3.8)

project(cini_test)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/test/main.cpp
    ${PROJECT_SOURCE_DIR}/test/test_c.c
    ${PROJECT_SOURCE_DIR}/test/test_cpp.cpp
    ${PROJECT_SOURCE_DIR}/test/test_util.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/test
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wshadow -Wundef)
endif()
