cmake_minimum_required(VERSION 3.4)

project(TimelapseYourDesctop)

file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SOURCES "*.cpp")
set(ALL_FILES ${HEADERS} ${SOURCES})

add_executable(TimalapseYourDesctop ${ALL_FILES} ${RES_FILES})

