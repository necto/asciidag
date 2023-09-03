#include "asciidag.h"

#include <gtest/gtest.h>
#include <string>

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

TEST(parse, twoConnectedNodesPipe) {
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

TEST(parse, longEdgePipe) {
  std::string str = R"(
   .
   |
   |
   .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, longSlash) {
  std::string str = R"(
    .
   /
  /
 .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, longEdgeBackslash) {
  std::string str = R"(
   .
    \
     \
      .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, longEdgeSlashPipe) {
  std::string str = R"(
     .
    /
    |
    .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, longEdgeBackslashPipe) {
  std::string str = R"(
   .
    \
    |
    .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, longEdgePipeSlash) {
  std::string str = R"(
     .
     |
     /
    .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, longEdgePipeBackslash) {
  std::string str = R"(
   .
   |
   \
    .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, longEdgeWiggly) {
  std::string str = R"(
   .
   |
   \
    \
    |
    \
    |
    /
    |
    /
   /
   |
   /
  /
 .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoEdges) {
  std::string str = R"(
   .
  / \
 .   .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 3U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 2U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[1].to, 2U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[2].outEdges.size(), 0U);
}

TEST(parse, hammock) {
  std::string str = R"(
   .
  / \
 .   .
  \ /
   .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 4U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 2U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[1].to, 2U);
  ASSERT_EQ(dag.nodes[1].outEdges.size(), 1U);
  ASSERT_EQ(dag.nodes[2].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[1].outEdges[0].to, 3U);
  EXPECT_EQ(dag.nodes[2].outEdges[0].to, 3U);
  EXPECT_EQ(dag.nodes[3].outEdges.size(), 0U);
}

TEST(parse, skewedHammock) {
  std::string str = R"(
   .
  / \
 .   .
 |   |
 |  /
 | /
 ||
 \|
  .
)";
  auto dag = parseDAG(str);
  ASSERT_EQ(dag.nodes.size(), 4U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 2U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[1].to, 2U);
  ASSERT_EQ(dag.nodes[1].outEdges.size(), 1U);
  ASSERT_EQ(dag.nodes[2].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[1].outEdges[0].to, 3U);
  EXPECT_EQ(dag.nodes[2].outEdges[0].to, 3U);
  EXPECT_EQ(dag.nodes[3].outEdges.size(), 0U);
}
