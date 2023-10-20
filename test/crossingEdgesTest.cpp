#include "testUtils.h"

#include <gtest/gtest.h>

using namespace asciidag;
using namespace asciidag::tests;

TEST(crossingEdgesTest, simpleIrreducibleCrossing) {
  EXPECT_EQ(parseAndRender(R"(
0   1
|\ /|
| X |
|/ \|
2   3
)"),
R"(
0  1
|\ |\
| \| \
| || |
| |/ |
| X  |
| |\ |
| || |
| /| /
|/ |/
2  3
)");
}

TEST(crossingEdgesTest, tripleCrossings) {
  EXPECT_EQ(parseAndRender(R"(
   0   1
  /|\ /|\
 / | X | \
 | |/ \| |
 | X   X |
 |/|   |\|
 2 \   / 4
    \ /
     3
)"),
R"(
 0   1
/|\ /|\
|| \\\ \
||  \\\ \
|\  || \ \
| \ || | |
| | |/ | |
| | X  | |
| | |\ | |
| | || | /
| | /| //
| |/ |/ |
| X  X  |
| |\ |\ |
| || || |
| /| /| /
|/ |/ |/
2  4  3
)");
}

TEST(crossingEdgesTest, crossOnOneOfTwoLayers) {
  EXPECT_EQ(parseAndRender(R"(
   0   1
  /|\ /|
 / | X |
 | |/ \|
 \ 4   2
  \   /
   \ /
    3
)"),
R"(
 0  1
/|\ |\
|| \\ \
|\  \\ \
| \ || |
| | |/ |
| | X  |
| | |\ |
| | || |
| | /| /
| |/ |/
| 2  4
| |
| |
| /
|/
3
)");
}

TEST(crossingEdgesTest, trickyWaypointLocations) {
  EXPECT_EQ(parseAndRender(R"(
   0   1
  /|\ /|\
 / | X ||
 | |/ \||
 \ 4   2|
  \   / /
   \ / /
    \|/
     3
)"),
R"(
 0   1
/|\ /|\
|| \\\ \
||  \\\ \
|\  || \ \
| \ || | |
| | |/ | |
| | X  | |
| | |\ | |
| | || | /
| | /| //
| |/ |/ |
| 4  2  |
|    |  |
|    |  |
|    |  /
|    / /
|   / /
|  / /
| / /
|/ /
\|/
 3
)");
}

TEST(crossingEdgesTest, crossMustSwapSuccessors) {
  EXPECT_EQ(parseAndRender(R"(
      0
     / \
    /   \
   /    |
  1   2 |
  |\ /| |
  | X | /
  |/ \|/
  3   4
)"),
R"(
0  2
|\ |\
| \| \
| || |
| |/ |
| X  |
| |\ |
| || |
| /| |
|/ | |
X  | |
|\ | |
| \\ \
| | \ \
| |  \ \
| |  | |
| 1  | |
| |\ | |
| || | |
| || / /
| /|/ /
|/ \|/
3   4
)");
}

TEST(crossingEdgesTest, sixNodes) {
  EXPECT_EQ(parseAndRender(R"(
      0
     /|\
    / \ \
   /   \ \
  / 1   2 \
 / /|\ /| |
 |/ | X | /
 3  |/ \|/
    4   5
)"),
R"(
 0   1
/|\ /|\
|| \\\ \
||  \\\ \
|\   \\\ \
| \  || \ \
|  \ || | |
|  | |/ | |
2  | 3  | |
|\ |    | |
| \|    / /
| ||   / /
| ||  / /
| || / /
| |/ | |
| X  | |
| |\ | |
| | \| |
| | || |
| | |/ |
| | X  |
| | |\ |
| | || |
| / // /
|/ // /
\|/ |/
 5  4
)");
}
