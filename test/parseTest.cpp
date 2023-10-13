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
    auto iter = std::find_if(nodes.begin(), nodes.end(), [&text](DAG::Node const& n) {
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
    auto findFrom = [&text, this](auto fromIter) {
      return std::find_if(fromIter, nodes.end(), [&text](DAG::Node const& n) {
        return n.text == text;
      });
    };
    auto iter = findFrom(nodes.begin());
    EXPECT_NE(iter, nodes.end());
    if (iter == nodes.end()) {
      return {this, 0};
    }
    // Make sure the node is unique
    EXPECT_EQ(findFrom(iter + 1), nodes.end());
    return {this, static_cast<size_t>(iter - nodes.begin())};
  }
};

template<typename... Arg>
std::vector<std::string> nodes(Arg ...args) {
  std::vector<std::string> ret{args...};
  std::sort(ret.begin(), ret.end());
  return ret;
}

DAGWithFunctions parseSuccessfully(std::string_view str) {
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
  EXPECT_EQ(dag.nodes.size(), 0U);
}

TEST(parse, singleNode) {
  std::string str = R"(

    .

)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("."));
  EXPECT_TRUE(dag.node(".").succs().empty());
}

TEST(parse, singleWideNode) {
  std::string str = R"(

    ###

)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("###"));
  EXPECT_TRUE(dag.node("###").succs().empty());
}

TEST(parse, twoDisconnectedNodes) {
  std::string str = R"(
    1 2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_TRUE(dag.node("1").succs().empty());
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, twoWideNodes) {
  std::string str = R"(
    AA BB
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("AA", "BB"));
  EXPECT_TRUE(dag.node("AA").succs().empty());
  EXPECT_TRUE(dag.node("BB").succs().empty());
}

TEST(parse, twoLineNode) {
  std::string str = R"(
     .
     .
)";

  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes(".\n."));
  EXPECT_TRUE(dag.node(".\n.").succs().empty());
}

TEST(parse, nodeSquare2) {
  std::string str = R"(
     12
     34
)";

  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("12\n34"));
  EXPECT_TRUE(dag.node("12\n34").succs().empty());
}

TEST(parse, fourFatNodes) {
  std::string str = R"(
     12 56
     34 78

     ab AB
     cd CD
)";

  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("12\n34", "56\n78", "ab\ncd", "AB\nCD"));
  EXPECT_TRUE(dag.node("12\n34").succs().empty());
  EXPECT_TRUE(dag.node("56\n78").succs().empty());
  EXPECT_TRUE(dag.node("ab\ncd").succs().empty());
  EXPECT_TRUE(dag.node("AB\nCD").succs().empty());
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
  EXPECT_EQ(dag.allNodes(), nodes("12", "34\n56", ".\n.", "AB\nCD", "#"));
  EXPECT_TRUE(dag.node("12").succs().empty());
  EXPECT_TRUE(dag.node("34\n56").succs().empty());
  EXPECT_TRUE(dag.node(".\n.").succs().empty());
  EXPECT_TRUE(dag.node("AB\nCD").succs().empty());
  EXPECT_TRUE(dag.node("#").succs().empty());
}

TEST(parse, twoConnectedNodesPipe) {
  std::string str = R"(
    1
    |
    2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, twoConnectedWideNodesPipeLeft) {
  std::string str = R"(
    1#
    |
    #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedWideNodesPipeRight) {
  std::string str = R"(
    1#
     |
    #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedWideNodesPipeRightSkew) {
  std::string str = R"(
    1#
     |
     #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedWideNodesPipeLeftSkew) {
  std::string str = R"(
     1#
     |
    #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedNodesSlash) {
  std::string str = R"(
     1
    /
   2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, twoConnectedWideNodesSlashRightSkew) {
  std::string str = R"(
    1#
    /
   #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedWideNodesSlashRightDoubleSkewLeft) {
  std::string str = R"(
    1#
   /
  #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedWideNodesSlashRightDoubleSkewRight) {
  std::string str = R"(
    1#
    /
  #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedWideNodesSlashRightTripleSkew) {
  std::string str = R"(
     1#
    /
  #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedNodesBackslash) {
  std::string str = R"(
   1
    \
     2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, twoConnectedWideNodesBackslashRightSkew) {
  std::string str = R"(
   1#
    \
    #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedWideNodesBackslashRightDoubleSkewLeft) {
  std::string str = R"(
  1#
   \
    #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedWideNodesBackslashRightDoubleSkewRight) {
  std::string str = R"(
   1#
    \
    #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, twoConnectedWideNodesBackslashRightTripleSkew) {
  std::string str = R"(
  1#
    \
     #2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1#", "#2"));
  EXPECT_EQ(dag.node("1#").succs(), nodes("#2"));
  EXPECT_TRUE(dag.node("#2").succs().empty());
}

TEST(parse, longEdgePipe) {
  std::string str = R"(
   1
   |
   |
   2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, longSlash) {
  std::string str = R"(
    1
   /
  /
 2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, longEdgeBackslash) {
  std::string str = R"(
   1
    \
     \
      2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, longEdgeSlashPipe) {
  std::string str = R"(
     1
    /
    |
    2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, longEdgeBackslashPipe) {
  std::string str = R"(
   1
    \
    |
    2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, longEdgePipeSlash) {
  std::string str = R"(
     1
     |
     /
    2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, longEdgePipeBackslash) {
  std::string str = R"(
   1
   |
   \
    2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, longEdgeWiggly) {
  std::string str = R"(
   1
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
 2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
}

TEST(parse, twoEdges) {
  std::string str = R"(
   1
  / \
 2   3
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2", "3"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2", "3"));
  EXPECT_TRUE(dag.node("2").succs().empty());
  EXPECT_TRUE(dag.node("3").succs().empty());
}

TEST(parse, hammock) {
  std::string str = R"(
   1
  / \
 2   3
  \ /
   4
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2", "3", "4"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2", "3"));
  EXPECT_EQ(dag.node("2").succs(), nodes("4"));
  EXPECT_EQ(dag.node("3").succs(), nodes("4"));
  EXPECT_TRUE(dag.node("4").succs().empty());
}

TEST(parse, skewedHammock) {
  std::string str = R"(
   1
  / \
 2   3
 |   |
 |   /
 |  /
 | /
 | |
 \ /
  4
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2", "3", "4"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2", "3"));
  EXPECT_EQ(dag.node("2").succs(), nodes("4"));
  EXPECT_EQ(dag.node("3").succs(), nodes("4"));
  EXPECT_TRUE(dag.node("4").succs().empty());
}

TEST(parse, wigglyHammock) {
  std::string str = R"(
   1
  / \
  \ |
  | \
  \  \
   2  3
  /   |
 /    \
 \     \
 |     |
 \     /
  \   /
   \ /
   |/
   4
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2", "3", "4"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2", "3"));
  EXPECT_EQ(dag.node("2").succs(), nodes("4"));
  EXPECT_EQ(dag.node("3").succs(), nodes("4"));
  EXPECT_TRUE(dag.node("4").succs().empty());
}

TEST(parse, wideNodeHammock) {
  std::string str = R"(
   #1#
   | |
 #2# #3#
  \   /
   #4#
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("#1#", "#2#", "#3#", "#4#"));
  EXPECT_EQ(dag.node("#1#").succs(), nodes("#2#", "#3#"));
  EXPECT_EQ(dag.node("#2#").succs(), nodes("#4#"));
  EXPECT_EQ(dag.node("#3#").succs(), nodes("#4#"));
  EXPECT_TRUE(dag.node("#4#").succs().empty());
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.pos, (Position{3U, 5U}));
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.pos, (Position{3U, 6U}));
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
  ASSERT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.pos, (Position{3U, 6U}));
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

TEST(parse, nodeWithOpenHoleBelow) {
  std::string str = R"(
    ###
    # #
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("###\n# #"));
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

TEST(parse, nodeWithClosedHole) {
  std::string str = R"(
    ###
    # #
    ###
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("###\n# #\n###"));
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

TEST(parse, nodeWithEdgeCharMidCenter) {
  std::string str = R"(
    ###
    #\#
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("###\n#\\#"));
}

TEST(parse, nodeWithEdgeCharMidRightPipe) {
  std::string str = R"(
    ###
    ##|
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("###\n##|"));
}

TEST(parse, nodeWithEdgeCharMidRightSlash) {
  std::string str = R"(
    ###
    ##/
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("###\n##/"));
}

TEST(parse, sideEdgeRight1) {
  std::string str = R"(
    #A#
    ###\#B#
        ###
)";
  auto dag = parseSuccessfully(str);
  std::string a = "#A#\n###";
  std::string b = "#B#\n###";
  EXPECT_EQ(dag.allNodes(), nodes(a, b));
  EXPECT_EQ(dag.node(a).succs(), nodes(b));
  EXPECT_TRUE(dag.node(b).succs().empty());
}

TEST(parse, sideEdgeRight2) {
  std::string str = R"(
    ###
    #A#\ ###
    ### \#B#
         ###
)";
  auto dag = parseSuccessfully(str);
  std::string a = "###\n#A#\n###";
  std::string b = "###\n#B#\n###";
  EXPECT_EQ(dag.allNodes(), nodes(a, b));
  EXPECT_EQ(dag.node(a).succs(), nodes(b));
  EXPECT_TRUE(dag.node(b).succs().empty());
}

TEST(parse, sideEdgeLeft1) {
  std::string str = R"(
        #A#
    #B#/###
    ###
)";
  auto dag = parseSuccessfully(str);
  std::string a = "#A#\n###";
  std::string b = "#B#\n###";
  EXPECT_EQ(dag.allNodes(), nodes(a, b));
  EXPECT_EQ(dag.node(a).succs(), nodes(b));
  EXPECT_TRUE(dag.node(b).succs().empty());
}

TEST(parse, sideEdgeLeft2) {
  std::string str = R"(
         ###
    ### /#A#
    #B#/ ###
    ###
)";
  auto dag = parseSuccessfully(str);
  std::string a = "###\n#A#\n###";
  std::string b = "###\n#B#\n###";
  EXPECT_EQ(dag.allNodes(), nodes(a, b));
  EXPECT_EQ(dag.node(a).succs(), nodes(b));
  EXPECT_TRUE(dag.node(b).succs().empty());
}

TEST(parse, sideEdgePipe) {
  std::string str = R"(
       1
       |A##
    B##|###
    ###|
       2
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2", "A##\n###", "B##\n###"));
  EXPECT_EQ(dag.node("1").succs(), nodes("2"));
  EXPECT_TRUE(dag.node("2").succs().empty());
  EXPECT_TRUE(dag.node("A##\n###").succs().empty());
  EXPECT_TRUE(dag.node("B##\n###").succs().empty());
}

TEST(parse, sideEdgePipes) {
  std::string str = R"(
    #1#
    | |
    |2|
    | |
    #3#
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("#1#", "2", "#3#"));
  EXPECT_EQ(dag.node("#1#").succs(), nodes("#3#", "#3#"));
  EXPECT_TRUE(dag.node("2").succs().empty());
  EXPECT_TRUE(dag.node("#3#").succs().empty());
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
  std::string first = "###\n###\n###\n###";
  std::string second = "###\n###\n###";
  EXPECT_EQ(dag.allNodes(), nodes(first, second));
  EXPECT_EQ(dag.node(first).succs(), nodes(second));
  EXPECT_TRUE(dag.node(second).succs().empty());
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
  EXPECT_EQ(dag.allNodes(), nodes("AAA", "B", "CCC"));
  EXPECT_EQ(dag.node("AAA").succs(), nodes("B", "CCC"));
  EXPECT_EQ(dag.node("B").succs(), nodes("CCC"));
  EXPECT_TRUE(dag.node("CCC").succs().empty());
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
  EXPECT_EQ(dag.allNodes(), nodes("AAA", "B", "CCC"));
  EXPECT_EQ(dag.node("AAA").succs(), nodes("B", "CCC"));
  EXPECT_EQ(dag.node("B").succs(), nodes("CCC"));
  EXPECT_TRUE(dag.node("CCC").succs().empty());
}

TEST(parse, edgeUndercuttingNodeRight) {
  std::string str = R"(
   1
    \2
     \
      3
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2", "3"));
  EXPECT_EQ(dag.node("1").succs(), nodes("3"));
  EXPECT_TRUE(dag.node("2").succs().empty());
  EXPECT_TRUE(dag.node("3").succs().empty());
}

TEST(parse, edgeUndercuttingNodeLeft) {
  std::string str = R"(
       1
     2/
     /
    3
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.allNodes(), nodes("1", "2", "3"));
  EXPECT_EQ(dag.node("1").succs(), nodes("3"));
  EXPECT_TRUE(dag.node("2").succs().empty());
  EXPECT_TRUE(dag.node("3").succs().empty());
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
  std::string bigNode = "###\n###\n###\n###\n###";
  EXPECT_EQ(dag.allNodes(), nodes(bigNode, "###\n###", "###"));
  EXPECT_EQ(dag.node(bigNode).succs(), nodes("###"));
  EXPECT_EQ(dag.node("###\n###").succs(), nodes("###"));
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
  std::string longNode = "#\n#\n#\n#\n#\n#\n#";
  std::string largeNode = "###\n###";
  EXPECT_EQ(dag.allNodes(), nodes(longNode, largeNode, "###"));
  EXPECT_EQ(dag.node(longNode).succs(), nodes(largeNode, largeNode, largeNode, largeNode, "###", "###"));
  EXPECT_EQ(dag.node(largeNode).succs(), nodes("###"));
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
  // Cannot use "parseSuccessfully" here because it does not admit self-loops
  auto dagOpt = parseDAG(str, err);
  EXPECT_EQ(err.code, ParseError::Code::None);
  ASSERT_TRUE(dagOpt.has_value());
  auto dag = DAGWithFunctions{*dagOpt};
  std::string node = "##\n##\n##\n##\n##\n##\n##";
  ASSERT_EQ(dag.allNodes(), nodes(node));
  ASSERT_EQ(dag.node(node).succs(), nodes(node, node));
}

TEST(parse, touchRight) {
  std::string str = R"(
   A B
    \|
    |\
    C D
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.node("A").succs(), nodes("C"));
  EXPECT_EQ(dag.node("B").succs(), nodes("D"));
  EXPECT_EQ(dag.node("C").succs(), nodes());
  EXPECT_EQ(dag.node("D").succs(), nodes());
}

TEST(parse, touchLeftStraght) {
  std::string str = R"(
   A B
   |/
   /|
  C D
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.node("A").succs(), nodes("C"));
  EXPECT_EQ(dag.node("B").succs(), nodes("D"));
  EXPECT_EQ(dag.node("C").succs(), nodes());
  EXPECT_EQ(dag.node("D").succs(), nodes());
}

TEST(parse, touchLeftRight) {
  std::string str = R"(
  A  B
   \/
   /|
  C D
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.node("A").succs(), nodes("C"));
  EXPECT_EQ(dag.node("B").succs(), nodes("D"));
  EXPECT_EQ(dag.node("C").succs(), nodes());
  EXPECT_EQ(dag.node("D").succs(), nodes());
}

TEST(parse, touchRightLeft) {
  std::string str = R"(
  A  B
   \/
   |\
   C D
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.node("A").succs(), nodes("C"));
  EXPECT_EQ(dag.node("B").succs(), nodes("D"));
  EXPECT_EQ(dag.node("C").succs(), nodes());
  EXPECT_EQ(dag.node("D").succs(), nodes());
}

TEST(parse, touchCross) {
  std::string str = R"(
  A  B
   \/
   /\
  C  D
)";
  auto dag = parseSuccessfully(str);
  EXPECT_EQ(dag.node("A").succs(), nodes("C"));
  EXPECT_EQ(dag.node("B").succs(), nodes("D"));
  EXPECT_EQ(dag.node("C").succs(), nodes());
  EXPECT_EQ(dag.node("D").succs(), nodes());
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
  ASSERT_EQ(dag.allNodes(), nodes("A", "B", "C", "D"));
  ASSERT_EQ(dag.node("A").succs(), nodes("D"));
  ASSERT_EQ(dag.node("B").succs(), nodes("C"));
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
  ASSERT_EQ(dag.allNodes(), nodes("A", "B", "C", "D", "X#"));
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
  ASSERT_EQ(dag.allNodes(), nodes("A", "B", "C", "D", "#X"));
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
  ASSERT_EQ(dag.allNodes(), nodes("A", "B", "C", "D", "X\n#"));
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
  ASSERT_EQ(dag.allNodes(), nodes("A", "B", "C", "D", "#\nX"));
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

TEST(parse, skewedCrossLeft) {
  std::string str = R"(
      A B
      |/
      X
     /|
    C D
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.allNodes(), nodes("A", "B", "C", "D"));
  ASSERT_EQ(dag.node("A").succs(), nodes("D"));
  ASSERT_EQ(dag.node("B").succs(), nodes("C"));
  ASSERT_EQ(dag.node("C").succs(), nodes());
  ASSERT_EQ(dag.node("D").succs(), nodes());
}

TEST(parse, skewedCrossRight) {
  std::string str = R"(
    A B
     \|
      X
      |\
      C D
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.allNodes(), nodes("A", "B", "C", "D"));
  ASSERT_EQ(dag.node("A").succs(), nodes("D"));
  ASSERT_EQ(dag.node("B").succs(), nodes("C"));
  ASSERT_EQ(dag.node("C").succs(), nodes());
  ASSERT_EQ(dag.node("D").succs(), nodes());
}

TEST(parse, tripleCross) {
  std::string str = R"(
    A B C
     \|/
      X
     /|\
    D E F
)";
  auto dag = parseSuccessfully(str);
  ASSERT_EQ(dag.allNodes(), nodes("A", "B", "C", "D", "E", "F"));
  ASSERT_EQ(dag.node("A").succs(), nodes("F"));
  ASSERT_EQ(dag.node("B").succs(), nodes("E"));
  ASSERT_EQ(dag.node("C").succs(), nodes("D"));
  ASSERT_EQ(dag.node("D").succs(), nodes());
  ASSERT_EQ(dag.node("E").succs(), nodes());
  ASSERT_EQ(dag.node("F").succs(), nodes());
}

TEST(parseError, tripleCrossMissingLowerEdge) {
  std::string str = R"(
    A B C
     \|/
      X
     /|
    D E
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::DanglingEdge);
  EXPECT_EQ(err.pos, (Position{3U, 8U}));
}

TEST(parseError, tripleCrossMissingUpperEdge) {
  std::string str = R"(
    A B
     \|
      X
     /|\
    D E F
)";
  ParseError err;
  auto result = parseDAG(str, err);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(err.code, ParseError::Code::SuspendedEdge);
  EXPECT_EQ(err.pos, (Position{3U, 8U}));
}

//TODO:
//  test for 2 consequitive crossings:
//  \ /  |
//   X   /
//  / \ /
//  |  X
//  | / \

// TODO: hemed in
// 1
//  \2
//  4\
//    3

// TODO: ergonomic way to specify the Position
