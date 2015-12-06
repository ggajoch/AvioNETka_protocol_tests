cmake_minimum_required(VERSION 3.3)
project(protocol)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB FreeRTOSSrc
FreeRTOS/*.c
FreeRTOS/include/*.h
FreeRTOS/portable/*.c
FreeRTOS/portable/*.h
FreeRTOS/FreeRTOS-Plus-Trace/*.c
FreeRTOS/FreeRTOS-Plus-Trace/Include/*.h
)

find_library(LIBWMM winmm)

#set(SOURCE_FILES main.cpp PhysicalLayer.h useful.h ${FreeRTOSSrc} subscriber.h ApplicationLayer.h rudder.h DataDescriptor.h protocol/PhysicalLayer.h)
set(SOURCE_FILES ${FreeRTOSSrc} main.cpp protocol/PhysicalLayer.h protocol/useful.h protocol/NetworkReceiver.h protocol/DataStructs.h protocol/NetworkSender.h protocol/NetworkLayer.h protocol/ApplicationLayer.h)

include_directories(FreeRTOS/include FreeRTOS/portable FreeRTOS/FreeRTOS-Plus-Trace/Include . )
add_executable(protocol ${SOURCE_FILES})

target_link_libraries(protocol ${LIBWMM})