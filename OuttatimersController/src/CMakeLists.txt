# CMakeLists.txt for Outtatimers Controller ESP-IDF Component

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
set(SOURCES
    "main.cpp"
    "led_driver.cpp"
    "effect_manager.cpp"
    "button_handler.cpp"
)

# Register the component with required dependencies
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES
        driver          # GPIO driver
        esp_timer       # High resolution timer
        freertos        # FreeRTOS
        nvs_flash       # Non-volatile storage
)

# Component configuration
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-missing-field-initializers
    -Wno-unused-variable
    -Wno-unused-function
)
