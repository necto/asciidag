add_library(asciidag)

target_sources(asciidag
  PRIVATE
    asciidag.cpp
  PUBLIC
    asciidag.h
  )

target_include_directories(asciidag
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
  )

