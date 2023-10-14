#include "testUtils.h"

#include <gtest/gtest.h>

using namespace asciidag;
using namespace asciidag::tests;

TEST(crossingEdgesTest, simpleIrreducibleCrossing) {
  EXPECT_EQ(parseAndRender(R"(
0   1
|\ /|
| X |
|/ \|
2   3
)"),
R"(
0  1
|\ |\
| \| \
| || |
| |/ |
. X  .
| |\ |
| || |
| /| /
|/ |/
2  3
)");
}
