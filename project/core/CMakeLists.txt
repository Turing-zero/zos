cmake_minimum_required(VERSION 3.14)

project(core LANGUAGES CXX)

option(ZOS_DEBUG "zos debug output." ON)
option(ZOS_USE_CUSTOM_SEMAPHORE "use custom semaphore." ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

if(ZOS_DEBUG)
    add_definitions(-DZOS_DEBUG)
endif()

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${fmt_INCLUDE_DIRS}
)
if(ZOS_USE_CUSTOM_SEMAPHORE)
    add_definitions(-DZOS_USE_CUSTOM_SEMAPHORE)
    set(ZOS_HEADERS
        ${ZOS_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/zos/*.h
        )
endif()
file(GLOB ZOS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/zos/*.h
    )
set(HEADER_FILES
    ${ZOS_HEADERS}
    )
set(CPP_FILES
    )
set(SOURCE_FILES
    ${CPP_FILES}
    ${HEADER_FILES}
    )
set(LIBS
    Threads::Threads
    fmt::fmt
    )
file(GLOB TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp
    )
foreach(test_file ${TEST_FILES})
    file(RELATIVE_PATH test_file_name ${CMAKE_CURRENT_SOURCE_DIR} ${test_file})
    string( REPLACE ".cpp" "" test_name ${test_file_name} )
    message("markdebug : " ${test_file} " -- " ${test_name})

    add_executable(${test_name}
        ${test_file_name}
        ${SOURCE_FILES}
    )
    target_link_libraries(${test_name} ${LIBS})
endforeach()
