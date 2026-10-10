add_subdirectory(basic_builder)
add_subdirectory(basic_text)

add_custom_target(
    samples
    DEPENDS basic_builder basic_text
)