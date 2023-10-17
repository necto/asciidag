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

TEST(crossingEdgesTest, crossOnOneOfTwoLayersFail) {
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
