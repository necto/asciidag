#include <gtest/gtest.h>
#include <string>

#include "asciidag.h"

TEST(parse, empty) {
  std::string str = R"(

)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 0U);
}

TEST(parse, singleNode) {
  std::string str = R"(

    .

)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges.size(), 0U);
}

TEST(parse, twoDisconnectedNodes) {
  std::string str = R"(
    . .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  EXPECT_EQ(dag.nodes[0].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoImmediatelyConnectedNodes) {
  std::string str = R"(
    .
    .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedNodesVertical) {
  std::string str = R"(
    .
    |
    .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedNodesSlash) {
  std::string str = R"(
     .
    /
   .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedNodesBackslash) {
  std::string str = R"(
   .
    \
     .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}
