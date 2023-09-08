#include "asciidag.h"

#include <gtest/gtest.h>
#include <string>


DAG parseSuccessfully(std::string str) {
  ParseError err;
  auto dag = parseDAG(str, err);
  EXPECT_TRUE(dag.has_value());
  if (dag) {
    return *dag;
  }
  return DAG{};
}

TEST(parse, empty) {
  std::string str = R"(

)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 0U);
}

TEST(parse, singleNode) {
  std::string str = R"(

    .

)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges.size(), 0U);
}

TEST(parse, singleWideNode) {
  std::string str = R"(

    ###

)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges.size(), 0U);
}

TEST(parse, twoDisconnectedNodes) {
  std::string str = R"(
    . .
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  EXPECT_EQ(dag.nodes[0].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoWideNodes) {
  std::string str = R"(
    AA BB
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  EXPECT_EQ(dag.nodes[0].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoImmediatelyConnectedNodes) {
  std::string str = R"(
    .
    .
)";
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesPipeLeft) {
  std::string str = R"(
    ##
    |
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesPipeRight) {
  std::string str = R"(
    ##
     |
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesPipeRightSkew) {
  std::string str = R"(
    ##
     |
     ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesPipeLeftSkew) {
  std::string str = R"(
     ##
     |
    ##
)";
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesSlashRightSkew) {
  std::string str = R"(
    ##
    /
   ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesSlashRightDoubleSkewLeft) {
  std::string str = R"(
    ##
   /
  ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesSlashRightDoubleSkewRight) {
  std::string str = R"(
    ##
    /
  ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesSlashRightTripleSkew) {
  std::string str = R"(
     ##
    /
  ##
)";
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesBackslashRightSkew) {
  std::string str = R"(
   ##
    \
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesBackslashRightDoubleSkewLeft) {
  std::string str = R"(
  ##
   \
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesBackslashRightDoubleSkewRight) {
  std::string str = R"(
   ##
    \
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
}

TEST(parse, twoConnectedWideNodesBackslashRightTripleSkew) {
  std::string str = R"(
  ##
    \
     ##
)";
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
  auto dag = parseSuccessfully(str);
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
 |   /
 |  /
 | /
 | |
 \ /
  .
)";
  auto dag = parseSuccessfully(str);
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

TEST(parse, wigglyHammock) {
  std::string str = R"(
   .
  / \
  \ |
  | \
  \  \
   .  .
  /   |
 /    \
 \     \
 |     |
 \     /
  \   /
   \ /
   |/
   .
)";
  auto dag = parseSuccessfully(str);
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

TEST(parse, wideNodeHammock) {
  std::string str = R"(
   ###
   | |
 ### ###
  \   /
   ###
)";
  auto dag = parseSuccessfully(str);
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

TEST(parseError, danglingPipeEOS) {
  std::string str = R"(
    .
    |)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, danglingPipeEOL) {
  std::string str = R"(
    .
    |
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, danglingPipeEmptyLine) {
  std::string str = R"(
    .
    |

)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, danglingPipeDisconnectedNodes) {
  std::string str = R"(
    .
    |
   . .
    .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, danglingSlash) {
  std::string str = R"(
    .
   /

)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 4U);
}

TEST(parseError, danglingBackslash) {
  std::string str = R"(
    .
     \

)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 6U);
}

TEST(parseError, dangling2Pipes) {
  std::string str = R"(
    .
    |
    |

)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 3U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, danglingPipeWithNondangling) {
  std::string str = R"(
    .
    |\
    ||
    |
    .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 3U);
  EXPECT_EQ(err.col, 6U);
}

TEST(parseError, danglingMissesNodeLeft) {
  std::string str = R"(
   ###
   /
   ###
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 4U);
}

TEST(parseError, danglingMissesNodeRight) {
  std::string str = R"(
   ###
     \
   ###
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 6U);
}

TEST(parseError, suspendedPipeLineStart) {
  std::string str = R"(|
.
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.line, 0U);
  EXPECT_EQ(err.col, 1U);
}

TEST(parseError, suspendedPipeEmptyLine) {
  std::string str = R"(
    |
    .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.line, 1U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, suspendedPipeWithNormalEdge) {
  std::string str = R"(
    .
     \
     |
    |/
    .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.line, 4U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, suspendedSlash) {
  std::string str = R"(
     /
    .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.line, 1U);
  EXPECT_EQ(err.col, 6U);
}

TEST(parseError, suspendedBackslash) {
  std::string str = R"(
   \
    .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.line, 1U);
  EXPECT_EQ(err.col, 4U);
}
