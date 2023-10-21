#include "testUtils.h"

#include <gtest/gtest.h>

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

} // namespace asciidag::tests
