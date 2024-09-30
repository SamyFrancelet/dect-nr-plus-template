cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

target_compile_options(app PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function)

project(app LANGUAGES C)

target_sources(app PRIVATE
	src/main.c
	src/dect_phy/dect_phy.c
)

target_include_directories(app PRIVATE
	src
)