#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace asciidag {

struct DAG {
  struct Node {
    std::vector<size_t> succs;
    std::string text;
  };

  std::vector<Node> nodes;
  // Invariant: root node index is 0U
};

struct Position {
  size_t line;
  size_t col;
};

std::ostream& operator<<(std::ostream& os, Position const& pos);

inline bool operator==(Position const& p1, Position const& p2) {
  return std::tie(p1.line, p1.col) == std::tie(p2.line, p2.col);
}

struct ParseError {
  enum class Code { None, DanglingEdge, SuspendedEdge, MergingEdge, NonRectangularNode };

  Code code;
  std::string message;
  Position pos;
};

std::ostream& operator<<(std::ostream& os, ParseError const& err);
std::string parseErrorCodeToStr(ParseError::Code code);

inline std::ostream& operator<<(std::ostream& os, ParseError::Code code) {
  return os << parseErrorCodeToStr(code);
}

struct RenderError {
  enum class Code { None, Unsupported };

  Code code;
  std::string message;
  size_t nodeId;
};

std::optional<std::string> renderDAG(DAG dag, RenderError& err);

std::optional<DAG> parseDAG(std::string str, ParseError& err);

} // namespace asciidag
