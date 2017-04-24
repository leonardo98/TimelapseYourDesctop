cmake_minimum_required(VERSION 3.4)

project(TimelapseYourDesktop)

file(GLOB_RECURSE HEADERS "source/*.h")
file(GLOB_RECURSE SOURCES "source/*.cpp")
set(ALL_FILES ${HEADERS} ${SOURCES})

add_executable(TimalapseYourDesktop ${ALL_FILES})


