#include "asciidag.h"

#include <gtest/gtest.h>
#include <string>

void checkRectangularString(std::string const& s) {
  size_t lastLine = 0;
  size_t lineLen = 0;
  for (size_t pos = 0; pos < s.size(); ++pos) {
    if (s[pos] == '\n') {
      size_t curLineLen = pos - lastLine;
      if (lastLine != 0) {
        EXPECT_EQ(curLineLen, lineLen);
      }
      lineLen = curLineLen;
      lastLine = pos + 1;
    }
  }
  EXPECT_NE(s.back(), '\n');
  if (lastLine != 0) {
    EXPECT_EQ(s.size() - lastLine, lineLen);
  }
}

void checkRectangularNodes(DAG const& dag) {
  for (auto const& n : dag.nodes) {
    checkRectangularString(n.text);
  }
}

DAG parseSuccessfully(std::string str) {
  ParseError err;
  auto dag = parseDAG(str, err);
  EXPECT_EQ(err.code, ParseError::Code::None);
  if (err.code != ParseError::Code::None) {
    // Print error message by violating an assertion
    EXPECT_EQ(err.message, "");
  }
  EXPECT_TRUE(dag.has_value());
  if (dag) {
    checkRectangularNodes(*dag);
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
  EXPECT_EQ(dag.nodes[0].text, ".");
}

TEST(parse, singleWideNode) {
  std::string str = R"(

    ###

)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[0].text, "###");
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
  EXPECT_EQ(dag.nodes[0].text, "AA");
  EXPECT_EQ(dag.nodes[1].text, "BB");
}

TEST(parse, twoLineNode) {
  std::string str = R"(
     .
     .
)";

  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 1);
  EXPECT_EQ(dag.nodes[0].text, ".\n.");
  EXPECT_TRUE(dag.nodes[0].outEdges.empty());
}

TEST(parse, nodeSquare2) {
  std::string str = R"(
     12
     34
)";

  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 1);
  EXPECT_EQ(dag.nodes[0].text, "12\n34");
  EXPECT_TRUE(dag.nodes[0].outEdges.empty());
}

TEST(parse, fourFatNodes) {
  std::string str = R"(
     12 56
     34 78

     ab AB
     cd CD
)";

  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 4);
  EXPECT_EQ(dag.nodes[0].text, "12\n34");
  EXPECT_TRUE(dag.nodes[0].outEdges.empty());
  EXPECT_EQ(dag.nodes[1].text, "56\n78");
  EXPECT_TRUE(dag.nodes[1].outEdges.empty());
  EXPECT_EQ(dag.nodes[2].text, "ab\ncd");
  EXPECT_TRUE(dag.nodes[2].outEdges.empty());
  EXPECT_EQ(dag.nodes[3].text, "AB\nCD");
  EXPECT_TRUE(dag.nodes[3].outEdges.empty());
}

TEST(parse, checkered) {
  std::string str = R"(
         34
     12  56
       AB
       CD
      .  #
      .
)";

  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 5);
  EXPECT_EQ(dag.nodes[0].text, "34\n56");
  EXPECT_TRUE(dag.nodes[0].outEdges.empty());
  EXPECT_EQ(dag.nodes[1].text, "12");
  EXPECT_TRUE(dag.nodes[1].outEdges.empty());
  EXPECT_EQ(dag.nodes[2].text, "AB\nCD");
  EXPECT_TRUE(dag.nodes[2].outEdges.empty());
  EXPECT_EQ(dag.nodes[3].text, ".\n.");
  EXPECT_TRUE(dag.nodes[3].outEdges.empty());
  EXPECT_EQ(dag.nodes[4].text, "#");
  EXPECT_TRUE(dag.nodes[4].outEdges.empty());
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 3U);
  EXPECT_EQ(err.col, 6U);
}

TEST(parseError, danglingPseudoMerge) {
  std::string str = R"(
    .
    |\
    |/
    |
    .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 6U);
}

TEST(parseError, danglingMissLeft) {
  std::string str = R"(
    .
    |\
    \/
     .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 3U);
  EXPECT_EQ(err.col, 6U);
}

TEST(parseError, danglingMissRight) {
  std::string str = R"(
    .
    |\
    \/
    .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.line, 3U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, suspendedPipeLineStart) {
  std::string str = R"(|
.
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
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
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.line, 1U);
  EXPECT_EQ(err.col, 4U);
}

TEST(parseError, mergingEdgeLeft) {
  std::string str = R"(
    .
   /|
   \|
    \
     .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::MergingEdge);
  EXPECT_EQ(err.line, 4U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, mergingEdgeRightSkewed) {
  std::string str = R"(
    .
    |\
    |/
    /
   .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::MergingEdge);
  EXPECT_EQ(err.line, 4U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, mergingEdgeLeftStraight) {
  std::string str = R"(
    .
    |\
    \/
     \
      .
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::MergingEdge);
  EXPECT_EQ(err.line, 4U);
  EXPECT_EQ(err.col, 6U);
}

TEST(parseError, nodeShiftedLeft) {
  std::string str = R"(
    ##
   ##
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, nodeShiftedRight) {
  std::string str = R"(
    ##
     ##
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 7U);
}

TEST(parseError, nodeShortLongMiddle) {
  std::string str = R"(
    ##
   ####
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, nodeShortLongLeft) {
  std::string str = R"(
    ##
   ###
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, nodeShortLongRight) {
  std::string str = R"(
    ##
    ###
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 7U);
}

TEST(parseError, nodeLongShortMiddle) {
  std::string str = R"(
   ####
    ##
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 4U);
}

TEST(parseError, nodeLongShortLeft) {
  std::string str = R"(
   ###
    ##
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 4U);
}

TEST(parseError, nodeLongShortRight) {
  std::string str = R"(
    ###
    ##
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 7U);
}

TEST(parseError, nodeWithOpenHoleAbove) {
  std::string str = R"(
    # #
    ###
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 6U);
}

TEST(parseError, nodeWithOpenHoleBelow) {
  std::string str = R"(
    ###
    # #
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_TRUE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::None);
  ASSERT_EQ(result->nodes.size(), 1U);
  EXPECT_TRUE(result->nodes[0].outEdges.empty());
  EXPECT_EQ(result->nodes[0].text, "###\n# #");
}

TEST(parseError, nodeWithOpenHoleLeft) {
  std::string str = R"(
    ###
     ##
    ###
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, nodeWithOpenHoleRight) {
  std::string str = R"(
    ###
    ##
    ###
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 7U);
}

TEST(parseError, nodeWithClosedHoleFalseAlarm) {
  std::string str = R"(
    ###
    # #
    ###
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_TRUE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::None);
  ASSERT_EQ(result->nodes.size(), 1U);
  EXPECT_TRUE(result->nodes[0].outEdges.empty());
  EXPECT_EQ(result->nodes[0].text, "###\n# #\n###");
}

TEST(parseError, nodeWithEdgeCharMidLeftNonrecNode) {
  std::string str = R"(
    ###
    /##
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::NonRectangularNode);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, nodeWithEdgeCharMidLeftSuspendedEdge) {
  std::string str = R"(
    ###
    \##
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.line, 2U);
  EXPECT_EQ(err.col, 5U);
}

TEST(parseError, nodeWithEdgeCharMidCenter) {
  std::string str = R"(
    ###
    #\#
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_TRUE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::None);
  ASSERT_EQ(result->nodes.size(), 1U);
  EXPECT_TRUE(result->nodes[0].outEdges.empty());
  EXPECT_EQ(result->nodes[0].text, "###\n#\\#");
}

TEST(parseError, nodeWithEdgeCharMidRightPipe) {
  std::string str = R"(
    ###
    ##|
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_TRUE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::None);
  ASSERT_EQ(result->nodes.size(), 1U);
  EXPECT_TRUE(result->nodes[0].outEdges.empty());
  EXPECT_EQ(result->nodes[0].text, "###\n##|");
}

TEST(parseError, nodeWithEdgeCharMidRightSlash) {
  std::string str = R"(
    ###
    ##/
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_TRUE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::None);
  ASSERT_EQ(result->nodes.size(), 1U);
  EXPECT_TRUE(result->nodes[0].outEdges.empty());
  EXPECT_EQ(result->nodes[0].text, "###\n##/");
}

TEST(parse, sideEdgeRight1) {
  std::string str = R"(
    ###
    ###\###
        ###
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[0].text, "###\n###");
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[1].text, "###\n###");
}

TEST(parse, sideEdgeRight2) {
  std::string str = R"(
    ###
    ###\ ###
    ### \###
         ###
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[0].text, "###\n###\n###");
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[1].text, "###\n###\n###");
}

TEST(parse, sideEdgeLeft1) {
  std::string str = R"(
        ###
    ###/###
    ###
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[0].text, "###\n###");
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[1].text, "###\n###");
}

TEST(parse, sideEdgeLeft2) {
  std::string str = R"(
         ###
    ### /###
    ###/ ###
    ###
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 1U);
  EXPECT_EQ(dag.nodes[0].text, "###\n###\n###");
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[1].text, "###\n###\n###");
}

TEST(parse, sideEdgePipe) {
  std::string str = R"(
       .
       |###
    ###|###
    ###|
       .
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 4U);
  ASSERT_EQ(dag.nodes[0].outEdges.size(), 1U);
  EXPECT_EQ(dag.nodes[0].outEdges[0].to, 3U);
  EXPECT_EQ(dag.nodes[0].text, ".");
  EXPECT_EQ(dag.nodes[1].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[1].text, "###\n###");
  EXPECT_EQ(dag.nodes[2].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[2].text, "###\n###");
  ASSERT_EQ(dag.nodes[3].outEdges.size(), 0U);
  EXPECT_EQ(dag.nodes[3].text, ".");
}

// TODO: test for node with edges starting and finishing on a side
//
//    right2
//
//    ###
//    ###\
//    ### \###
//         ###
//         ###
//
//  same for left2
//
//   ###
//   | |
//   |#|
//   | |
//   ###
