add_executable(dummy dummy.c)

target_sources(dummy PRIVATE dummy.c)

target_link_libraries(dummy PRIVATE pico_stdlib)
pico_add_extra_outputs(dummy)

# add url via pico_set_program_url
example_auto_set_url(dummy)