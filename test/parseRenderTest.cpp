#include "asciidag.h"
#include "testUtils.h"

#include <gtest/gtest.h>
#include <iostream>

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
    std::sort(node.succs.begin(), node.succs.end(), [&](size_t a, size_t b) {
      return ret.nodes[a].text < ret.nodes[b].text;
    });
  }
  return ret;
}

bool compareDAGs(DAG const& a, DAG const& b) {
  if (a.nodes.size() != a.nodes.size()) {
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
    GTEST_FAIL() <<"Graph \n" <<rendering <<" was transformed from " <<a <<" to " <<b;
  }
}

void assertRenderAndParseIdentity(DAG const& dag) {
  RenderError renderErr;
  auto pic = renderDAG(dag, renderErr);
  EXPECT_EQ(renderErr.code, RenderError::Code::None);
  if (renderErr.code != RenderError::Code::None) {
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
      // Print error message by violating an assertion
      EXPECT_EQ(parseErr.message, "");
      // Print error location by violating an assertion
      EXPECT_EQ(parseErr.pos, (Position{0, 0}));
    }
    ASSERT_TRUE(dagClone.has_value());
    ASSERT_NO_FATAL_FAILURE(assertEqual(dag, *dagClone));
  }
}

TEST(parseRender, empty) {
  DAG dag;
  ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
}

TEST(parseRender, oneNode) {
  DAG dag;
  dag.nodes.push_back({{}, "0"});
  ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
}

TEST(parseRender, oneEdge) {
  DAG dag;
  dag.nodes.push_back({{1}, "0"});
  dag.nodes.push_back({{}, "1"});
  ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
}

void configureDAGFromSeed(DAG &dag, size_t seed) {
  for (auto& node : dag.nodes) {
    node.succs.clear();
  }
  size_t shift = 0;
  for (int node = 0; node < dag.nodes.size(); ++node) {
    for (int succ = node + 1; succ < dag.nodes.size(); ++succ) {
      if (3 <= dag.nodes[node].succs.size()) {
        // single-character nodes do not support more than 3 successors
        break;
      }
      size_t mask = 1 << shift;
      if (seed & mask) {
        dag.nodes[node].succs.push_back(succ);
      }
      ++shift;
    }
  }
}

size_t numberOfEdgeConfigurations(size_t nodeCount) {
  size_t const maxEdgesCount = nodeCount * (nodeCount - 1) / 2;
  return 1 << maxEdgesCount;
}

TEST(parseRender, generated3) {
  DAG dag;
  dag.nodes.push_back({{}, "0"});
  dag.nodes.push_back({{}, "1"});
  dag.nodes.push_back({{}, "2"});
  size_t const nPermutations = numberOfEdgeConfigurations(dag.nodes.size());
  for (size_t seed = 0; seed < nPermutations; ++seed) {
    configureDAGFromSeed(dag, seed);
    ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
  }
}

TEST(parseRender, generated4) {
  DAG dag;
  dag.nodes.push_back({{}, "0"});
  dag.nodes.push_back({{}, "1"});
  dag.nodes.push_back({{}, "2"});
  dag.nodes.push_back({{}, "3"});
  size_t const nPermutations = numberOfEdgeConfigurations(dag.nodes.size());
  for (size_t seed = 0; seed < nPermutations; ++seed) {
    configureDAGFromSeed(dag, seed);
    ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
  }
}

// TODO:
// TEST(parseRender, generated5) {
//   DAG dag;
//   dag.nodes.push_back({{}, "0"});
//   dag.nodes.push_back({{}, "1"});
//   dag.nodes.push_back({{}, "2"});
//   dag.nodes.push_back({{}, "3"});
//   dag.nodes.push_back({{}, "4"});
//   size_t const nPermutations = numberOfEdgeConfigurations(dag.nodes.size());
//   for (size_t seed = 0; seed < nPermutations; ++seed) {
//     configureDAGFromSeed(dag, seed);
//     ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
//   }
// }
