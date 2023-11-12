#include "testUtils.h"

#include <gtest/gtest.h>

namespace {

using namespace asciidag;

DAG canonicalDAG(DAG const& orig) {
  DAG ret = orig;
  std::sort(ret.nodes.begin(), ret.nodes.end(), [](auto const& a, auto const& b) {
    return a.text < b.text;
  });
  auto equal = std::adjacent_find(ret.nodes.begin(), ret.nodes.end(), [](auto const& a, auto const& b) {
    return a.text == b.text;
  });
  assert(equal == ret.nodes.end() && "Cannot compare graphs with similar nodes.");
  std::vector<size_t> idMap(orig.nodes.size());
  for (size_t origId = 0; origId < orig.nodes.size(); ++origId) {
    size_t newId = std::
      distance(ret.nodes.begin(), std::find_if(ret.nodes.begin(), ret.nodes.end(), [&](auto const& n) {
                 return n.text == orig.nodes[origId].text;
               }));
    idMap[origId] = newId;
  }
  for (auto &node : ret.nodes) {
    for (size_t &succ : node.succs) {
      succ = idMap[succ];
    }
    std::sort(node.succs.begin(), node.succs.end());
  }
  return ret;
}

bool compareDAGs(DAG const& a, DAG const& b) {
  if (a.nodes.size() != b.nodes.size()) {
    return false;
  }
  for (size_t i = 0; i < a.nodes.size(); ++i) {
    auto const& nodeA = a.nodes[i];
    auto const& nodeB = b.nodes[i];
    if (nodeA.succs.size() != nodeB.succs.size()) {
      return false;
    }
    if (nodeA.text != nodeB.text) {
      return false;
    }
    for (size_t j = 0; j < nodeA.succs.size(); ++j) {
      if (nodeA.succs[j] != nodeB.succs[j]) {
        return false;
      }
    }
  }
  return true;
}

void assertEqual(DAG const& a, DAG const& b) {
  DAG canonA = canonicalDAG(a);
  DAG canonB = canonicalDAG(b);
  if (!compareDAGs(canonA, canonB)) {
    RenderError err;
    std::string rendering = renderDAG(a, err).value_or("");
    GTEST_FAIL(
    ) << "Graph \n"
      << rendering << " was transformed from " <<toDOT(canonA) << " to " << toDOT(canonB);
  }
}

} // namespace

namespace asciidag::tests {

std::string parseAndRender(std::string_view str) {
  ParseError parseErr;
  auto dag = parseDAG(str, parseErr);
  EXPECT_EQ(parseErr.code, ParseError::Code::None);
  if (parseErr.code != ParseError::Code::None) {
    // Print error message by violating an assertion
    EXPECT_EQ(parseErr.message, "");
    // Print error location by violating an assertion
    EXPECT_EQ(parseErr.pos, (Position{0, 0}));
    return "";
  }
  EXPECT_TRUE(dag.has_value());
  if (dag) {
    RenderError renderErr;
    auto result = renderDAG(*dag, renderErr);
    EXPECT_EQ(renderErr.code, RenderError::Code::None);
    if (renderErr.code != RenderError::Code::None) {
      // Print error message by violating an assertion
      EXPECT_EQ(renderErr.message, "");
      // Print error location by violating an assertion
      EXPECT_EQ(renderErr.nodeId, 0U);
      return "";
    }
    EXPECT_TRUE(result.has_value());
    if (result) {
      return "\n" + *result;
    }
  }
  return "";
}

bool isNodeChar(char c) {
  switch (c) {
    case '\n':
      return false;
    case ' ':
      return false;
    case 'X':
      return false;
    case '/':
      return false;
    case '|':
      return false;
    case '\\':
      return false;
    default:
      return true;
  }
}

std::unordered_map<string, std::pair<size_t, size_t>> parseLayers(string_view str) {
  std::unordered_map<string, std::pair<size_t, size_t>> ret;
  size_t curLayer = 0;
  size_t curCol = 0;
  bool anyNodesOnThisLine = false;
  string curNode = "";
  auto addNode = [&ret](string&& curNode, size_t curLayer, size_t curCol) {
    auto [_, inserted] =
      ret.insert(std::make_pair(std::move(curNode), std::make_pair(curLayer, curCol)));
    EXPECT_TRUE(inserted);
  };
  for (char c : str) {
    if (isNodeChar(c)) {
      curNode += c;
    } else {
      if (!curNode.empty()) {
        anyNodesOnThisLine = true;
        addNode(std::move(curNode), curLayer, curCol);
        curNode.clear();
        ++curCol;
      }
      if (c == '\n') {
        if (anyNodesOnThisLine) {
          ++curLayer;
        }
        curCol = 0;
        anyNodesOnThisLine = false;
      }
    }
  }
  if (!curNode.empty()) {
    addNode(std::move(curNode), curLayer, curCol);
  }
  return ret;
}

size_t maxLayer(std::unordered_map<string, std::pair<size_t, size_t>> const& layerMapping) {
  size_t maxLayer = 0;
  for (auto const& [s, v] : layerMapping) {
    maxLayer = std::max(v.first, maxLayer);
  }
  return maxLayer;
}

Vec2<size_t> reconstructLayers(
  DAG const& dag,
  std::unordered_map<string, std::pair<size_t, size_t>> const& layerMapping
) {
  Vec2<size_t> ret;
  size_t const N = dag.nodes.size();
  size_t const nLayers = maxLayer(layerMapping) + 1;
  ret.resize(nLayers);
  for (size_t i = 0; i < N; ++i) {
    EXPECT_LT(0, layerMapping.count(dag.nodes[i].text));
    auto const& coords = layerMapping.at(dag.nodes[i].text);
    ret[coords.first].push_back(i);
  }
  for (auto& layer : ret) {
    std::sort(layer.begin(), layer.end(), [&layerMapping, &dag](size_t a, size_t b) {
      return layerMapping.at(dag.nodes[a].text).second < layerMapping.at(dag.nodes[b].text).second;
    });
  }
  return ret;
}

std::pair<DAG, Vec2<size_t>> parseWithLayers(string_view str) {
  ParseError parseErr;
  auto dag = parseDAG(str, parseErr);
  EXPECT_EQ(parseErr.code, ParseError::Code::None);
  if (parseErr.code != ParseError::Code::None) {
    // Print error message by violating an assertion
    EXPECT_EQ(parseErr.message, "");
    // Print error location by violating an assertion
    EXPECT_EQ(parseErr.pos, (Position{0, 0}));
    return {};
  }
  auto layerMapping = parseLayers(str);
  return std::make_pair(std::move(*dag), reconstructLayers(*dag, layerMapping));
}

void assertRenderAndParseIdentity(DAG const& dag) {
  RenderError renderErr;
  auto pic = renderDAG(dag, renderErr);
  EXPECT_EQ(renderErr.code, RenderError::Code::None);
  if (renderErr.code != RenderError::Code::None) {
    std::cout <<toDOT(dag) <<"\n";
    // Print error message by violating an assertion
    EXPECT_EQ(renderErr.message, "");
    // Print error location by violating an assertion
    EXPECT_EQ(renderErr.nodeId, 0U);
  }
  ASSERT_TRUE(pic.has_value());
  if (pic) {
    ParseError parseErr;
    auto dagClone = parseDAG(*pic, parseErr);
    EXPECT_EQ(parseErr.code, ParseError::Code::None);
    if (parseErr.code != ParseError::Code::None) {
      std::cout <<toDOT(dag) <<"\n";
      std::cout <<*pic <<"\n";
      // Print error message by violating an assertion
      EXPECT_EQ(parseErr.message, "");
      // Print error location by violating an assertion
      EXPECT_EQ(parseErr.pos, (Position{0, 0}));
    }
    ASSERT_TRUE(dagClone.has_value());
    ASSERT_NO_FATAL_FAILURE(assertEqual(dag, *dagClone));
  }
}

} // namespace asciidag::tests
