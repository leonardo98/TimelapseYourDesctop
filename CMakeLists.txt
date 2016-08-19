cmake_minimum_required(VERSION 3.4)

project(TimelapseYourDesktop)

file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SOURCES "*.cpp")
set(ALL_FILES ${HEADERS} ${SOURCES})

add_executable(TimalapseYourDesktop ${ALL_FILES})


