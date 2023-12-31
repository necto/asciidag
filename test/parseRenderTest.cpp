#include "asciidag.h"
#include "testUtils.h"

#include <cstddef>
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <random>

using namespace asciidag;
using namespace asciidag::tests;

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

TEST(parseRender, stableSuccs) {
  // Here some former implementation used to mix up the node positions in
  // a layer so that it no longer matched the positions in the "succs" vector.
  DAG dag;
  dag.nodes.push_back(DAG::Node{{3, 4, 5, 6}, "00000\n00000\n00000\n00000"});
  dag.nodes.push_back(DAG::Node{{2, 4, 6, 9}, "1111\n1111\n1111\n1111"});
  dag.nodes.push_back(DAG::Node{{4, 5, 6, 9}, "222\n222\n222\n222"});
  dag.nodes.push_back(DAG::Node{{6, 7, 8, 9}, "33333\n33333\n33333\n33333\n33333\n33333\n33333\n33333"});
  dag.nodes.push_back(DAG::Node{{5, 7}, "4\n4\n4\n4\n4"});
  dag.nodes.push_back(DAG::Node{{6, 7, 8, 9}, "555555\n555555\n555555\n555555\n555555\n555555\n555555"});
  dag.nodes.push_back(DAG::Node{{}, "666666\n666666\n666666\n666666\n666666\n666666"});
  dag.nodes.push_back(DAG::Node{{}, "777\n777\n777\n777\n777\n777\n777"});
  dag.nodes.push_back(DAG::Node{{}, "8888\n8888\n8888"});
  dag.nodes.push_back(DAG::Node{{}, "99"});
  ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
}

TEST(parseRender, stablePreds) {
  DAG dag;
  dag.nodes.push_back(DAG::Node{{3, 4, 5}, "0"});
  dag.nodes.push_back(DAG::Node{{3, 5}, "1"});
  dag.nodes.push_back(DAG::Node{{3, 4, 5}, "2"});
  dag.nodes.push_back(DAG::Node{{}, "3"});
  dag.nodes.push_back(DAG::Node{{}, "4"});
  dag.nodes.push_back(DAG::Node{{}, "5"});
  ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
}

TEST(parseRender, nineNodesStableCrossings) {
  DAG dag;
  dag.nodes.push_back(DAG::Node{{3, 8}, "00\n00\n00\n00\n00\n00\n00"});
  dag.nodes.push_back(DAG::Node{{2, 5, 7, 8}, "1111\n1111"});
  dag.nodes.push_back(DAG::Node{{3, 4, 5, 6, 7, 8}, "2222\n2222\n2222\n2222\n2222\n2222\n2222\n2222"});
  dag.nodes.push_back(DAG::Node{{4, 7, 8}, "3333333"});
  dag.nodes.push_back(DAG::Node{{5, 6, 7, 8}, "4444444\n4444444\n4444444\n4444444\n4444444\n4444444\n4444444\n4444444"});
  dag.nodes.push_back(DAG::Node{{}, "55555"});
  dag.nodes.push_back(DAG::Node{{}, "6\n6\n6\n6\n6\n6\n6"});
  dag.nodes.push_back(DAG::Node{{}, "77\n77\n77\n77\n77\n77\n77\n77"});
  dag.nodes.push_back(DAG::Node{{}, "888\n888\n888\n888\n888\n888\n888\n888"});
  ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
}

TEST(parseRender, insertingAlwaysLeftMostCrossingFirst) {
  DAG dag;
  dag.nodes.push_back(DAG::Node{{1, 3, 4, 6, 7, 8}, "000000"});
  dag.nodes.push_back(DAG::Node{{2, 4, 5, 8}, "11\n11"});
  dag.nodes.push_back(DAG::Node{{3, 4, 5}, "222\n222\n222\n222\n222\n222\n222\n222"});
  dag.nodes.push_back(DAG::Node{{4, 5, 7, 8}, "333333\n333333"});
  dag.nodes.push_back(DAG::Node{{5, 7, 8}, "44\n44\n44\n44\n44"});
  dag.nodes.push_back(DAG::Node{{6, 7, 8}, "5555\n5555\n5555"});
  dag.nodes.push_back(DAG::Node{{}, "66666666"});
  dag.nodes.push_back(DAG::Node{{}, "777777"});
  dag.nodes.push_back(DAG::Node{{}, "888"});
  ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
}

TEST(parseRender, insertingAlwaysLeftMostCrossingFirst2) {
  DAG dag;
  dag.nodes.push_back(DAG::Node{{1, 5}, "00000\n00000\n00000"});
  dag.nodes.push_back(DAG::Node{{3, 4, 5, 6, 7, 8}, "1111\n1111\n1111\n1111"});
  dag.nodes.push_back(DAG::Node{{4, 5, 6, 7, 8}, "2222"});
  dag.nodes.push_back(DAG::Node{{4, 5, 6}, "3333333\n3333333\n3333333\n3333333\n3333333"});
  dag.nodes.push_back(DAG::Node{{7, 8}, "44444\n44444\n44444\n44444"});
  dag.nodes.push_back(DAG::Node{{6, 7, 8}, "555555\n555555\n555555\n555555\n555555\n555555\n555555\n555555"});
  dag.nodes.push_back(DAG::Node{{}, "666\n666\n666\n666\n666\n666\n666\n666"});
  dag.nodes.push_back(DAG::Node{{}, "77\n77"});
  dag.nodes.push_back(DAG::Node{{}, "888\n888\n888\n888\n888\n888\n888\n888"});
  ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
}

TEST(parseRender, threeIterationsToReadjustEdges) {
  DAG dag;
  dag.nodes.push_back(DAG::Node{{2, 3, 6, 7},"00000000\n00000000\n00000000\n00000000" });
  dag.nodes.push_back(DAG::Node{{4, 5, 6, 8},"1111111\n1111111\n1111111\n1111111\n1111111\n1111111\n1111111" });
  dag.nodes.push_back(DAG::Node{{4, 5, 7},"2222\n2222\n2222\n2222\n2222" });
  dag.nodes.push_back(DAG::Node{{6, 7},"333333\n333333\n333333\n333333\n333333" });
  dag.nodes.push_back(DAG::Node{{8},"444\n444\n444\n444\n444" });
  dag.nodes.push_back(DAG::Node{{},"55\n55\n55\n55\n55\n55\n55" });
  dag.nodes.push_back(DAG::Node{{7},"6\n6\n6\n6\n6\n6\n6\n6" });
  dag.nodes.push_back(DAG::Node{{8},"777777\n777777\n777777\n777777\n777777\n777777" });
  dag.nodes.push_back(DAG::Node{{},"88888\n88888\n88888\n88888\n88888" });
  ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
}

std::string rectLabel(char filler, size_t width, size_t height) {
  std::string ret;
  bool first = true;
  for (size_t line = 0; line < height; ++line) {
    if (!first) {
      ret += '\n';
    }
    first = false;
    for (size_t col = 0; col < width; ++col) {
      ret += filler;
    }
  }
  return ret;
}

DAG graphNodesFromSeed(size_t seed, size_t size) {
  DAG ret;
  for (size_t i = 0; i < size; ++i) {
    size_t width = 1 + (seed & 0b111);
    seed >>= 3;
    size_t height = 1 + (seed & 0b111);
    seed >>= 3;
    ret.nodes.push_back({{}, rectLabel('0' + i, width, height)});
  }
  return ret;
}

void configureDAGFromSeed(DAG &dag, size_t seed) {
  for (auto& node : dag.nodes) {
    node.succs.clear();
  }
  std::vector<size_t> nPreds(dag.nodes.size(), 0);
  size_t shift = 0;
  for (int node = 0; node < dag.nodes.size(); ++node) {
    size_t nodeWidth =
      std::find(dag.nodes[node].text.begin(), dag.nodes[node].text.end(), '\n')
      - dag.nodes[node].text.begin();
    for (int succ = node + 1; succ < dag.nodes.size(); ++succ) {
      size_t succWidth =
        std::find(dag.nodes[succ].text.begin(), dag.nodes[succ].text.end(), '\n')
        - dag.nodes[succ].text.begin();
      if (2 + nodeWidth <= dag.nodes[node].succs.size()) {
        break;
      }
      if (2 + succWidth <= nPreds[succ]) {
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

class probeRandomGraphs
  : public testing::TestWithParam<std::tuple<size_t, size_t>> {
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

std::array<std::string, 10> const nodeLabelDeeper = {
  "0",
  "1\n1",
  "2\n2\n2",
  "3\n3\n3\n3",
  "4\n4\n4\n4\n4",
  "5\n5\n5\n5\n5\n5",
  "6\n6\n6\n6\n6\n6\n6",
  "7\n7\n7\n7\n7\n7\n7\n7",
  "8\n8\n8\n8\n8\n8\n8\n8\n8",
  "9\n9\n9\n9\n9\n9\n9\n9\n9\n9"
};

std::array<std::string, 10> const nodeLabelShallower = {
  "0\n0\n0\n0\n0\n0\n0\n0\n0\n0",
  "1\n1\n1\n1\n1\n1\n1\n1\n1",
  "2\n2\n2\n2\n2\n2\n2\n2",
  "3\n3\n3\n3\n3\n3\n3",
  "4\n4\n4\n4\n4\n4",
  "5\n5\n5\n5\n5",
  "6\n6\n6\n6",
  "7\n7\n7",
  "8\n8",
  "9"
};

std::array<std::string, 10> const nodeLabelDepthZigZag = {
  "000\n000",
  "11\n11\n11",
  "2\n2",
  "3\n3\n3\n3\n3",
  "4\n4\n4",
  "555",
  "6\n6\n6\n6\n6\n6",
  "77\n77\n77\n77",
  "88\n88",
  "9"
};

constexpr size_t batchSize = 10000;

TEST_P(enumerateAllGraphs, parseOfRenderIsIdentity) {
  DAG dag;
  auto const [nodeLabel, nodeCount, from] = GetParam();
  for (size_t nodeId = 0; nodeId < nodeCount; ++nodeId) {
    dag.nodes.push_back({{}, (*nodeLabel)[nodeId]});
  }
  size_t to = std::min(from + batchSize, numberOfEdgeConfigurations(nodeCount));
  for (size_t seed = from; seed < to; ++seed) {
    configureDAGFromSeed(dag, seed);
    ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
  }
}

TEST_P(probeRandomGraphs, parseOfRenderIsIdentity) {
  auto const [nodeCount, seed] = GetParam();
  std::mt19937_64 gen(seed);
  gen.discard(1);
  size_t nodesSeed = gen();
  size_t edgesSeed = gen();
  DAG dag = graphNodesFromSeed(nodesSeed, nodeCount);
  for (size_t i = 0; i < std::min(batchSize, numberOfEdgeConfigurations(nodeCount)); ++i) {
    edgesSeed = gen();
    configureDAGFromSeed(dag, edgesSeed);
    ASSERT_NO_FATAL_FAILURE(assertRenderAndParseIdentity(dag));
  }
}

INSTANTIATE_TEST_SUITE_P(
  testSome345nodeGraphs,
  probeRandomGraphs,
  testing::Combine(
    testing::Values((size_t)3, (size_t)4, (size_t)5),
    testing::Range((size_t)0, 4*batchSize, batchSize)
  )
);

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
      &nodeLabelSaw,
      &nodeLabelDeeper,
      &nodeLabelShallower,
      &nodeLabelDepthZigZag
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
      &nodeLabelSaw,
      &nodeLabelDeeper,
      &nodeLabelShallower,
      &nodeLabelDepthZigZag
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
      &nodeLabelSaw,
      &nodeLabelDeeper,
      &nodeLabelShallower,
      &nodeLabelDepthZigZag
    ),
    testing::Values((size_t)5),
    testing::Range((size_t)0, numberOfEdgeConfigurations(5), batchSize)
  )
);

//#define LONG_BRUTEFORCE_TESTS
#ifdef LONG_BRUTEFORCE_TESTS

INSTANTIATE_TEST_SUITE_P(
  testSome6789nodeGraphs,
  probeRandomGraphs,
  testing::Combine(
    testing::Values((size_t)6, (size_t)7, (size_t)8, (size_t)9),
    testing::Range((size_t)0, 100*batchSize, batchSize)
  )
);

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
      &nodeLabelSaw,
      &nodeLabelDeeper,
      &nodeLabelShallower,
      &nodeLabelDepthZigZag
    ),
    testing::Values((size_t)6),
    testing::Range((size_t)0, numberOfEdgeConfigurations(6), batchSize)
  )
);

INSTANTIATE_TEST_SUITE_P(
  test7nodeGraphs,
  enumerateAllGraphs,
  testing::Combine(
    //testing::Values(&nodeLabelSaw),
    testing::Values(
      &nodeLabelSingleDigit,
      &nodeLabelDown,
      &nodeLabelUp,
      &nodeLabelDownUp,
      &nodeLabelUpDown,
      &nodeLabelSaw,
      &nodeLabelDeeper,
      &nodeLabelShallower,
      &nodeLabelDepthZigZag
    ),
    testing::Values((size_t)7),
    testing::Range((size_t)0, numberOfEdgeConfigurations(7), batchSize)
  )
);

INSTANTIATE_TEST_SUITE_P(
  test8nodeGraphs,
  enumerateAllGraphs,
  testing::Combine(
    testing::Values(
      // &nodeLabelSingleDigit,
      // &nodeLabelDown,
      // &nodeLabelUp,
      // &nodeLabelDownUp,
      // &nodeLabelUpDown,
      // &nodeLabelSaw,
      // &nodeLabelDeeper,
      // &nodeLabelShallower,
      &nodeLabelDepthZigZag
    ),
    testing::Values((size_t)8),
    testing::Range((size_t)0, numberOfEdgeConfigurations(8), batchSize)
  )
);

#endif // LONG_BRUTEFORCE_TESTS
