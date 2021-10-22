cmake_minimum_required(VERSION 3.8)

project(cini_test)

set(TOP_DIR ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/src/cini.cpp
	${PROJECT_SOURCE_DIR}/test/main.cpp
	${PROJECT_SOURCE_DIR}/test/test_c.c
	${PROJECT_SOURCE_DIR}/test/test_cpp.cpp
	${PROJECT_SOURCE_DIR}/test/test_util.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/test
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/test"
	VS_STARTUP_PROJECT "cini_test"
)
