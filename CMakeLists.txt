CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(ConcurrentQueue)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("------------ Options -------------")
message("  CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

message("----------------------------------")

message("-------------- Env ---------------")
message("  CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("  CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message("  PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("  PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message("----------------------------------")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
endif()

if (NOT MSVC)
    ## -Wall -Werror -Wextra -Wno-format -Wno-unused-function
    set(CMAKE_CXX_FLAGS_DEFAULT "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -fPIC")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEFAULT} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEFAULT} -g -pg -D_DEBUG")
endif()

if (WIN32)
    add_compile_options("-D_WIN32_WINNT=0x0601")
    add_compile_options("-D_CRT_SECURE_NO_WARNINGS")
    set(EXTRA_LIBS ${EXTRA_LIBS} ws2_32 mswsock winmm)
endif()

if (UNIX)
    set(EXTRA_LIBS ${EXTRA_LIBS} pthread)
else()
    set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

include_directories(src)
include_directories(src/ConcurrentQueue)

set(SOURCE_FILES 
    src/ConcurrentQueue/common.h
    src/ConcurrentQueue/MinMax.h
    src/ConcurrentQueue/PowerOf2.h
    src/ConcurrentQueue/NativeMutex.h
    src/ConcurrentQueue/QueueWrapper.h
    src/ConcurrentQueue/ValueMessage.h
    src/ConcurrentQueue/Sequence.h
    src/ConcurrentQueue/LockedRingQueue.h
    src/ConcurrentQueue/SingleRingQueue.h
    src/ConcurrentQueue/DisruptorRingQueue.h
    src/ConcurrentQueue/stop_watch.h
    src/ConcurrentQueue/sleep.h
    src/ConcurrentQueue/sleep.c
    src/ConcurrentQueue/main.cpp
    )

add_custom_target(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
)

add_custom_target(release
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Release"
)

add_executable(ConcurrentQueue ${SOURCE_FILES})
target_link_libraries(ConcurrentQueue ${EXTRA_LIBS})
