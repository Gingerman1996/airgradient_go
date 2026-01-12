idf_component_register(
    SRCS "src/cap1203.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c freertos log
)
