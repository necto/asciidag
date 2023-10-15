#include "asciidag.h"

#include <gtest/gtest.h>

using namespace asciidag;

TEST(dag2dot, emptyDAG) {
  DAG test;
  EXPECT_EQ(toDOT(test), R"(digraph "DAG" {
}
)");
}

TEST(dag2dot, nodeWithQuote) {
  DAG test;
  test.nodes.push_back(DAG::Node{{}, R"(Multi
line
with "
some" quotes and
] { special chars)"});

  EXPECT_EQ(toDOT(test), R"(digraph "DAG" {
  n0[shape=record,label="Multi\nline\nwith \"\nsome\" quotes and\n] \{ special chars"];

}
)");
}

TEST(dag2dot, singleEdge) {
  DAG test;
  test.nodes.push_back(DAG::Node{{1}, "first node"});
  test.nodes.push_back(DAG::Node{{}, "second node"});

  EXPECT_EQ(toDOT(test),
R"(digraph "DAG" {
  n0[shape=record,label="first node"];
  n0 -> n1;

  n1[shape=record,label="second node"];

}
)");
}

TEST(dag2dot, pluralOutgoingIncomingEdges) {
  DAG test;
  test.nodes.push_back(DAG::Node{{2, 3}, "0"});
  test.nodes.push_back(DAG::Node{{2, 3}, "1"});
  test.nodes.push_back(DAG::Node{{3}, "2"});
  test.nodes.push_back(DAG::Node{{}, "3"});

  EXPECT_EQ(toDOT(test),
R"(digraph "DAG" {
  n0[shape=record,label="0"];
  n0 -> n2;
  n0 -> n3;

  n1[shape=record,label="1"];
  n1 -> n2;
  n1 -> n3;

  n2[shape=record,label="2"];
  n2 -> n3;

  n3[shape=record,label="3"];

}
)");
}
