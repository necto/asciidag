add_library(asciidag)

target_sources(asciidag
  PRIVATE
    asciidag.cpp
    asciidagImpl.h
  PUBLIC
    asciidag.h
  )

target_compile_options(asciidag PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_include_directories(asciidag
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
  )

