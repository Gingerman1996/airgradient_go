idf_component_register(
    SRCS
        "airgradient-g.cpp"
        "sensor.cpp"
        "ui_display.cpp"
    INCLUDE_DIRS
        "."
    REQUIRES
        "stcc4"
        "esp_sgp4x"
        "sps30"
        "sensirion_gas_index_algorithm"
        "esp_epaper"
        "esp_lcd"
        "lvgl"
)

# Optimize for size (OTA partition is 1 MB)
target_compile_options(${COMPONENT_LIB} PRIVATE "-Os")
# Linker GC unused sections to reduce binary size
target_link_options(${COMPONENT_LIB} PRIVATE "-Wl,--gc-sections")
