#include "testUtils.h"

#include "asciidag.h"
#include "asciidagImpl.h"

#include <gtest/gtest.h>

namespace asciidag::tests {

std::string parseAndRender(std::string_view str) {
  ParseError parseErr;
  auto dag = parseDAG(str, parseErr);
  EXPECT_EQ(parseErr.code, ParseError::Code::None);
  if (parseErr.code != ParseError::Code::None) {
    // Print error message by violating an assertion
    EXPECT_EQ(parseErr.message, "");
    // Print error location by violating an assertion
    EXPECT_EQ(parseErr.pos, (Position{0, 0}));
  }
  EXPECT_TRUE(dag.has_value());
  if (dag) {
    RenderError renderErr;
    auto result = renderDAG(*dag, renderErr);
    EXPECT_EQ(renderErr.code, RenderError::Code::None);
    if (renderErr.code != RenderError::Code::None) {
      // Print error message by violating an assertion
      EXPECT_EQ(renderErr.message, "");
      // Print error location by violating an assertion
      EXPECT_EQ(renderErr.nodeId, 0U);
    }
    EXPECT_TRUE(result.has_value());
    if (result) {
      return "\n" + *result;
    }
  }
  return "";
}

} // namespace asciidag::tests
