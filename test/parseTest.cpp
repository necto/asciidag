#include "asciidag.h"

#include <gtest/gtest.h>
#include <string>

using namespace asciidag;

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

void checkValidEdges(DAG const& dag) {
  if (dag.nodes.empty()) {
    return;
  }

  size_t maxId = dag.nodes.size() - 1;

  for (size_t id = 0; id <= maxId; ++id) {
    for (auto const& e : dag.nodes[id].succs) {
      EXPECT_LE(e, maxId);
      EXPECT_LT(id, e);
    }
  }
}

class NodeInfo {
public:
    NodeInfo(DAG const* dag, size_t id) : dag(dag), id(id) {}

    std::vector<std::string> succs() const {
      std::vector<std::string> ret;
      for (auto const& e : dag->nodes[id].succs) {
        std::string const& text = dag->nodes[e].text;
        ret.push_back(text);
      }
      std::sort(ret.begin(), ret.end());
      return ret;
    }

private:
  DAG const* dag;
  size_t id;
};

class DAGWithFunctions : public DAG {
public:
  bool hasNode(std::string const& text) const {
    auto iter = std::find_if(nodes.begin(), nodes.end(), [text](DAG::Node const& n) {
      return n.text == text;
    });
    return iter != nodes.end();
  }

  std::vector<std::string> allNodes() const {
    std::vector<std::string> ret;
    for (auto const& n : nodes) {
      std::string const& text = n.text;
      ret.push_back(text);
    }
    std::sort(ret.begin(), ret.end());
    return ret;
  }

  NodeInfo node(std::string const& text) const {
    auto iter = std::find_if(nodes.begin(), nodes.end(), [text](DAG::Node const& n) {
      return n.text == text;
    });
    EXPECT_NE(iter, nodes.end());
    if (iter == nodes.end()) {
      return {this, 0};
    }
    return {this, static_cast<size_t>(iter - nodes.begin())};
  }
};

template<typename... Arg>
std::vector<std::string> nodes(Arg ...args) {
  std::vector<std::string> ret{args...};
  std::sort(ret.begin(), ret.end());
  return ret;
}

DAGWithFunctions parseSuccessfully(std::string str) {
  ParseError err;
  auto dag = parseDAG(str, err);
  EXPECT_EQ(err.code, ParseError::Code::None);
  if (err.code != ParseError::Code::None) {
    // Print error message by violating an assertion
    EXPECT_EQ(err.message, "");
    // Print error location by violating an assertion
    EXPECT_EQ(err.pos, (Position{0, 0}));
  }
  EXPECT_TRUE(dag.has_value());
  if (dag) {
    checkRectangularNodes(*dag);
    checkValidEdges(*dag);
    return {*dag};
  }
  return DAGWithFunctions{};
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
  EXPECT_EQ(dag.nodes[0].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[0].text, ".");
}

TEST(parse, singleWideNode) {
  std::string str = R"(

    ###

)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[0].text, "###");
}

TEST(parse, twoDisconnectedNodes) {
  std::string str = R"(
    . .
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoWideNodes) {
  std::string str = R"(
    AA BB
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  EXPECT_TRUE(dag.nodes[0].succs.empty());
}

TEST(parse, nodeSquare2) {
  std::string str = R"(
     12
     34
)";

  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 1);
  EXPECT_EQ(dag.nodes[0].text, "12\n34");
  EXPECT_TRUE(dag.nodes[0].succs.empty());
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
  EXPECT_TRUE(dag.nodes[0].succs.empty());
  EXPECT_EQ(dag.nodes[1].text, "56\n78");
  EXPECT_TRUE(dag.nodes[1].succs.empty());
  EXPECT_EQ(dag.nodes[2].text, "ab\ncd");
  EXPECT_TRUE(dag.nodes[2].succs.empty());
  EXPECT_EQ(dag.nodes[3].text, "AB\nCD");
  EXPECT_TRUE(dag.nodes[3].succs.empty());
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
  EXPECT_TRUE(dag.nodes[0].succs.empty());
  EXPECT_EQ(dag.nodes[1].text, "12");
  EXPECT_TRUE(dag.nodes[1].succs.empty());
  EXPECT_EQ(dag.nodes[2].text, "AB\nCD");
  EXPECT_TRUE(dag.nodes[2].succs.empty());
  EXPECT_EQ(dag.nodes[3].text, ".\n.");
  EXPECT_TRUE(dag.nodes[3].succs.empty());
  EXPECT_EQ(dag.nodes[4].text, "#");
  EXPECT_TRUE(dag.nodes[4].succs.empty());
}

TEST(parse, twoConnectedNodesPipe) {
  std::string str = R"(
    .
    |
    .
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesPipeLeft) {
  std::string str = R"(
    ##
    |
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesPipeRight) {
  std::string str = R"(
    ##
     |
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesPipeRightSkew) {
  std::string str = R"(
    ##
     |
     ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesPipeLeftSkew) {
  std::string str = R"(
     ##
     |
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedNodesSlash) {
  std::string str = R"(
     .
    /
   .
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesSlashRightSkew) {
  std::string str = R"(
    ##
    /
   ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesSlashRightDoubleSkewLeft) {
  std::string str = R"(
    ##
   /
  ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesSlashRightDoubleSkewRight) {
  std::string str = R"(
    ##
    /
  ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesSlashRightTripleSkew) {
  std::string str = R"(
     ##
    /
  ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedNodesBackslash) {
  std::string str = R"(
   .
    \
     .
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesBackslashRightSkew) {
  std::string str = R"(
   ##
    \
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesBackslashRightDoubleSkewLeft) {
  std::string str = R"(
  ##
   \
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesBackslashRightDoubleSkewRight) {
  std::string str = R"(
   ##
    \
    ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoConnectedWideNodesBackslashRightTripleSkew) {
  std::string str = R"(
  ##
    \
     ##
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
}

TEST(parse, twoEdges) {
  std::string str = R"(
   .
  / \
 .   .
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 3U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].succs[1], 2U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[2].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].succs[1], 2U);
  ASSERT_EQ(dag.nodes[1].succs.size(), 1U);
  ASSERT_EQ(dag.nodes[2].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[1].succs[0], 3U);
  EXPECT_EQ(dag.nodes[2].succs[0], 3U);
  EXPECT_EQ(dag.nodes[3].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].succs[1], 2U);
  ASSERT_EQ(dag.nodes[1].succs.size(), 1U);
  ASSERT_EQ(dag.nodes[2].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[1].succs[0], 3U);
  EXPECT_EQ(dag.nodes[2].succs[0], 3U);
  EXPECT_EQ(dag.nodes[3].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].succs[1], 2U);
  ASSERT_EQ(dag.nodes[1].succs.size(), 1U);
  ASSERT_EQ(dag.nodes[2].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[1].succs[0], 3U);
  EXPECT_EQ(dag.nodes[2].succs[0], 3U);
  EXPECT_EQ(dag.nodes[3].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].succs[1], 2U);
  ASSERT_EQ(dag.nodes[1].succs.size(), 1U);
  ASSERT_EQ(dag.nodes[2].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[1].succs[0], 3U);
  EXPECT_EQ(dag.nodes[2].succs[0], 3U);
  EXPECT_EQ(dag.nodes[3].succs.size(), 0U);
}

TEST(parseError, danglingPipeEOS) {
  std::string str = R"(
    .
    |)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 4U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 6U}));
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
  EXPECT_EQ(err.pos, (Position{3U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{3U, 6U}));
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
  EXPECT_EQ(err.pos, (Position{3U, 6U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 4U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 6U}));
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
  EXPECT_EQ(err.pos, (Position{3U, 6U}));
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
  EXPECT_EQ(err.pos, (Position{3U, 5U}));
}

TEST(parseError, suspendedPipeLineStart) {
  std::string str = R"(|
.
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.pos, (Position{0U, 1U}));
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
  EXPECT_EQ(err.pos, (Position{1U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{4U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{1U, 6U}));
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
  EXPECT_EQ(err.pos, (Position{1U, 4U}));
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
  EXPECT_EQ(err.pos, (Position{4U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{4U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{4U, 6U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 7U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 7U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 4U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 4U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 7U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 6U}));
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
  EXPECT_TRUE(result->nodes[0].succs.empty());
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
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 7U}));
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
  EXPECT_TRUE(result->nodes[0].succs.empty());
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
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_EQ(err.pos, (Position{2U, 5U}));
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
  EXPECT_TRUE(result->nodes[0].succs.empty());
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
  EXPECT_TRUE(result->nodes[0].succs.empty());
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
  EXPECT_TRUE(result->nodes[0].succs.empty());
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].text, "###\n###");
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].text, "###\n###\n###");
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].text, "###\n###");
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].text, "###\n###\n###");
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
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
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 3U);
  EXPECT_EQ(dag.nodes[0].text, ".");
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[1].text, "###\n###");
  EXPECT_EQ(dag.nodes[2].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[2].text, "###\n###");
  ASSERT_EQ(dag.nodes[3].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[3].text, ".");
}

TEST(parse, sideEdgePipes) {
  std::string str = R"(
    ###
    | |
    |#|
    | |
    ###
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 3U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs[0], 2U);
  EXPECT_EQ(dag.nodes[0].succs[1], 2U);
  EXPECT_EQ(dag.nodes[0].text, "###");
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[1].text, "#");
  ASSERT_EQ(dag.nodes[2].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[2].text, "###");
}

TEST(parse, sideEdgeSquiggle) {
  std::string str = R"(
       ###
      /###
   ###\###
   ###/###
   ###
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 2U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].text, "###\n###\n###\n###");
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[1].text, "###\n###\n###");
}

TEST(parse, sideEdgeTurnAwayFromNodeLeft) {
  std::string str = R"(
    AAA
     ||
     |B
     /|
    CCC
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 3U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].succs[1], 2U);
  EXPECT_EQ(dag.nodes[0].text, "AAA");
  EXPECT_EQ(dag.nodes[1].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[1].succs[0], 2U);
  EXPECT_EQ(dag.nodes[1].text, "B");
  EXPECT_EQ(dag.nodes[2].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[2].text, "CCC");
}

TEST(parse, sideEdgeTurnAwayFromNodeRight) {
  std::string str = R"(
     AAA
     ||
     B|
     |\
     CCC
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 3U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 2U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].succs[1], 2U);
  EXPECT_EQ(dag.nodes[0].text, "AAA");
  EXPECT_EQ(dag.nodes[1].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[1].succs[0], 2U);
  EXPECT_EQ(dag.nodes[1].text, "B");
  EXPECT_EQ(dag.nodes[2].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[2].text, "CCC");
}

TEST(parse, edgeUndercuttingNodeRight) {
  std::string str = R"(
   1
    \2
     \
      3
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 3U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 2U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[2].succs.size(), 0U);
}

TEST(parse, edgeUndercuttingNodeLeft) {
  std::string str = R"(
       1
     2/
     /
    3
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 3U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 2U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 0U);
  EXPECT_EQ(dag.nodes[2].succs.size(), 0U);
}

TEST(parse, sideEdgeSquiglyPipe) {
  std::string str = R"(
     ###
     ###\
     ###|###
     ###/###
     ###\|
         ###
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 3U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 2U);
  EXPECT_EQ(dag.nodes[1].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[1].succs[0], 2U);
  EXPECT_EQ(dag.nodes[2].succs.size(), 0U);
}

TEST(parse, parallelSideEdges) {
  std::string str = R"(
     #
     #\
     #\\
     #\\\
     #\###
     #\###
     #\\|
       ###
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 3U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 6U);
  EXPECT_EQ(dag.nodes[0].succs[0], 1U);
  EXPECT_EQ(dag.nodes[0].succs[1], 1U);
  EXPECT_EQ(dag.nodes[0].succs[2], 1U);
  EXPECT_EQ(dag.nodes[0].succs[3], 1U);
  EXPECT_EQ(dag.nodes[0].succs[4], 2U);
  EXPECT_EQ(dag.nodes[0].succs[5], 2U);
  ASSERT_EQ(dag.nodes[1].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[1].succs[0], 2U);
  EXPECT_EQ(dag.nodes[2].succs.size(), 0U);
}

TEST(parse, selfLoop) {
  // This is not a DAG because it includes a self-loop
  // the parsing algorithm accepts it even though it does not have to
  std::string str = R"(
    ##
    ##\
    ##\\
    ##/|
    ## /
    ##/
    ##
)";
  ParseError err;
  // Cannot use "parseSuccessfully" here because it does not admin self-loops
  auto dag = parseDAG(str, err);
  EXPECT_EQ(err.code, ParseError::Code::None);
  ASSERT_TRUE(dag.has_value());
  ASSERT_EQ(dag->nodes.size(), 1U);
  ASSERT_EQ(dag->nodes[0].succs.size(), 2U);
  EXPECT_EQ(dag->nodes[0].succs[0], 0U);
  EXPECT_EQ(dag->nodes[0].succs[1], 0U);
}

TEST(parse, simpleEdgeCross) {
  std::string str = R"(
    A   B
     \ /
      X
     / \
    C   D
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 4U);
  ASSERT_EQ(dag.nodes[0].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[0].succs[0], 3U);
  EXPECT_EQ(dag.nodes[0].text, "A");
  EXPECT_EQ(dag.nodes[3].text, "D");

  EXPECT_EQ(dag.nodes[1].succs.size(), 1U);
  EXPECT_EQ(dag.nodes[1].succs[0], 2U);
  EXPECT_EQ(dag.nodes[1].text, "B");
  EXPECT_EQ(dag.nodes[2].text, "C");
}

TEST(parseError, crossMissingLeftBottomEdge) {
  std::string str = R"(
    A   B
     \ /
      X
       \
        D
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.pos, (Position{3U, 8U}));
}

TEST(parseError, crossMissingRightBottomEdge) {
  std::string str = R"(
    A   B
     \ /
      X
     /
    C   D
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.pos, (Position{3U, 8U}));
}

TEST(parseError, crossMissingBothBottomEdges) {
  std::string str = R"(
    A   B
     \ /
      X
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.pos, (Position{3U, 8U}));
}

TEST(parseError, crossMissingLeftTopEdge) {
  std::string str = R"(
    A   B
       /
      X
     / \
    C   D
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.pos, (Position{3U, 8U}));
}

TEST(parseError, crossMissingRightTopEdge) {
  std::string str = R"(
    A
     \
      X
     / \
    C   D
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.pos, (Position{3U, 8U}));
}

TEST(parseError, crossMissingBothTopEdges) {
  std::string str = R"(
      X
     / \
    A   B
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.pos, (Position{1U, 8U}));
}

TEST(parseError, standaloneX) {
  std::string str = R"(
      X
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  ASSERT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.pos, (Position{1U, 8U}));
}

TEST(parse, standaloneXX) {
  std::string str = R"(
      XX
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.allNodes(), nodes("XX"));
}

TEST(parse, crossAdjacentToNodeLeft) {
  // When adjacent to a node, 'X' is part of the node
  // and it does not represent edge crossing.
  std::string str = R"(
    A   B
     \ /
      X#
     / \
    C   D
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 5U);
  EXPECT_TRUE(dag.hasNode("X#"));
}

TEST(parse, crossAdjacentToNodeRight) {
  // When adjacent to a node, 'X' is part of the node
  // and it does not represent edge crossing.
  std::string str = R"(
    A   B
     \ /
     #X
     / \
    C   D
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 5U);
  EXPECT_TRUE(dag.hasNode("#X"));
}

TEST(parse, crossAdjacentToNodeTop) {
  // When adjacent to a node, 'X' is part of the node
  // and it does not represent edge crossing.
  std::string str = R"(
    A   B
     \ /
      X
     /#\
    C   D
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 5U);
  EXPECT_TRUE(dag.hasNode("X\n#"));
}

TEST(parse, crossAdjacentToNodeBottom) {
  // When adjacent to a node, 'X' is part of the node
  // and it does not represent edge crossing.
  std::string str = R"(
    A   B
     \#/
      X
     / \
    C   D
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.nodes.size(), 5U);
  EXPECT_TRUE(dag.hasNode("#\nX"));
}

TEST(parse, doubleXIsNotCrossing) {
  // "XX" is a regular node and not a double-edge crossing
  std::string str = R"(
    A B C  D
    | | | /
    \ | //
     \\//
      XX
     //\\
    ######
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.allNodes(), nodes("A", "B", "C", "D", "XX", "######"));
}

// TODO:
// test for skewed cross:
//   |/
//   X
//  /|
//
//  \|
//   X
//   |\
//
//TODO:
//  test for triple cross:
//  \|/
//   X
//  /|\
