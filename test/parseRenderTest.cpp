#include "asciidag.h"
#include "testUtils.h"

#include <cstddef>
#include <gtest/gtest-param-test.h>
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
      << toDOT(a) << "\n"
      << rendering << " was transformed from " << a << " to " << b;
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
  std::vector<size_t> nPreds(dag.nodes.size(), 0);
  size_t shift = 0;
  for (int node = 0; node < dag.nodes.size(); ++node) {
    for (int succ = node + 1; succ < dag.nodes.size(); ++succ) {
      assert(dag.nodes[node].text.find('\n') == std::string::npos);
      if (2 + dag.nodes[node].text.size() <= dag.nodes[node].succs.size()) {
        break;
      }
      if (2 + dag.nodes[succ].text.size() <= nPreds[succ]) {
        continue;
      }
      size_t mask = 1 << shift;
      if (seed & mask) {
        dag.nodes[node].succs.push_back(succ);
        ++nPreds[succ];
      }
      ++shift;
    }
  }
}

constexpr size_t numberOfEdgeConfigurations(size_t nodeCount) {
  size_t const maxEdgesCount = nodeCount * (nodeCount - 1) / 2;
  return 1ULL << maxEdgesCount;
}

class enumerateAllGraphs
  : public testing::TestWithParam<std::tuple<std::array<std::string, 10> const*, size_t, size_t>> {
};

std::array<std::string, 10> const nodeLabelSingleDigit =
  {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

std::array<std::string, 10> const nodeLabelUp =
  {"0", "11", "222", "3333", "44444", "555555", "6666666", "77777777", "888888888", "9999999999"};

std::array<std::string, 10> const nodeLabelDown =
  {"00000000000", "111111111", "22222222", "3333333", "444444", "55555", "6666", "777", "88", "9"};

std::array<std::string, 10> const nodeLabelUpDown =
  {"0", "11", "222", "3333", "44444", "5555", "666", "77", "8", "9"};

std::array<std::string, 10> const nodeLabelDownUp =
  {"00000", "1111", "222", "33", "4", "5", "66", "777", "8888", "99999"};

std::array<std::string, 10> const nodeLabelSaw =
  {"00", "1", "222", "3", "444", "5", "66", "777", "8", "9999"};

constexpr size_t batchSize = 10000;

TEST_P(enumerateAllGraphs, parseOfRenderIsIdentity) {
  DAG dag;
  auto const [nodeLabel, nodeCount, from] = GetParam();
  auto param = GetParam();
  for (size_t nodeId = 0; nodeId < nodeCount; ++nodeId) {
    dag.nodes.push_back({{}, (*nodeLabel)[nodeId]});
  }
  size_t to = std::min(from + batchSize, numberOfEdgeConfigurations(nodeCount));
  for (size_t seed = from; seed < to; ++seed) {
    configureDAGFromSeed(dag, seed);
    //std::cout <<seed <<":\n" <<toDOT(dag) <<"\n";
    ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
  }
}

INSTANTIATE_TEST_SUITE_P(
  test3nodeGraphs,
  enumerateAllGraphs,
  testing::Combine(
    testing::Values(
      &nodeLabelSingleDigit,
      &nodeLabelDown,
      &nodeLabelUp,
      &nodeLabelDownUp,
      &nodeLabelUpDown,
      &nodeLabelSaw
    ),
    testing::Values((size_t)3),
    testing::Range((size_t)0, numberOfEdgeConfigurations(3), batchSize)
  )
);

INSTANTIATE_TEST_SUITE_P(
  test4nodeGraphs,
  enumerateAllGraphs,
  testing::Combine(
    testing::Values(
      &nodeLabelSingleDigit,
      &nodeLabelDown,
      &nodeLabelUp,
      &nodeLabelDownUp,
      &nodeLabelUpDown,
      &nodeLabelSaw
    ),
    testing::Values((size_t)4),
    testing::Range((size_t)0, numberOfEdgeConfigurations(4), batchSize)
  )
);

INSTANTIATE_TEST_SUITE_P(
  test5nodeGraphs,
  enumerateAllGraphs,
  testing::Combine(
    testing::Values(
      &nodeLabelSingleDigit,
      &nodeLabelDown,
      &nodeLabelUp,
      &nodeLabelDownUp,
      &nodeLabelUpDown,
      &nodeLabelSaw
    ),
    testing::Values((size_t)5),
    testing::Range((size_t)0, numberOfEdgeConfigurations(5), batchSize)
  )
);

//#define LONG_BRUTFORCE_TESTS
#ifdef LONG_BRUTFORCE_TESTS

INSTANTIATE_TEST_SUITE_P(
  test6nodeGraphs,
  enumerateAllGraphs,
  testing::Combine(
    testing::Values(
      &nodeLabelSingleDigit,
      &nodeLabelDown,
      &nodeLabelUp,
      &nodeLabelDownUp,
      &nodeLabelUpDown,
      &nodeLabelSaw
    ),
    testing::Values((size_t)6),
    testing::Range((size_t)0, numberOfEdgeConfigurations(6), batchSize)
  )
);

INSTANTIATE_TEST_SUITE_P(
  test7nodeGraphs,
  enumerateAllGraphs,
  testing::Combine(
    testing::Values(&nodeLabelSaw),
    testing::Values((size_t)7),
    testing::Range((size_t)0, numberOfEdgeConfigurations(7), batchSize)
  )
);

INSTANTIATE_TEST_SUITE_P(
  test8nodeGraphs,
  enumerateAllGraphs,
  testing::Combine(
    testing::Values(&nodeLabelSaw),
    testing::Values((size_t)8),
    testing::Range((size_t)0, numberOfEdgeConfigurations(8), batchSize)
  )
);

#endif // LONG_BRUTFORCE_TESTS
