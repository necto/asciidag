#include "asciidagImpl.h"
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
|  \
1   2
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
4 3
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
4 3
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
3 4
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
|  \
1   2
|  /
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
1 0   2
| |  /
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
0 1   2
| |  /
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
| |  \
4 3   5
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
| |  \
5 3   4
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
| |  \
4 3   5
| |  /
| | /
| |/
6 7
)");
}

TEST(crossingMinimizationTest, untangleEquipotentialStrayNodeUpperLayer) {
  EXPECT_EQ(parseAndRender(R"(
2   3 4
|\  | |
| \ / |
|  X  |
| / \ |
|/   \|
5     6
)"),
R"(
3   2     4
|  / \   /
| /  /  /
|/  /  /
||  | /
|/  |/
5   6
)");
}

TEST(crossingMinimizationTest, untangleEquipotentialStrayNodeLowerLayer) {
  EXPECT_EQ(parseAndRender(R"(
0     1
|\   /|
| \ / |
|  X  |
| / \ |
|/  | |
2   3 4
)"),
R"(
0   1
|\  |\
| \ \ \
|  \ \ \
|  | |  \
|  \ /   \
3   2     4
)");
}

TEST(crossingMinimizationTest, untangleEquipotentialStrayNodeMiddleLayer) {
  EXPECT_EQ(parseAndRender(R"(
0     1
|\   /|
| \ / |
|  X  |
| / \ |
|/  | |
2   3 4
|\  | |
| \ / |
|  X  |
| / \ |
|/   \|
5     6
)"),
R"(
0   1
|\  |\
| \ \ \
|  \ \ \
|  | |  \
|  \ /   \
3   2     4
|  / \   /
| /  /  /
|/  /  /
||  | /
|/  |/
5   6
)");
}

TEST(crossingMinimizationTest, untangleCentripetalSymmetricalCrossingFail) {
  // TODO
  // Here both on the layer above and layer below, all 3 nodes
  // aim at the center (position 1), so they stay where they are
  // Also, moving nodes only on one layer does not reduce number of crossings
  // you must move nodes on both layers simultaneously
  EXPECT_EQ(parseAndRender(R"(
 0   1   2
  \ / \ /
   X   X
  / \ / \
 3   4   5
)"),
R"(
0     1     2
 \   / \   /
 |  /  |  /
 | /   | /
 | |   | |
 \ /   \ /
  X     X
 / \   / \
 |  \  |  \
 |   \ |   \
 |   | |   |
 /   \ /   \
3     4     5
)");
}

TEST(crossingMinimizationTest, untangleUnrelatedGraphInTheMiddleFail) {
  // TODO
  // Here the 1-4 graph is best taken on the side to avoid extra edge crossings.
  // However, moving only 1 or only 4 to either side brings no immediate improvement,
  // so it stays in place.
  EXPECT_EQ(parseAndRender(R"(
 0 1 2
 |\|/|
 | X |
 |/|\|
 3 4 5
)"),
R"(
0   1 2
|\  | |\
| \ \ \ \
|  \ \ \ \
|  | |  \ \
|  \ /  | |
|   X   | |
|  / \  | |
| /  |  / |
| |  \ /  |
| |   X   |
| |  / \  |
| \  |  \ |
|  \ /  | |
|   X   | |
|  / \  | |
| /  /  / /
|/  /  / /
||  | / /
|/  | |/
3   4 5
)");
}

TEST(crossingMinimizationTest, deconstructedRenderingNoMinimizationParallel) {
  auto str = R"(
0 1
| |
2 3
)";
  auto [dag, layers] = parseWithLayers(str);
  EXPECT_EQ(str, '\n' + renderDAGWithLayers(dag, layers));
}

TEST(crossingMinimizationTest, deconstructedRenderingNoUncrossing) {
  auto str = R"(
0 1
| |
\ /
 X
/ \
| |
2 3
)";
  auto [dag, layers] = parseWithLayers(str);
  EXPECT_EQ(R"(
0     1
 \
  \
   \
    \
 /   \
2     3
)", '\n' + renderDAGWithLayers(dag, layers));
}

TEST(crossingMinimizationTest, deconstructedRenderingNoMinimizationCrossing) {
  auto str = R"(
0     1
 \   /
 |  /
 \ /
  X
 / \
 |  \
 /   \
2     3
)";
  auto [dag, layers] = parseWithLayers(str);
  layers = insertCrossNodes(dag, layers);
  EXPECT_EQ(str, '\n' + renderDAGWithLayers(dag, layers));
}

TEST(crossingMinimizationTest, deconstructedRenderingCrossingRemoved) {
  auto str = R"(
0   1
 \ /
  X
 / \
2   3
)";
  auto [dag, layers] = parseWithLayers(str);
  minimizeCrossings(layers, dag);
  EXPECT_EQ(R"(
0 1
| |
3 2
)", '\n' + renderDAGWithLayers(dag, layers));
}

TEST(crossingMinimizationTest, danglingNodeDoesNotPreventSimpleSwap) {
  // In the bottom-up pass, the dangling node 4 here might have prevented the
  // proper swap of 2 and 3 that removes crossing in the lower layer, because if
  // 4 is assigned an incorrect target position, it might introduce crossings
  // with the upper layer
  auto str = R"(
 0   1
/|\ /|\
|| \\\ \
||  \\\ \
||  || \ \
|\  || |  \
| \ || |  |
| | |/ |  |
2 3 4  5  6
| |    |  |
\ /    /  /
 X    /  /
/ \  /  /
|  \ | /
|   \|/
7    8
)";
  auto [dag, layers] = parseWithLayers(str);
  minimizeCrossings(layers, dag);
  EXPECT_EQ(R"(
  0     1
 /|\   /|\
 || \  \\ \
 ||  \  \\ \
 ||   \  \\ \
 ||    \  \\ \
 ||     \  \\ \
 ||      \ | \ \
 |\      | |  \ \
 | \     | |   \ \
 |  \    | |   |  \
 /   \   \ /   \   \
3     2   4     5   6
|     |        /   /
|     |       /   /
|     |      /   /
|     |     /   /
|     |    /   /
|     |   /   /
|     |  /   /
|     | /   /
|     |/   /
|     /|  /
|    / / /
|   / / /
|  / / /
|  |/ /
|  \|/
7   8
)", '\n' + renderDAGWithLayers(dag, layers));
}
