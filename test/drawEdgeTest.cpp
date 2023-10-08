#include "asciidagImpl.h"

#include <gtest/gtest.h>
#include <sstream>

using namespace asciidag;
using std::string;
using std::vector;

vector<string> derenderCanvas(string const& rendered) {
  vector<string> ret;
  string curLine;
  std::stringstream ss(rendered);
  size_t width = 0;
  while (getline(ss, curLine, '\n')) {
    width = std::max(width, curLine.size());
    ret.push_back(curLine);
  }
  for (auto& line : ret) {
    line.resize(width, ' ');
  }
  return ret;
}

string drawEdgeFromSpec(string const& spec) {
  Position from;
  Direction fromAngle;
  bool fromFound = false;
  Position to;
  Direction toAngle;
  bool toFound = false;
  auto canvas = derenderCanvas(spec);
  for (size_t lineNum = 0; lineNum < canvas.size(); ++lineNum) {
    for (size_t col = 0; col < canvas[lineNum].size(); ++col) {
      if (canvas[lineNum][col] == '.') {
        assert(!toFound);
        if (fromFound) {
          to = {lineNum, col};
          toFound = true;
          assert(0 < lineNum);
          if (0 < col && canvas[lineNum - 1][col - 1] == '\\') {
            toAngle = Direction::Right;
          } else if (canvas[lineNum - 1][col] == '|') {
            toAngle = Direction::Straight;
          } else {
            assert(col + 1 < canvas[lineNum - 1].size());
            assert(canvas[lineNum - 1][col + 1] == '/');
            toAngle = Direction::Left;
          }
        } else {
          from = {lineNum, col};
          fromFound = true;
          assert(lineNum < canvas.size());
          if (0 < col && canvas[lineNum + 1][col - 1] == '/') {
            fromAngle = Direction::Left;
          } else if (canvas[lineNum + 1][col] == '|') {
            fromAngle = Direction::Straight;
          } else {
            assert(col + 1 < canvas[lineNum + 1].size());
            assert(canvas[lineNum + 1][col + 1] == '\\');
            fromAngle = Direction::Right;
          }
        }
      }
    }
  }
  // Erase the edge characters
  for (auto &line : canvas) {
    for (auto & c : line) {
      if (c != '.') c = ' ';
    }
  }
  drawEdge(from, fromAngle, to, toAngle, canvas);
  return renderCanvas(canvas);
}

TEST(drawEdge, straightDownLen1) {
  std::string spec = R"(
  .
  |
  .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightDownLen2) {
  std::string spec = R"(
  .
  |
  |
  .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightDownLen3) {
  std::string spec = R"(
  .
  |
  |
  |
  .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightDownLen4) {
  std::string spec = R"(
  .
  |
  |
  |
  |
  .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightDownLen5) {
  std::string spec = R"(
  .
  |
  |
  |
  |
  |
  .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightLeftLen1) {
  std::string spec = R"(
  .
 /
.
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightLeftLen2) {
  std::string spec = R"(
   .
  /
 /
.
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightLeftLen3) {
  std::string spec = R"(
    .
   /
  /
 /
.
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}


TEST(drawEdge, straightLeftLen4) {
  std::string spec = R"(
     .
    /
   /
  /
 /
.
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightLeftLen5) {
  std::string spec = R"(
      .
     /
    /
   /
  /
 /
.
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRightLen1) {
  std::string spec = R"(
  .
   \
    .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRightLen2) {
  std::string spec = R"(
  .
   \
    \
     .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRightLen3) {
  std::string spec = R"(
  .
   \
    \
     \
      .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRightLen4) {
  std::string spec = R"(
  .
   \
    \
     \
      \
       .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRightLen5) {
  std::string spec = R"(
  .
   \
    \
     \
      \
       \
        .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, pivot1StepRight) {
  std::string spec = R"(
  .
   \
   |
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, pivot1StepRightLongStraight) {
  std::string spec = R"(
  .
   \
   |
   |
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}


TEST(drawEdge, pivot2StepsRight) {
  std::string spec = R"(
  .
   \
    \
    |
    .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, pivot2StepsRightLongStraight) {
  std::string spec = R"(
  .
   \
    \
    |
    |
    .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, pivot1StepRightExtraStepAtTheEnd) {
  std::string spec = R"(
  .
   \
   |
   \
    .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, pivot1StepRightExtraStepAtTheEndLongStraight) {
  std::string spec = R"(
  .
   \
   |
   |
   \
    .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, suboptimalInitialDirections) {
  std::string spec = R"(
  .
   \
   /
  /
  \
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, suboptimalInitialDirectionsLen3) {
  std::string spec = R"(
   .
    \
    /
   /
  /
  \
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, suboptimalInitialDirectionsStraightSegment) {
  std::string spec = R"(
   .
    \
    /
   /
  /
  |
  |
  \
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, leftToStraight) {
  std::string spec = R"(
   .
    \
    /
   /
   |
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRight) {
  std::string spec = R"(
  .
  |
  \
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRightShiftBy1Len5) {
  std::string spec = R"(
  .
  |
  |
  \
   \
   |
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRightShiftBy1Len4) {
  std::string spec = R"(
  .
  |
  \
   \
   |
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRightShiftBy2Len5) {
  std::string spec = R"(
  .
  |
  \
   \
    \
    |
    .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightRightShiftBy2Len6) {
  std::string spec = R"(
  .
  |
  |
  \
   \
    \
    |
    .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightTurnRightBy2Len5) {
  std::string spec = R"(
  .
  |
  |
  |
  \
   \
    .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightTurnRightBy2Len4) {
  std::string spec = R"(
  .
  |
  |
  \
   \
    .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightTurnRightBy1Len3) {
  std::string spec = R"(
.
|
|
\
 .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightTurnRightBy1Len4) {
  std::string spec = R"(
  .
  |
  |
  |
  \
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightTurnRightBy1Len5) {
  std::string spec = R"(
  .
  |
  |
  |
  |
  \
   .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightTurnRightBy3Len4) {
  std::string spec = R"(
  .
  |
  \
   \
    \
     .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightTurnRightBy3Len5) {
  std::string spec = R"(
  .
  |
  |
  \
   \
    \
     .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, straightTurnRightBy3Len6) {
  std::string spec = R"(
  .
  |
  |
  |
  \
   \
    \
     .
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

// TODO: all the test from above but mirrored
