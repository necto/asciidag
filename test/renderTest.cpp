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
1
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
1 .
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
1 . .
2 3
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
1 .
2 .
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
  test.nodes.push_back(DAG::Node{{}, "."});
  EXPECT_EQ(renderSuccessfully(test),
            R"(
#
1 2 3
4 .
.
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
