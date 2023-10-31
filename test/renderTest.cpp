#include "asciidag.h"

#include <gtest/gtest.h>

using namespace asciidag;

std::string renderSuccessfully(DAG const& dag) {
  RenderError err;
  auto result = renderDAG(dag, err);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(err.code, RenderError::Code::None);
  if (!result) {
    return "";
  }
  return "\n" + *result;
}

TEST(render, singleNode) {
  DAG test;
  test.nodes.push_back(DAG::Node{{}, "#"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
#
)");
}

TEST(render, singleEdge) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0
|
|
|
1
)");
}

TEST(render, twoSimpleEdgesDiverge) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0
|\
| \
| |
| |
1 2
)");
}

TEST(render, twoSimpleEdgesDivergeDiffOrder) {
  DAG test;
  test.nodes.push_back(DAG::Node{{2, 1}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0
|\
| \
| |
| |
1 2
)");
}

TEST(render, twoSimpleEdgesConverge) {
  DAG test;
  test.nodes.push_back(DAG::Node{{2}, "0"});
  test.nodes.push_back(DAG::Node{{2}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0 1
| |
| |
| /
|/
2
)");
}

TEST(render, threeSimpleEdgesDiverge) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2, 3}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
  0
 /|\
/ | \
| | |
| | |
1 2 3
)");
}

TEST(render, threeSimpleEdgesDivergeOrder321) {
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 2, 1}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
  0
 /|\
/ | \
| | |
| | |
1 2 3
)");
}

TEST(render, threeSimpleEdgesDivergeOrder312) {
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 1, 2}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
  0
 /|\
/ | \
| | |
| | |
1 2 3
)");
}

TEST(render, threeSimpleEdgesConverge) {
  DAG test;
  test.nodes.push_back(DAG::Node{{3}, "0"});
  test.nodes.push_back(DAG::Node{{3}, "1"});
  test.nodes.push_back(DAG::Node{{3}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0 1 2
| | |
| | |
\ | /
 \|/
  3
)");
}

TEST(render, twoParallelSimpleEdges) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{3}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0 2
| |
| |
| |
1 3
)");
}

TEST(render, nonStraightRightEdge) {
  // Edges come close and touch slightly ambiguously, but
  // the parser can figureout proper handling for it
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{4, 5}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0   3
|\  |\
| \ | \
| | | |
| | | |
1 2 4 5
)");
}

TEST(render, tripleEdgePair) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 1, 1}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
  0
 /|\
 |||
 \|/
  1
)");
}

TEST(render, twoTripleEdgePairs) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 1, 1}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{3, 3, 3}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
  0     2
 /|\   /|\
 |||   |||
 \|/   \|/
  1     3
)");
}

TEST(render, nonStraightLeftEdge) {
  DAG test;
  test.nodes.push_back(DAG::Node{{2}, "0"});
  test.nodes.push_back(DAG::Node{{2}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{5}, "3"});
  test.nodes.push_back(DAG::Node{{5}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0 1 3 4
| | | |
| | | |
| / | /
|/  |/
2   5
)");
}


TEST(render, twoParallelRightEdges) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{4}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0   3
|\  |
| \ |
| | |
| | |
1 2 4
)");
}

TEST(render, twoParallelLeftEdges) {
  DAG test;
  test.nodes.push_back(DAG::Node{{}, "0"});
  test.nodes.push_back(DAG::Node{{2}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{4}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0 1 3
  | |
  / /
 / /
/ /
| |
2 4
)");
}

TEST(render, hammock) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2}, "0"});
  test.nodes.push_back(DAG::Node{{3}, "1"});
  test.nodes.push_back(DAG::Node{{3}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
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

TEST(render, multiLayerEdge) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2}, "0"});
  test.nodes.push_back(DAG::Node{{2}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0
|\
| \
| |
| |
1 |
| |
| |
| /
|/
2
)");
}

TEST(render, twoMultiLayerEdges) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2, 3}, "0"});
  test.nodes.push_back(DAG::Node{{2, 3}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
  0
 /|\
/ | \
| |  \
| |   \
| |   |
| |   |
| 1   |
| |\  |
| | \ |
| / | /
|/  |/
2   3
)");
}

TEST(render, twoLayerEdge) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 3}, "0"});
  test.nodes.push_back(DAG::Node{{2}, "1"});
  test.nodes.push_back(DAG::Node{{3}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0
|\
| \
| |
| |
1 |
| |
| |
| |
2 |
| |
| |
| /
|/
3
)");
}

TEST(render, fourLayers) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2, 3}, "#"});
  test.nodes.push_back(DAG::Node{{4}, "1"});
  test.nodes.push_back(DAG::Node{{4}, "2"});
  test.nodes.push_back(DAG::Node{{5}, "3"});
  test.nodes.push_back(DAG::Node{{5}, "4"});
  test.nodes.push_back(DAG::Node{{}, "B"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
  #
 /|\
/ | \
| | |
| | |
1 2 3
| | |
| | |
| / |
|/  |
4   |
|   |
|   |
|   /
|  /
| /
|/
B
)");
}

TEST(renderError, emptyStringNodeUnsupported) {
  DAG test;
  test.nodes.push_back(DAG::Node{{}, ""});
  RenderError err;
  auto result = renderDAG(test, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, RenderError::Code::Unsupported);
  EXPECT_EQ(err.nodeId, 0U);
}

TEST(renderError, wideStringNodeUnsupported) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1}, "."});
  test.nodes.push_back(DAG::Node{{}, ".."});
  RenderError err;
  auto result = renderDAG(test, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, RenderError::Code::Unsupported);
  EXPECT_EQ(err.nodeId, 1U);
}

TEST(renderError, tooManyOutgoingEdges) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2, 3, 4}, "."});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  RenderError err;
  auto result = renderDAG(test, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, RenderError::Code::Overcrowded);
  EXPECT_EQ(err.nodeId, 0U);
}

TEST(renderError, tooManyIncomingEdges) {
  DAG test;
  test.nodes.push_back(DAG::Node{{4}, "0"});
  test.nodes.push_back(DAG::Node{{4}, "1"});
  test.nodes.push_back(DAG::Node{{4}, "2"});
  test.nodes.push_back(DAG::Node{{4}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  RenderError err;
  auto result = renderDAG(test, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, RenderError::Code::Overcrowded);
  EXPECT_EQ(err.nodeId, 4U);
}

TEST(render, conflictingEdgesFromSamePredecessor) {
  DAG test;
  test.nodes.push_back(DAG::Node{{}, "0"});
  test.nodes.push_back(DAG::Node{{4}, "1"});
  test.nodes.push_back(DAG::Node{{4, 5}, "2"});
  test.nodes.push_back(DAG::Node{{5}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test), R"(
0 1 2   3
  | |\  |
  | |/  |
  / /|  /
 / / / /
/ / / /
|/  |/
4   5
)");
}

TEST(render, twoEdgeCrossingsFromSamePredecessor) {
  DAG test;
  test.nodes.push_back(DAG::Node{{2, 3, 4}, "0"});
  test.nodes.push_back(DAG::Node{{2, 3}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0   1
 /|\  |\
/ | \ | \
| |  \\  \
| |  | \  \
| |  | |  |
| |  \ /  |
| |   X   |
| |  / \  |
| | /  /  |
| |/  /   /
| ||  |  /
| ||  | /
| |/  |/
4 2   3
)");
}

TEST(render, dependenciesRightToLeft) {
  // If edge 2->3 is drawn before 2->4, 2->4 would have not enough space
  DAG test;
  test.nodes.push_back(DAG::Node{{}, "0"});
  test.nodes.push_back(DAG::Node{{4}, "1"});
  test.nodes.push_back(DAG::Node{{3, 4}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  EXPECT_EQ(renderSuccessfully(test), R"(
0 1 2
  | |\
  | |/
  / /|
 / / /
/ / /
|/  |
4   3
)");
}

TEST(render, complex6nodes) {
  // Crossing minimization fails here to swap two edges
  // then crossing insertion somehow fails to help the problem -
  // it does not reduce the number of edge crosses
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "0"});
  test.nodes.push_back(DAG::Node{{2, 3, 4}, "1"});
  test.nodes.push_back(DAG::Node{{3, 5}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0     1
 /|\   /|\
/ | \ / | \
| | |/  |  \
| | ||  |   \
| | ||  |   |
| | |/  |   |
| | 4   2   |
| |     |\  |
| |     |/  |
| |     /|  /
| |    / / /
| \   / / /
|  \ /  | |
|   X   | |
|  / \  | |
| /  |  | |
|/   |  / /
||   | / /
||   |/ /
|/   \|/
5     3
)");
}

TEST(render, wellConnected6nodes) {
  DAG test;
  test.nodes.push_back(DAG::Node{{2, 3, 4}, "0"});
  test.nodes.push_back(DAG::Node{{2, 3, 4}, "1"});
  test.nodes.push_back(DAG::Node{{5}, "2"});
  test.nodes.push_back(DAG::Node{{4}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0     1
 /|\   /|\
/ | \  || \
| |  \ |\  \
| |  | | \  \
| |  | |  \ |
| |  \ /  | |
| |   X   | |
| |  / \  | |
| | /  /  | |
| |/  /   / /
| ||  |  / /
| ||  | / /
| |/  |/  |
| 2   3   |
| |   |   |
| \   |   /
|  \  |  /
\  |  | /
 \ /  | |
  X   | |
 / \  | |
/  |  | |
|  |  / /
|  | / /
|  |/ /
|  \|/
5   4
)");
}

TEST(render, wellConnected6nodes2) {
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "0"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "1"});
  test.nodes.push_back(DAG::Node{{3, 4}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0     1   2
 /|\   /|\  |\
/ | \  || \ | \
| |  \ ||  \\  \
| |  | ||   \\  \
| |  | |\    \\  \
| |  | | \   | \  \
| |  | |  \  | |  |
| |  \ /  |  \ /  |
| |   X   |   X   |
| |  / \  |  / \  |
| |  |  \ | /  /  /
| |  |  | | | /  /
| \  |  | | | | /
|  \ /  | | | | |
|   X   | | | | |
|  / \  | | | | |
| /  /  / | | | |
| | /  /  / | | |
| | |  \ /  | | |
| | |   X   | | |
| | |  / \  | | |
| | | /  |  | | |
| | | |  |  / | |
| | | |  \ /  | |
| | | |   X   | |
| | | |  / \  | |
| | | | /  |  | |
| | | |/   |  / /
| | | ||   | / /
| / \ ||   |/ /
|/   \|/   \|/
5     3     4
)");
}

TEST(render, crossedPlusUncrossedGraphs) {
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 5}, "0"});
  test.nodes.push_back(DAG::Node{{4}, "1"});
  test.nodes.push_back(DAG::Node{{3, 5}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test), R"(
0   1 2
|\  | |\
| \ | | \
|  \\ \  \
|  | \ \  \
|  | |  \ |
|  \ /  | |
|   X   | |
|  / \  | |
| /  |  | |
| |  |  / |
| |  \ /  |
| |   X   |
| |  / \  |
| |  |  \ |
| \  |  | |
|  \ /  | |
|   X   | |
|  / \  | |
| /  /  | |
|/  /   / /
||  |  / /
||  | / /
|/  | |/
3   4 5
)");
}

TEST(render, wellConnected6nodes3) {
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 5}, "0"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "1"});
  test.nodes.push_back(DAG::Node{{3, 5}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test), R"(
0     1   2
|\   /|\  |\
| \  || \ | \
|  \ ||  \\  \
|  | ||   \\  \
|  | |\    \\  \
|  | | \   | \  \
|  | |  \  | |  |
|  \ /  |  \ /  |
|   X   |   X   |
|  / \  |  / \  |
| /  |  | /  /  /
| |  |  | | /  /
| |  |  / | | /
| |  \ /  | | |
| |   X   | | |
| |  / \  | | |
| | /  |  | | |
| | |  |  / | |
| | |  \ /  | |
| | |   X   | |
| | |  / \  | |
| | |  |  \ | |
| | \  |  | | |
| |  \ /  | | |
| |   X   | | |
| |  / \  | | |
| | /  /  | | |
| |/  /   / / /
| ||  |  / / /
\ ||  |  |/ /
 \|/  |  \|/
  3   4   5
)");
}

TEST(render, fullyConnected3x3) {
  // Requires 5 iterations in crossing-insertion/crossing minimization
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "0"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "1"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0     1     2
 /|\   /|\   /|\
/ | \  || \  \\ \
| |  \ |\  \  \\ \
| |  | | \  \ | \ \
| |  | |  \ | | | |
| |  \ /  | | | | |
| |   X   | | | | |
| |  / \  | | | | |
| | /  /  / / | | |
| | | /  / /  | | |
| | | | /  |  / | |
| | | | |  \ /  | |
| | | | |   X   | |
| | | | |  / \  | |
| | | | |  |  \ | |
| | | | \  |  | | |
| | | |  \ /  | | |
| | | |   X   | | |
| | | |  / \  | | |
| | | |  \  \ \ \ \
| | | |   \  \ \ \ \
| | \ \    \  \ \ \ \
| |  \ \   |  |  \ \ \
| \  |  \  |  |  | |  \
|  \ /   \ /  |  \ /  |
|   X     X   |   X   |
|  / \   / \  |  / \  |
| /  |  /  /  | /  /  /
| |  | /  /   |/  /  /
| |  | |  |   /| /  /
| |  | |  |  / //  /
| |  | |  | / / | /
| |  \ /  | | | | |
| |   X   | | | | |
| |  / \  | | | | |
| | /  |  | | | | |
| | |  |  / | | | |
| | |  \ /  | | | |
| | |   X   | | | |
| | |  / \  | | | |
| | | /  |  | | | |
| | | |  |  / | | |
| | | |  \ /  | | |
| | | |   X   | | |
| | | |  / \  | | |
| | | | /  |  | | |
| | | | |  |  / | |
| | | | |  \ /  | |
| | | | |   X   | |
| | | | |  / \  | |
| | | | | /  |  | |
| | | | |/   |  / /
| | | | ||   | / /
\ | / \ ||   |/ /
 \|/   \|/   \|/
  3     4     5
)");
}

TEST(render, narrowGapForAnEdgeEgress) {
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "0"});
  test.nodes.push_back(DAG::Node{{3, 6}, "1"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  test.nodes.push_back(DAG::Node{{}, "6"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0     2   1
 /|\   /|\  |\
/ | \  || \ | \
| |  \ |\  \\  \
| |  | | \  \\  \
| |  | |  \ | \ |
| |  \ /  | | | |
| |   X   | | | |
| |  / \  | | | |
| |  |  \ | | | |
| \  |  | | | | |
|  \ /  | | | | |
|   X   | | | | |
|  / \  | | | | |
| /  /  / | | | |
| | /  /  / | | |
| | |  \ /  | | |
| | |   X   | | |
| | |  / \  | | |
| | | /  |  | | |
| | |/   |  / / /
| | ||   | / / /
| / ||   |/ / /
|/  |/   \|/  |
4   5     3   6
)");
}

TEST(render, narrowGapForAnEdgeIngress) {
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "0"});
  test.nodes.push_back(DAG::Node{{2, 3, 4}, "1"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "2"});
  test.nodes.push_back(DAG::Node{{5, 6}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  test.nodes.push_back(DAG::Node{{}, "6"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0     1
 /|\   /|\
/ | \ / | \
| | | | |  \
| | | | |   \
| | | | \    \
| | | |  \    \
| | | |   \   |
| | | |   |   |
| | | |   2   |
| | | |  /|\  |
| | | |  \\ \ \
| | | |   \\ \ \
| | | \    \\ \ \
| | |  \    \\ \ \
| \ \   \   | \ \ \
|  \ \   \  | |  \ \
\  |  \  |  | |  | |
 \ /   \ /  | |  \ /
  X     X   | |   X
 / \   / \  | |  / \
/  /  /  /  | | /  /
| /  /  /   / //  /
| | /  /   / //  /
| | |  |  / / | /
| | |  \ /  | | |
| | |   X   | | |
| | |  / \  | | |
| | | /  |  | | |
| | |/   |  / / /
| | ||   | / / /
| \ ||   |/ / /
|  \|/   \|/  |
|   3     4   |
|   |\        |
|   | \       /
|   | |      /
|   | |     /
|   | |    /
|   | |   /
|   | |  /
\   / | /
 \ /  | |
  X   | |
 / \  | |
/  |  | |
|  |  / /
|  | / /
|  |/ /
|  \|/
6   5
)");
}

TEST(render, notHemmingInAnyEdge) {
  // Here the edges around the edge 0->6 can come too
  // close to each other and prevent the edge 0->6 from passing,
  // even though there is enough space for all three of them
  DAG test;
  test.nodes.push_back(DAG::Node{{5, 6, 7}, "0"});
  test.nodes.push_back(DAG::Node{{2, 3, 4}, "1"});
  test.nodes.push_back(DAG::Node{{3, 5, 6}, "2"});
  test.nodes.push_back(DAG::Node{{5}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  test.nodes.push_back(DAG::Node{{}, "6"});
  test.nodes.push_back(DAG::Node{{}, "7"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0     1
 /|\   /|\
/ | \  \\ \
| | |   \\ \
| | |   | \ \
| | |   |  \ \
| | |   |   \ \
| | |   |   | |
| | |   |   | |
7 | |   2   4 |
  | |  /|\    |
  | | / |/    |
  | |/  ||    /
  | ||  ||   /
  / /|  /|  /
 / / / / / /
/ / / / / /
| |/  | |/
| 6   | 3
|     | |
|     | |
|     / /
|    / /
|   / /
|  / /
\ / /
 \|/
  5
)");
}

TEST(render, full3x3PlusTwoDisconnectedNodes) {
  DAG test;
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "0"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "1"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  test.nodes.push_back(DAG::Node{{}, "6"});
  test.nodes.push_back(DAG::Node{{}, "7"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0   6 7   1     2
 /|\       /|\   /|\
/ | \     / |/  / |/
| | |    /  /| /  ||
| | |   /  / //   ||
| | |  /  / //    /|
| | | /  / / |   / /
| | | | /  | |  / /
| | | | |  \ /  | |
| | | | |   X   | |
| | | | |  / \  | |
| | | | |  |  \ | |
| | | | \  |  | | |
| | | |  \ /  | | |
| | | |   X   | | |
| | | |  / \  | | |
| | | |  |  \ | | |
| | | \  |  | | | |
| | |  \ /  | | | |
| | |   X   | | | |
| | |  / \  | | | |
| | |  |  \ | | | |
| | \  |  | | | | |
| |  \ /  | | | | |
| |   X   | | | | |
| |  / \  | | | | |
| |  |  \ | | | | |
| \  |  | | | | | |
|  \ /  | | | | | |
|   X   | | | | | |
|  / \  | | | | | |
| /  |  | | | | | |
| |  |  / | | | | |
| |  \ /  | | | | |
| |   X   | | | | |
| |  / \  | | | | |
| |  \  \ | | | \ \
| |   \  \| | |  \ \
| \   |  |\ \ \   \ \
|  \  |  | \ \ \   \ \
\  |  |  | |  \ \  |  \
 \ /  |  \ /  |  \ /  |
  X   |   X   |   X   |
 / \  |  / \  |  / \  |
/  |  | /  /  | /  /  /
|  |  | | /   |/  /  /
|  |  | | |   /| /  /
|  |  | | |  / //  /
|  |  / | | / / | /
|  \ /  | | | | | |
|   X   | | | | | |
|  / \  | | | | | |
| /  |  | | | | | |
| |  |  / | | | | |
| |  \ /  | | | | |
| |   X   | | | | |
| |  / \  | | | | |
| | /  /  / | | | |
| | | /  /  / | | |
| | | |  \ /  | | |
| | | |   X   | | |
| | | |  / \  | | |
| | | | /  |  | | |
| | | | |  |  / | |
| | | | |  \ /  | |
| | | | |   X   | |
| | | | |  / \  | |
| | | | | /  |  | |
| | | | |/   |  / /
| | | | ||   | / /
\ | / \ ||   |/ /
 \|/   \|/   \|/
  3     4     5
)");
}

TEST(render, case8nodes1) {
  DAG test;
  test.nodes.push_back(DAG::Node{{2, 4, 5}, "0"});
  test.nodes.push_back(DAG::Node{{5, 6, 7}, "1"});
  test.nodes.push_back(DAG::Node{{4, 6, 7}, "2"});
  test.nodes.push_back(DAG::Node{{4, 5, 7}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{6}, "5"});
  test.nodes.push_back(DAG::Node{{}, "6"});
  test.nodes.push_back(DAG::Node{{}, "7"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0     1     3
 /|\   /|\   /|\
/ | \ / | \  || \
| | | | |  \ |\  \
| | | | |  | | \  \
| | | | |  | |  \ |
| | | | |  \ /  | |
| | | | |   X   | |
| | | | |  / \  | |
| | | | |  |  \ | |
| | | | \  |  | | |
| | | |  \ /  | | |
| | | |   X   | | |
| | | |  / \  | | |
| | | |  \  \ | | |
| \ \ \   \  \\ | |
|  \ \ \   \  \\\ |
\   \ \ \  |  | \\\
 \   \ \ \ |  | | \\
  \   \ \ \|  | | | \
  |   |  \|/  | | | |
  2   |   5   | | | |
 /|\  |   |   | | | |
/ | \ |   |   / / / /
| | | |   |  / / / /
| | | \   / / / / /
| | |  \ /  | | | |
| | |   X   | | | |
| | |  / \  | | | |
| | |  |  \ | | | |
| | \  |  | | | | |
| |  \ /  | | | | |
| |   X   | | | | |
| |  / \  | | | | |
| |  |  \ | | | \ \
| |  |   \| | |  \ \
| |  |   |\ \ \   \ \
| |  |   | \ \ \   \ \
| \  |   | |  \ \  |  \
|  \ /   \ /  |  \ /  |
|   X     X   |   X   |
|  / \   / \  |  / \  |
| /  /  /  /  | /  /  /
| | /  /  /   |/  /  /
| | | /  /    /| /  /
| | | |  |   / //  /
| | | |  |  / / | /
| | | |  \ /  | | |
| | | |   X   | | |
| | | |  / \  | | |
| | | | /  |  | | |
| | | | |  |  / | |
| | | | |  \ /  | |
| | | | |   X   | |
| | | | |  / \  | |
| | | | \  \  \ | |
| | | \  \  \ | | |
| | |  \ /  | | | |
| | |   X   | | | |
| | |  / \  | | | |
| | |  |  \ | | | |
| | \  |  | | | | |
| |  \ /  | | | | |
| |   X   | | | | |
| |  / \  | | | | |
| | /  |  | | | | |
| |/   |  / / / / /
| ||   | / / / / /
\ ||   |/ /  |/ /
 \|/   \|/   \|/
  6     4     7
)");
}

TEST(render, crossingsSharingTheSameXnodeAbove) {
  // Here at one point, if not taken special precaution, the algo will try to
  // insert two "X" nodes that share the same "X" predecessor, and it will do
  // that in the wrong order swapping the meaning of the edges going through the
  // "X". keepOneCrossingPerXNode prevents this.
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 6, 7}, "0"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "1"});
  test.nodes.push_back(DAG::Node{{3, 4, 5}, "2"});
  test.nodes.push_back(DAG::Node{{4}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{6, 7}, "5"});
  test.nodes.push_back(DAG::Node{{}, "6"});
  test.nodes.push_back(DAG::Node{{}, "7"});
  EXPECT_EQ(renderSuccessfully(test), R"(
  0     2
 /|\   /|\
 \\ \  \\ \
  \\ \  \\ \
  | \ \  \\ \
  |  \ \  \\ \
  |   \ \ | \ \
  |   | | | | |
  |   | | | | |
  1   | | | | |
 /|\  | | | | |
/ | \ | | \ \ \
| |  \| |  \ \ \
| |  |\ \   \ \ \
| |  | \ \   \ \ \
| |  | |  \  |  \ \
| |  \ /   \ /  | |
| |   X     X   | |
| |  / \   / \  | |
| \  \  \  |  \ | |
|  \  \  \ |  | | |
\  |  |  | |  | | |
 \ /  |  \ /  | | |
  X   |   X   | | |
 / \  |  / \  | | |
/  /  | /  /  / | |
| /   //  /  /  / /
| |  //  /  /  / /
| | / | /  /  / /
| | | | |  \ /  |
| | | | |   X   |
| | | | |  / \  |
| | | \ \  \  \ \
| | |  \ \  \  \ \
| | \  |  \ |  | |
| |  \ /  | |  \ /
| |   X   | |   X
| |  / \  | |  / \
| | /  /  / | /  /
| | | /  /  / | /
| | | |  \ /  | |
| | | |   X   | |
| | | |  / \  | |
| | | | /  /  | |
| | | | | /   / /
| | | | | |  / /
| | / | | | / /
| |/  | | |/  |
| 3   | | 5   |
| |   | | |\  |
| |   / | | \ |
| |  /  / | | |
| |  \ /  | | |
| |   X   | | |
| |  / \  | | |
| | /  /  | | |
| |/  /   / / /
| ||  |  / / /
\ ||  | / / /
 \|/  |/  |/
  4   6   7
)");
}
