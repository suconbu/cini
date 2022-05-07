cmake_minimum_required(VERSION 3.8)

project(cini_test)
enable_testing()

set(SOURCES
    ${PROJECT_SOURCE_DIR}/test/test.c
    ${PROJECT_SOURCE_DIR}/test/test_c.c
    ${PROJECT_SOURCE_DIR}/test/test_cpp.cpp
    ${PROJECT_SOURCE_DIR}/test/test_util.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_test(
    NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
)

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
