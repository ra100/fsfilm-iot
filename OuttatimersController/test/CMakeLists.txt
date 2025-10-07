# CMakeLists.txt for host-based tests
# This file configures the test component for running on the Linux host

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define test source files
set(TEST_SOURCES
    "test_main.cpp"
)

# Register the test component
# When building for host (IDF_TARGET=linux), this creates a test executable
idf_component_register(
    SRCS ${TEST_SOURCES}
    INCLUDE_DIRS "." "mocks" "../src"
    REQUIRES
        unity           # Unity test framework
        # Add other required components that have host support
)

# For host builds, we need to add mock includes
if(CONFIG_IDF_TARGET_LINUX)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        HOST_BUILD=1
    )
    
    # Add compile options for host testing
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-missing-field-initializers
        -Wno-unused-variable
        -Wno-unused-function
    )
endif()