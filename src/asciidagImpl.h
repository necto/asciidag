#pragma once

#include "asciidag.h"

#include <string>
#include <vector>

namespace asciidag {

enum class Direction : int { Left = 1, Straight = 2, Right = 3 };

class Canvas {
public:
  static Canvas create(std::vector<Position> const& coordinates);
  static Canvas fromString(std::string const& str);

  void newMark(Position const& pos, char c);
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

} // namespace asciidag
