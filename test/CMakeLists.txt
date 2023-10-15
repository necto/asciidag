include(FetchContent)
FetchContent_Declare(gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
  GIT_SHALLOW ON
)
# configure build of googletest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)

add_executable(
    unit_tests
    parseTest.cpp
    renderTest.cpp
    drawEdgeTest.cpp
    crossingMinimizationTest.cpp
    crossingEdgesTest.cpp
    testUtils.cpp
    parseRenderTest.cpp
    dotTest.cpp
    )

target_link_libraries(unit_tests
  PRIVATE
    asciidag
    gtest_main
  )

# automatic discovery of unit tests
include(GoogleTest)
gtest_discover_tests(unit_tests
  PROPERTIES
    LABELS "unit"
  DISCOVERY_TIMEOUT  # how long to wait (in seconds) before crashing
    240
  )
