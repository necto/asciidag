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
|| \
|\  \
| \ |
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
|| \
|\  \
| \ |
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
|| \
|\  \
| \ |
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
| / /
|/ /
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
  // FIXME: edges touch ambiguously
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2}, "0"});
  test.nodes.push_back(DAG::Node{{}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{4, 5}, "3"});
  test.nodes.push_back(DAG::Node{{}, "4"});
  test.nodes.push_back(DAG::Node{{}, "5"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
0  3
|\ |\
| \| \
| |\  \
| | \ |
| | | |
1 2 4 5
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
| | / /
| // /
|/ |/
2  5
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
0  3
|\ |
| \\
| | \
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
1 .
| |
| |
| /
|/
2
)");
}

TEST(render, twoMultiLayerEdgesBroken) {
  // TODO: handle intersection, or shift the nodes in the layers
  DAG test;
  test.nodes.push_back(DAG::Node{{1, 2, 3}, "0"});
  test.nodes.push_back(DAG::Node{{2, 3}, "1"});
  test.nodes.push_back(DAG::Node{{}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
 0
/|\
|| \
|\  \
| \  \
|  \ |
|  | |
1  . .
|\ | |
| \| |
|  / |
| /| /
|/ |/
2  3
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
1 .
| |
| |
| |
2 .
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
|| \
|\  \
| \ |
| | |
1 2 3
| | |
| | /
| //
|/ |
4  .
|  |
|  |
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
