cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# For host builds (Linux target), include the test directory
# For hardware builds, only include the main source
if(CONFIG_IDF_TARGET_LINUX)
    # Host-based testing mode
    set(EXTRA_COMPONENT_DIRS "test")
endif()

project(OuttatimersController)
