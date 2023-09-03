#include <gtest/gtest.h>

#include "asciidag.h"

TEST(render, singleNode) {
  DAG test;
  test.nodes.push_back(DAG::Node{{{1, {}}, {2, {}}, {3, {}}}, {}});
  test.nodes.push_back(DAG::Node{{{4, {}}}, {}});
  test.nodes.push_back(DAG::Node{{{4, {}}}, {}});
  test.nodes.push_back(DAG::Node{{{5, {}}}, {}});
  test.nodes.push_back(DAG::Node{{{5, {}}}, {}});
  test.nodes.push_back(DAG::Node{{}, {}});
  EXPECT_EQ(renderDAG(test), "0 \n1 2 3 \n4 \n5 \n");
}
