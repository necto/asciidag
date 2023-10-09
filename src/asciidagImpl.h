#pragma once

#include "asciidag.h"

#include <vector>
#include <string>

namespace asciidag {

enum class Direction : int { Left = 1, Straight = 2, Right = 3 };

void drawEdge(
  Position cur,
  Direction curDir,
  Position to,
  Direction finishDir,
  std::vector<std::string>& canvas
);

std::string renderCanvas(std::vector<std::string> const& canvas);

} // namespace asciidag
