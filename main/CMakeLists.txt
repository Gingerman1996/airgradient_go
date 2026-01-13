idf_component_register(
    SRCS
        airgradient-go.cpp
        gps.cpp
        i2c_scanner.cpp
        log_storage.cpp
        sensor.cpp
        ui_display.cpp
    INCLUDE_DIRS
        "."
    REQUIRES
        "stcc4"
        "esp_sgp4x"
        "sps30"
        "sensirion_gas_index_algorithm"
        "cap1203"
        "bq25629"
        "libnmea"
        "esp_epaper"
        "esp_lcd"
        "lvgl"
        "espressif__spi_nand_flash"
        "fatfs"
        "dps368"
)

# Optimize for size (OTA partition is 1 MB)
target_compile_options(${COMPONENT_LIB} PRIVATE "-Os")
# Linker GC unused sections to reduce binary size
target_link_options(${COMPONENT_LIB} PRIVATE "-Wl,--gc-sections")
