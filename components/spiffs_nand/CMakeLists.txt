idf_component_register(
    SRCS
        "src/spiffs_cache.c"
        "src/spiffs_check.c"
        "src/spiffs_gc.c"
        "src/spiffs_hydrogen.c"
        "src/spiffs_nucleus.c"
    INCLUDE_DIRS
        "include"
    PRIV_INCLUDE_DIRS
        "src"
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set_source_files_properties(src/spiffs_nucleus.c PROPERTIES COMPILE_FLAGS -Wno-stringop-truncation)
endif()

set_source_files_properties(
    src/spiffs_cache.c
    src/spiffs_check.c
    src/spiffs_gc.c
    src/spiffs_hydrogen.c
    src/spiffs_nucleus.c
    PROPERTIES COMPILE_FLAGS -Wno-format
)
