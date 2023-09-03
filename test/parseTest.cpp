#include <gtest/gtest.h>
#include <string>

#include "asciidag.h"

TEST(parse, twoConnectedNodes) {
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
