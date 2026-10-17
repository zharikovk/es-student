add_library(profiling STATIC
    src/profiling.c
)
target_link_libraries(profiling PRIVATE
  systime
  pico_stdlib
)

target_include_directories(profiling PUBLIC
    include
)
