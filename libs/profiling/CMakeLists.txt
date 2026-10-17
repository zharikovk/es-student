add_library(profiling STATIC
    src/profiling.c
)
target_link_libraries(profiling
  PUBLIC systime
  PRIVATE  pico_stdlib
)

target_include_directories(profiling PUBLIC
    include
)
