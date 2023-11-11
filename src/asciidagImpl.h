#pragma once

#include "asciidag.h"

#include <string>
#include <vector>

namespace asciidag::detail {

template <typename T>
using Vec = std::vector<T>;

template <typename T>
using Vec2 = Vec<Vec<T>>;

using std::string;
using std::string_view;

using namespace std::string_literals;

enum class Direction : int { Left = 1, Straight = 2, Right = 3 };

class Canvas {
public:
  static Canvas
  create(std::vector<Position> const& coordinates, std::vector<Position> const& dimensions);
  static Canvas fromString(std::string const& str);

  void newMark(Position const& pos, char c);
  void newMark(Position const& pos, std::string const& str);
  void clearPos(Position const& pos);
  char getChar(Position const& pos) const;
  bool isEmpty(Position const& pos) const { return getChar(pos) == ' '; }
  size_t width() const;
  size_t height() const;
  bool inBounds(Position const& pos) const;

  std::string render() const;

private:
  Canvas(){};
  std::vector<std::string> lines;
};

/// Returns false if it failed to draw the edge
bool drawEdge(Position cur, Direction curDir, Position to, Direction finishDir, Canvas& canvas);

string renderDAGWithLayers(DAG const& dag, Vec2<size_t> layers);

void minimizeCrossings(Vec2<size_t>& layers, DAG& dag);

Vec2<size_t> insertCrossNodes(DAG& dag, Vec2<size_t> const& layers);

struct CrossingPair {
  size_t fromLeft;
  size_t fromRight;
  size_t toLeft;
  size_t toRight;
};

Vec<CrossingPair>
findNonConflictingCrossings(DAG const& dag, Vec<size_t> const& lAbove, Vec<size_t> const& lBelow);

size_t countCrossings(DAG const& dag, Vec<size_t> const& lAbove, Vec<size_t> const& lBelow);

} // namespace asciidag::detail
