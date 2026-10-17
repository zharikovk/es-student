add_library(stdio-text-protocol STATIC
    src/stdio-text-protocol.c
)
target_link_libraries(stdio-text-protocol PRIVATE
    pico_stdlib
    pico_unique_id
)
target_compile_definitions(stdio-text-protocol PRIVATE
    DEVICE_BOARD="${PICO_BOARD}"
)
target_include_directories(stdio-text-protocol PUBLIC
    include
)
