add_library(systime STATIC
    src/systime.c
)
target_link_libraries(systime PRIVATE
    pico_stdlib
)
target_compile_definitions(systime PRIVATE
    DEVICE_BOARD="${PICO_BOARD}"
)
target_include_directories(systime PUBLIC
    include
)
