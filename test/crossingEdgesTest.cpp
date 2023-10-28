#include "asciidagImpl.h"
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
| X  |
| |\ |
| || |
| /| /
|/ |/
2  3
)");
}

TEST(crossingEdgesTest, tripleCrossings) {
  EXPECT_EQ(parseAndRender(R"(
   0   1
  /|\ /|\
 / | X | \
 | |/ \| |
 | X   X |
 |/|   |\|
 2 \   / 4
    \ /
     3
)"),
R"(
 0    1
/|\  /|\
|| \ || \
|\  \|\  \
| \ || \ |
| | |/ | |
| | X  | |
| | |\ | |
| | || | /
| | /| //
| |/ |/ |
| X  X  |
| |\ |\ |
| || || |
| /| /| /
|/ |/ |/
2  4  3
)");
}

TEST(crossingEdgesTest, crossOnOneOfTwoLayers) {
  EXPECT_EQ(parseAndRender(R"(
   0   1
  /|\ /|
 / | X |
 | |/ \|
 \ 4   2
  \   /
   \ /
    3
)"),
R"(
 0  1
/|\ |\
|| \\ \
|\  \\ \
| \ || |
| | |/ |
| | X  |
| | |\ |
| | || |
| | /| /
| |/ |/
| 2  4
| |
| |
| /
|/
3
)");
}

TEST(crossingEdgesTest, trickyWaypointLocations) {
  EXPECT_EQ(parseAndRender(R"(
   0   1
  /|\ /|\
 / | X ||
 | |/ \||
 \ 4   2|
  \   / /
   \ / /
    \|/
     3
)"),
R"(
 0    1
/|\  /|\
|| \ || \
|\  \|\  \
| \ || \ |
| | |/ | |
| | X  | |
| | |\ | |
| | || | /
| | /| //
| |/ |/ |
| 4  2  |
|    |  |
|    |  |
|    |  /
|    / /
|   / /
|  / /
| / /
|/ /
\|/
 3
)");
}

TEST(crossingEdgesTest, crossMustSwapSuccessors) {
  EXPECT_EQ(parseAndRender(R"(
      0
     / \
    /   \
   /    |
  1   2 |
  |\ /| |
  | X | /
  |/ \|/
  3   4
)"),
R"(
0  2
|\ |\
| \\ \
|  \\ \
|  | \ \
|  | | |
|  | | |
1  | | |
|\ | | |
| \\ | |
| | \| |
| | |/ |
| | X  |
| | |\ |
| | || |
| | /| |
| |/ | |
| X  | |
| |\ | |
| || | |
| || / /
| /|/ /
|/ \|/
3   4
)");
}

TEST(crossingEdgesTest, sixNodes) {
  EXPECT_EQ(parseAndRender(R"(
      0
     /|\
    / \ \
   /   \ \
  / 1   2 \
 / /|\ /| |
 |/ | X | /
 3  |/ \|/
    4   5
)"),
R"(
 0    1
/|\  /|\
|| \ \\ \
||  \ \\ \
|\   \| \ \
| \  || | |
| |  |/ | |
| 2  3  | |
| |\    | |
| | \   | /
| | |   //
| | |  //
| | | //
| | |/ |
| | X  |
| | |\ |
| | || |
| / // /
|/ // /
\|/ |/
 5  4
)");
}

TEST(crossingDiscoveryTest, singleEdge) {
  auto str = R"(
0
|
1
)";
  auto [dag, layers] = parseWithLayers(str);
  ASSERT_EQ(layers.size(), 2U);
  EXPECT_EQ(countCrossings(dag, layers[0], layers[1]), 0U);
  auto crossings = findNonConflictingCrossings(dag, layers[0], layers[1]);
  EXPECT_EQ(crossings.size(), 0U);
}

TEST(crossingDiscoveryTest, singleCrossing) {
  auto str = R"(
0   1
 \ /
  X
 / \
2   3
)";
  auto [dag, layers] = parseWithLayers(str);
  ASSERT_EQ(layers.size(), 2U);
  EXPECT_EQ(countCrossings(dag, layers[0], layers[1]), 1U);
  auto crossings = findNonConflictingCrossings(dag, layers[0], layers[1]);
  ASSERT_EQ(crossings.size(), 1U);
  ASSERT_EQ(crossings[0].fromLeft, 0U);
  ASSERT_EQ(crossings[0].fromRight, 1U);
  ASSERT_EQ(crossings[0].toLeft, 2U);
  ASSERT_EQ(crossings[0].toRight, 3U);
}

TEST(crossingDiscoveryTest, twoIndependentEdges) {
  auto str = R"(
0  1
|  |
2  3
)";
  auto [dag, layers] = parseWithLayers(str);
  ASSERT_EQ(layers.size(), 2U);
  EXPECT_EQ(countCrossings(dag, layers[0], layers[1]), 0U);
  auto crossings = findNonConflictingCrossings(dag, layers[0], layers[1]);
  EXPECT_EQ(crossings.size(), 0U);
}

TEST(crossingDiscoveryTest, crossingAndIndependentEdge) {
  auto str = R"(
0   1 2
 \ /  |
  X   |
 / \  |
3   4 5
)";
  auto [dag, layers] = parseWithLayers(str);
  ASSERT_EQ(layers.size(), 2U);
  EXPECT_EQ(countCrossings(dag, layers[0], layers[1]), 1U);
  auto crossings = findNonConflictingCrossings(dag, layers[0], layers[1]);
  ASSERT_EQ(crossings.size(), 1U);
  ASSERT_EQ(crossings[0].fromLeft, 0U);
  ASSERT_EQ(crossings[0].fromRight, 1U);
  ASSERT_EQ(crossings[0].toLeft, 3U);
  ASSERT_EQ(crossings[0].toRight, 4U);
}

TEST(crossingDiscoveryTest, findsTopmostCrossingFirstFail) {
  auto str = R"(
0   1   2
 \   \ /
  \   X
   \ / \
    X   \
   / \   \
  3   4   5
)";
  auto [dag, layers] = parseWithLayers(str);
  ASSERT_EQ(layers.size(), 2U);
  EXPECT_EQ(countCrossings(dag, layers[0], layers[1]), 2U);
  auto crossings = findNonConflictingCrossings(dag, layers[0], layers[1]);
  ASSERT_EQ(crossings.size(), 1U);
  ASSERT_EQ(crossings[0].fromLeft, 0U);
  ASSERT_EQ(crossings[0].fromRight, 2U);
  ASSERT_EQ(crossings[0].toLeft, 3U);
  ASSERT_EQ(crossings[0].toRight, 4U);
  // FIXME: should be the other crossing:
  // ASSERT_EQ(crossings[0].fromLeft, 1U);
  // ASSERT_EQ(crossings[0].fromRight, 2U);
  // ASSERT_EQ(crossings[0].toLeft, 3U);
  // ASSERT_EQ(crossings[0].toRight, 5U);
}

TEST(crossingDiscoveryTest, twoCrossingsSameEdgeDifferentSides) {
  auto str = R"(
0   1   2
 \ /   /
  X   /
 / \ /
 |  X
 \ / \
  X   \
 / \  |
3   4 5
)";
  auto [dag, layers] = parseWithLayers(str);
  ASSERT_EQ(layers.size(), 2U);
  EXPECT_EQ(countCrossings(dag, layers[0], layers[1]), 3U);
  auto crossings = findNonConflictingCrossings(dag, layers[0], layers[1]);
  ASSERT_EQ(crossings.size(), 1U);
  ASSERT_EQ(crossings[0].fromLeft, 0U);
  ASSERT_EQ(crossings[0].fromRight, 1U);
  ASSERT_EQ(crossings[0].toLeft, 4U);
  ASSERT_EQ(crossings[0].toRight, 5U);
}
