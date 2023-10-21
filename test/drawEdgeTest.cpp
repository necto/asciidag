#include "testUtils.h"

#include <gtest/gtest.h>

namespace {

using namespace asciidag;
using namespace asciidag::tests;
using std::string;

void eraseEdgeCharacters(Canvas& canvas) {
  for (size_t line = 0; line < canvas.height(); ++line) {
    for (size_t col = 0; col < canvas.width(); ++col) {
      if (char c = canvas.getChar({line, col});
          c != '.' && c != '#' && c != ' ') {
        canvas.clearPos({line, col});
      }
    }
  }
}

Direction getEntryAngle(Position dotPos, Canvas const& canvas) {
  assert(0 < dotPos.line);
  if (0 < dotPos.col && canvas.getChar({dotPos.line - 1, dotPos.col - 1}) == '\\') {
    return Direction::Right;
  }
  if (canvas.getChar({dotPos.line - 1, dotPos.col}) == '|') {
    return Direction::Straight;
  }
  assert(dotPos.col + 1 < canvas.width());
  assert(canvas.getChar({dotPos.line - 1, dotPos.col + 1}) == '/');
  return Direction::Left;
}

Direction getExitAngle(Position dotPos, Canvas const& canvas) {
  assert(dotPos.line < canvas.height());
  if (0 < dotPos.col && canvas.getChar({dotPos.line + 1, dotPos.col - 1}) == '/') {
    return Direction::Left;
  }
  if (canvas.getChar({dotPos.line + 1, dotPos.col}) == '|') {
    return Direction::Straight;
  }
  assert(dotPos.col + 1 < canvas.width());
  assert(canvas.getChar({dotPos.line + 1, dotPos.col + 1}) == '\\');
  return Direction::Right;
}

std::tuple<Position, Direction, Position, Direction>
extractStartAndFinish(Canvas const& canvas) {
  Position from;
  Direction fromAngle;
  Position to;
  Direction toAngle;
  bool fromFound = false;
  bool toFound = false;
  for (size_t lineNum = 0; lineNum < canvas.height(); ++lineNum) {
    for (size_t col = 0; col < canvas.width(); ++col) {
      if (canvas.getChar({lineNum, col}) == '.') {
        assert(!toFound);
        if (fromFound) {
          to = {lineNum, col};
          toFound = true;
          toAngle = getEntryAngle(to, canvas);
        } else {
          from = {lineNum, col};
          fromFound = true;
          fromAngle = getExitAngle(from, canvas);
        }
      }
    }
  }
  assert(fromFound && toFound);
  return {from, fromAngle, to, toAngle};
}

string drawEdgeFromSpec(string const& spec) {
  auto canvas = Canvas::fromString(spec);
  auto [from, fromAngle, to, toAngle] = extractStartAndFinish(canvas);
  eraseEdgeCharacters(canvas);
  bool success = drawEdge(from, fromAngle, to, toAngle, canvas);
  EXPECT_TRUE(success);
  return canvas.render();
}

bool drawEdgeFromSpecFails(string const& spec) {
  auto canvas = Canvas::fromString(spec);
  auto [from, fromAngle, to, toAngle] = extractStartAndFinish(canvas);
  eraseEdgeCharacters(canvas);
  bool success = drawEdge(from, fromAngle, to, toAngle, canvas);
  return !success;
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

TEST(drawEdge, avoidOneObstacleAbove) {
  std::string spec = R"(
 .
  \
  |
 #/
 /
/
|
.
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, avoidTwoObstaclesAbove) {
  std::string spec = R"(
 .
  \
  /
 #|
 #/
 /
/
|
.
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, avoidObstacleLeft) {
  std::string spec = R"(
 .
  \
  /
 /
#|
 |
 |
 |
 /
/
|
.
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdge, avoidObstacleInfeasibleAlternativeDirection) {
  std::string spec = R"(
 .
  \
  /
 /#
#|
 |
 |
 |
 /
/
|
.
)";
  EXPECT_EQ(spec, drawEdgeFromSpec(spec));
}

TEST(drawEdgeError, incompatibleDirections) {
  std::string spec = R"(
  .
   \
   \
    .
)";
  EXPECT_TRUE(drawEdgeFromSpecFails(spec));
}

TEST(drawEdgeError, tooFarRight) {
  std::string spec = R"(
  .
  |
  \
   \
    \
     |
     .
)";
  EXPECT_TRUE(drawEdgeFromSpecFails(spec));
}

TEST(drawEdgeError, unavoidableObstacle) {
  std::string spec = R"(
  .
  |
  \
   #
    \
    |
    .
)";
  EXPECT_TRUE(drawEdgeFromSpecFails(spec));
}

TEST(drawEdgeError, almostAvoidableObstacle) {
  std::string spec = R"(
  .
  |
  \
  #\
   /
  |
  .
)";
  EXPECT_TRUE(drawEdgeFromSpecFails(spec));
}

TEST(drawEdgeError, tryingToAvoidThreeObstaclesAbove) {
  std::string spec = R"(
 .
  \
  /
 #|
 #/
 #
/
|
.
)";
  EXPECT_TRUE(drawEdgeFromSpecFails(spec));
}

TEST(drawEdgeError, limitedFlexibility) {
  // The edge path finding is limited to a narrow set of options
  // not deviating too far from the target direction.
  // Here it fails to find the path that requires it to step to an opposite direction
  // before locking on the target.
  std::string spec = R"(
  .
  |
  /
 /##
 \
  \
   \
    \
     .
)";
  EXPECT_TRUE(drawEdgeFromSpecFails(spec));
}

} // namespace

// TODO: all the test from above but mirrored
