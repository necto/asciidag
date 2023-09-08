#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct DAG {
  struct OutEdge {
    size_t to;
  };

  struct Node {
    std::vector<OutEdge> outEdges;
    std::string text;
  };

  std::vector<Node> nodes;
  // Invariant: root node index is 0U
};

struct ParseError {
  enum class Code { None, DanglingEdge, SuspendedEdge, MergingEdge };

  Code code;
  std::string message;
  size_t line;
  size_t col;
};

std::ostream &operator<<(std::ostream &os, ParseError const &err);
std::string parseCodeToStr(ParseError::Code code);

std::string renderDAG(DAG const &dag);

std::optional<DAG> parseDAG(std::string str, ParseError &err);
