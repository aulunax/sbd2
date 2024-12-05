cmake_minimum_required(VERSION 3.5.0)
project(Sbd2 VERSION 0.1.0 LANGUAGES C CXX)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/BlockIO)
include_directories(${PROJECT_SOURCE_DIR}/include/Btree)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/src/CommandLine.cpp
    ${PROJECT_SOURCE_DIR}/src/BlockIO/BlockInputOutput.cpp
    ${PROJECT_SOURCE_DIR}/src/BlockIO/IndexBlockIO.cpp
    ${PROJECT_SOURCE_DIR}/src/BlockIO/RecordBlockIO.cpp
    ${PROJECT_SOURCE_DIR}/src/Btree/BtreeNode.cpp
    ${PROJECT_SOURCE_DIR}/src/Btree/BtreePage.cpp
    ${PROJECT_SOURCE_DIR}/src/Btree/BtreeHandler.cpp
    ${PROJECT_SOURCE_DIR}/src/Record.cpp
)

add_executable(Sbd2 ${SOURCES})


