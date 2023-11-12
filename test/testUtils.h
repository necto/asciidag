#pragma once

#include "asciidag.h"
#include "asciidagImpl.h"

namespace asciidag::tests {

using namespace asciidag::detail;

string parseAndRender(string_view str);

std::pair<DAG, Vec2<size_t>> parseWithLayers(string_view str);

void assertRenderAndParseIdentity(DAG const& dag);

} // namespace asciidag::tests
