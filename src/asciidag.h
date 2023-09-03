#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

struct DAG {
  using Props = std::unordered_map<std::string, std::string>;

  struct OutEdge {
    size_t to;
    Props props;
  };

  struct Node {
    std::vector<OutEdge> outEdges;
    Props props;
  };

  std::vector<Node> nodes;
  // Invariant: root node == 0U
};

std::string renderDAG(DAG const& dag);
DAG parseDAG(std::string str);
