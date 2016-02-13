cmake_minimum_required(VERSION 3.3)
project(protocol)


file(DOWNLOAD https://github.com/google/googletest/archive/release-1.7.0.tar.gz ${CMAKE_CURRENT_SOURCE_DIR}/gtest.tar.gz)

execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xvf ${CMAKE_CURRENT_SOURCE_DIR}/gtest.tar.gz
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -O0")

file(GLOB FreeRTOSSrc
FreeRTOS/*.c
FreeRTOS/include/*.h
FreeRTOS/portable/*.c
FreeRTOS/portable/*.h
FreeRTOS/FreeRTOS-Plus-Trace/*.c
FreeRTOS/FreeRTOS-Plus-Trace/Include/*.h
)

find_library(LIBWMM winmm)
find_library(LIBWINSOCK ws2_32)

set(SOURCE_FILES ${FreeRTOSSrc} ++FreeRTOS.h main.cpp protocol/DataStructs.h protocol/NetworkLayer.h protocol/StackInterfaces.h protocol/useful.h protocol/Commands.h protocol/SystemCommands.h protocol/stackError.h)

include_directories(FreeRTOS/include FreeRTOS/portable FreeRTOS/FreeRTOS-Plus-Trace/Include .)
add_executable(protocol ${SOURCE_FILES})

target_link_libraries(protocol ${LIBWMM} ${LIBWINSOCK})

set(TESTS tests/test1.cpp)

include_directories(googletest-release-1.7.0 googletest-release-1.7.0/include)
add_executable(protocolTests googletest-release-1.7.0/src/gtest-all.cc googletest-release-1.7.0/src/gtest_main.cc ${TESTS})
target_link_libraries(protocolTests)


