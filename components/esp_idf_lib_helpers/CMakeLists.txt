list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/eil-cmake-utils/cmake)

idf_component_register(
    INCLUDE_DIRS .
    REQUIRES freertos
)

include(eil_ci)
