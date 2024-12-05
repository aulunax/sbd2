cmake_minimum_required(VERSION 3.5.0)
project(Sbd2 VERSION 0.1.0 LANGUAGES C CXX)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/src/CommandLine.cpp
    ${PROJECT_SOURCE_DIR}/src/BlockInputOutput.cpp
    ${PROJECT_SOURCE_DIR}/src/RecordBlockIO.cpp
    ${PROJECT_SOURCE_DIR}/src/Record.cpp
)

add_executable(Sbd2 ${SOURCES})


