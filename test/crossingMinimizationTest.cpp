#include "asciidag.h"

#include <gtest/gtest.h>

using namespace asciidag;

std::string parseAndRender(std::string_view str) {
  ParseError parseErr;
  auto dag = parseDAG(str, parseErr);
  EXPECT_EQ(parseErr.code, ParseError::Code::None);
  if (parseErr.code != ParseError::Code::None) {
    // Print error message by violating an assertion
    EXPECT_EQ(parseErr.message, "");
    // Print error location by violating an assertion
    EXPECT_EQ(parseErr.pos, (Position{0, 0}));
  }
  EXPECT_TRUE(dag.has_value());
  if (dag) {
    RenderError renderErr;
    auto result = renderDAG(*dag, renderErr);
    EXPECT_EQ(renderErr.code, RenderError::Code::None);
    if (renderErr.code != RenderError::Code::None) {
      // Print error message by violating an assertion
      EXPECT_EQ(renderErr.message, "");
      // Print error location by violating an assertion
      EXPECT_EQ(renderErr.nodeId, 0U);
    }
    EXPECT_TRUE(result.has_value());
    if (result) {
      return "\n" + *result;
    }
  }
  return "";
}

TEST(crossingMinimizationTest, preserveSingleEdge) {
  EXPECT_EQ(parseAndRender(R"(
0
|
1
)"),
R"(
0
|
|
|
1
)");
}

TEST(crossingMinimizationTest, preserveTwoEdgesDiverge) {
  EXPECT_EQ(parseAndRender(R"(
0
|\
1 2
)"),
R"(
0
|\
| \
| |
| |
1 2
)");
}

TEST(crossingMinimizationTest, preserveTwoParallel) {
  EXPECT_EQ(parseAndRender(R"(
1 2
| |
3 4
)"),
R"(
1 2
| |
| |
| |
3 4
)");
}

TEST(crossingMinimizationTest, untangleTwoCrossed) {
  EXPECT_EQ(parseAndRender(R"(
1   2
 \ /
  X
 / \
3   4
)"),
R"(
1 2
| |
| |
| |
4 3
)");
}

TEST(crossingMinimizationTest, untangleDoubleCrossing) {
  EXPECT_EQ(parseAndRender(R"(
1   2
 \ /
  X
 / \
3   4
 \ /
  X
 / \
5   6
)"),
R"(
1 2
| |
| |
| |
4 3
| |
| |
| |
5 6
)");
}

TEST(crossingMinimizationTest, untangleCrossThenParallel) {
  EXPECT_EQ(parseAndRender(R"(
1   2
 \ /
  X
 / \
3   4
|   |
5   6
)"),
R"(
1 2
| |
| |
| |
4 3
| |
| |
| |
6 5
)");
}

TEST(crossingMinimizationTest, untangleParallelThenCross) {
  EXPECT_EQ(parseAndRender(R"(
1   2
|   |
3   4
 \ /
  X
 / \
5   6
)"),
R"(
1 2
| |
| |
| |
3 4
| |
| |
| |
6 5
)");
}


TEST(crossingMinimizationTest, preserveHammock) {
  EXPECT_EQ(parseAndRender(R"(
  0
 / \
1   2
 \ /
  3
)"),
R"(
0
|\
| \
| |
| |
1 2
| |
| |
| /
|/
3
)");
}
