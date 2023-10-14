#include "testUtils.h"

#include <gtest/gtest.h>

using namespace asciidag;
using namespace asciidag::tests;

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

TEST(crossingMinimizationTest, untangleCrossByTwo) {
  EXPECT_EQ(parseAndRender(R"(
  0 1 2
   \| |
    X |
    |\|
    | X
    | |\
    3 4 5
)"),
R"(
0 1 2
| | |
| | |
| | |
5 3 4
)");
}

TEST(crossingMinimizationTest, untangleCrossTwoByTwo) {
  EXPECT_EQ(parseAndRender(R"(
  0 1 2 3
   \ \| |
    \ X |
     \|\|
      X X
      |\|\
      | X \
      | |\ \
      4 5 6 7
)"),
R"(
0 1 2 3
| | | |
| | | |
| | | |
6 7 4 5
)");
}

TEST(crossingMinimizationTest, untangleDiffDirections) {
  EXPECT_EQ(parseAndRender(R"(
  0 1 2
   \|/
    X
   /|\
  3 4 5
)"),
R"(
0 1 2
| | |
| | |
| | |
5 4 3
)");
}

TEST(crossingMinimizationTest, untangleTwoPredsCrossedOne) {
  // This untangling requries moving the nodes in the upper layer
  EXPECT_EQ(parseAndRender(R"(
    0 1 2
    |/ /
    X /
   /|/
  3 4
)"),
R"(
1 0 2
| | |
| | |
| | /
| |/
3 4
)");
}

TEST(crossingMinimizationTest, untangleTwoPredsCrossedTwo) {
  // This untangling requries moving the nodes in the upper layer
  EXPECT_EQ(parseAndRender(R"(
    0 1 2
    |/ /
    X /
   /|/
  / X
  |/|
  3 4
)"),
R"(
0 1 2
| | |
| | |
| | /
| |/
4 3
)");
}

TEST(crossingMinimizationTest, untangleTwoSuccsCrossedOne) {
  EXPECT_EQ(parseAndRender(R"(
    0 1
    |/|
    X |
   /| |
  3 4 5
)"),
R"(
0 1
| |\
| | \
| | |
| | |
4 3 5
)");
}

TEST(crossingMinimizationTest, untangleTwoSuccsCrossedTwo) {
  EXPECT_EQ(parseAndRender(R"(
    0 1
    |/|
    X /
   /|/
  / X
 / /|
3 4 5
)"),
R"(
0 1
| |\
| | \
| | |
| | |
5 3 4
)");
}

TEST(crossingMinimizationTest, untangleHammockWithIntruder) {
  EXPECT_EQ(parseAndRender(R"(
1   2
 \ / \
  X   \
 / \   \
3   4   5
 \ /   /
  X   /
 / \ /
6   7
)"),
R"(
1 2
| |\
| | \
| | |
| | |
4 3 5
| | |
| | |
| | /
| |/
6 7
)");
}

// TODO:
// TEST(crossingMinimizationTest, untangleCentripetalSymmetricalCrossing) {
//   EXPECT_EQ(parseAndRender(R"(
//  0   1   2
//   \ / \ /
//    X   X
//   / \ / \
//  3   4   5
// )"),
// R"(
// )");
// }

// TODO:
// TEST(crossingMinimizationTest, untangleStrayLongerEdgeFail) {
//   EXPECT_EQ(parseAndRender(R"(
// 0     1
// |\   /|
// | \ / |
// |  X  |
// | / \ |
// |/  | |
// 2   3 4
// |\  | |
// | \ / |
// |  X  |
// | / \ |
// |/   \|
// 5     6
// )"),
// R"(
// )");
// }
