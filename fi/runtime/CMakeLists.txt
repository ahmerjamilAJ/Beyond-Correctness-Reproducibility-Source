cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

project(llfi-rt)

add_library(llfi-rt SHARED
    CommonFaultInjectors.cpp
    FaultInjectionLib.c
    FaultInjectorManager.cpp
    InstTraceLib.c
    ProfilingLib.cpp
    Utils.c
)

TARGET_LINK_LIBRARIES(llfi-rt pthread)
