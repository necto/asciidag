#include "asciidag.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace asciidag {

namespace {

using namespace std::string_literals;

std::vector<std::vector<size_t>> dagLayers(DAG const& dag) {
  std::vector<size_t> rank(dag.nodes.size(), 0);
  bool changed = false;
  do {
    changed = false;
    for (size_t n = 0; n < dag.nodes.size(); ++n) {
      for (auto const& e : dag.nodes[n].outEdges) {
        if (rank[e.to] < rank[n] + 1) {
          changed = true;
          rank[e.to] = rank[n] + 1;
        }
      }
    }
  } while (changed);
  size_t maxRank = *std::max_element(rank.begin(), rank.end());
  std::vector<std::vector<size_t>> ret(maxRank + 1);
  for (size_t n = 0; n < dag.nodes.size(); ++n) {
    ret[rank[n]].push_back(n);
  }
  return ret;
}

template <typename A>
std::ostream& operator<<(std::ostream& os, std::set<A> v) {
  os << "{";
  bool first = true;
  for (auto const& x : v) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << x;
  }
  os << "}";
  return os;
}

template <typename A>
std::ostream& operator<<(std::ostream& os, std::vector<A> v) {
  os << "[";
  bool first = true;
  for (auto const& x : v) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << x;
  }
  os << "]";
  return os;
}

template <typename A, typename B>
std::ostream& operator<<(std::ostream& os, std::unordered_map<A, B> map) {
  os << "{";
  bool first = true;
  for (auto const& [a, b] : map) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << a << " -> " << b;
  }
  os << "}";
  return os;
}

template <typename T>
void append(std::vector<T>& to, std::vector<T> const& from) {
  for (auto const& x : from) {
    to.push_back(x);
  }
}

template <typename T>
void add(std::set<T>& to, std::set<T> const& from) {
  for (auto const& x : from) {
    to.insert(x);
  }
}

template <typename K, typename V>
std::optional<V> findAndEraseIf(std::unordered_map<K, V>& map, K const& k) {
  std::optional<V> ret = std::nullopt;
  if (auto iter = map.find(k); iter != map.end()) {
    ret.emplace(std::move(iter->second));
    map.erase(iter);
  }
  return ret;
}

template <typename K, typename V>
V const* getIf(std::unordered_map<K, V> const& map, K const& k) {
  if (auto iter = map.find(k); iter != map.end()) {
    return &iter->second;
  }
  return nullptr;
}

using EdgeMap = std::unordered_map<size_t, size_t>;

class EdgesInFlight {
public:
  enum class Direction : int { Left = 1, Straight = 2, Right = 3 };

  static std::optional<Direction> edgeChar(char c);
  static char edgeChar(Direction dir);

  std::vector<size_t> findNRemoveEdgesToNode(size_t col);
  std::vector<size_t> findNRemoveEdgesToEdge(Direction dir, EdgeMap const& prevNodes, size_t col);
  std::optional<ParseError> findDanglingEdge(size_t line) const;
  friend std::ostream& operator<<(std::ostream& os, EdgesInFlight const& edges);

  std::optional<ParseError>
  updateOrError(std::vector<size_t> const& fromNodes, Direction dir, Position const& pos);

private:

  /// 0 - empty map, just for padding
  /// The rest corresponds to Direction options
  EdgeMap edges[4];
};

std::optional<EdgesInFlight::Direction> EdgesInFlight::edgeChar(char c) {
  switch (c) {
    case '|':
      return Direction::Straight;
    case '\\':
      return Direction::Right;
    case '/':
      return Direction::Left;
  }
  return {};
}

char EdgesInFlight::edgeChar(Direction dir) {
  switch (dir) {
    case Direction::Left:
      return '/';
    case Direction::Straight:
      return '|';
    case Direction::Right:
      return '\\';
  }
  assert(false && "Unexpected direction");
  return '?';
}

int toInt(EdgesInFlight::Direction dir) {
  return static_cast<int>(dir);
}

std::optional<ParseError> EdgesInFlight::
  updateOrError(std::vector<size_t> const& fromNodes, Direction dir, Position const& pos) {
  if (fromNodes.size() == 1) {
    edges[toInt(dir)][pos.col] = fromNodes.front();
    return {};
  }
  if (fromNodes.size() != 0) {
    return ParseError{ParseError::Code::MergingEdge, "Edges merged into one edge", pos};
  }
  return ParseError{
    ParseError::Code::SuspendedEdge,
    "Edge "s + edgeChar(dir) + " is suspended (not attached to any source node)",
    pos
  };
}

/// [Node, Left, Straight, Right]
static int columnShift[/* line above */ 4][/* line below */ 4] = {
  {0, +1, 0, -1},
  {+1, +1, 0, 0},
  {0, 0, 0, 0},
  {-1, 0, 0, -1},
};

std::vector<size_t> EdgesInFlight::findNRemoveEdgesToNode(size_t col) {
  std::vector<size_t> ret;
  for (auto dir : {toInt(Direction::Left), toInt(Direction::Straight), toInt(Direction::Right)}) {
    if (auto to = findAndEraseIf(edges[dir], col + columnShift[dir][0])) {
      ret.push_back(*to);
    }
  }
  return ret;
}

std::vector<size_t>
EdgesInFlight::findNRemoveEdgesToEdge(Direction dirBelow, EdgeMap const& prevNodes, size_t col) {
  std::vector<size_t> ret;
  for (auto dirAbove :
       {toInt(Direction::Left), toInt(Direction::Straight), toInt(Direction::Right)}) {
    if (auto from = findAndEraseIf(edges[dirAbove], col + columnShift[dirAbove][toInt(dirBelow)])) {
      ret.push_back(*from);
    }
  }
  // Avoid connecting an edge to a node if it is already connected to an edge
  if (ret.empty()) {
    if (auto to = getIf(prevNodes, col + columnShift[0][toInt(dirBelow)])) {
      ret.push_back(*to);
    }
  }
  return ret;
}

[[maybe_unused]]
std::ostream& operator<<(std::ostream& os, EdgesInFlight const& edges) {
  bool first = true;
  for (auto dir :
       {EdgesInFlight::Direction::Left,
        EdgesInFlight::Direction::Straight,
        EdgesInFlight::Direction::Right}) {
    os << EdgesInFlight::edgeChar(dir) << ' ' << edges.edges[toInt(dir)];
    if (!first) {
      os << ' ';
    }
    first = false;
  }
  return os;
}

std::optional<ParseError> EdgesInFlight::findDanglingEdge(size_t line) const {
  std::optional<ParseError> ret;

  for (auto dir : {Direction::Left, Direction::Straight, Direction::Right}) {
    for (auto const& [col, src] : edges[toInt(dir)]) {
      if (!ret || col < ret->pos.col) {
        ret = ParseError{
          ParseError::Code::DanglingEdge,
          "Dangling edge "s + edgeChar(dir) + " from " + std::to_string(src),
          {line, col}
        };
      }
    }
  }
  return ret;
}

class NodeCollector {
public:
  std::optional<ParseError> tryAddNode(EdgesInFlight& prevEdges, Position const& pos);

  void addNodeChar(char c) { partialNode.push_back(c); }

  bool isPartOfANode(size_t col) const;

  DAG buildDAG() && { return {std::move(nodes)}; }

  EdgeMap const& getPrevNodes() const { return prevNodes; }

  void newLine();

private:
  std::optional<ParseError> checkRectangularNewNode(Position const& pos);
  void startNewNode(EdgesInFlight& prevEdges, Position const& pos);
  std::optional<ParseError> checkRectangularNodeLine(size_t nodeAbove, Position const& pos);
  void addNodeLine(size_t nodeAbove, EdgesInFlight& prevEdges, Position const& pos);
  std::optional<size_t> findNodeAbove(size_t col);

  std::vector<DAG::Node> nodes = {};
  std::string partialNode = "";
  EdgeMap prevNodes;
  EdgeMap currNodes;
};

std::optional<ParseError> NodeCollector::tryAddNode(EdgesInFlight& prevEdges, Position const& pos) {
  if (partialNode.empty()) {
    return {};
  }
  if (auto nodeAbove = findNodeAbove(pos.col)) {
    if (auto err = checkRectangularNodeLine(*nodeAbove, pos)) {
      return err;
    }
    addNodeLine(*nodeAbove, prevEdges, pos);
  } else {
    if (auto err = checkRectangularNewNode(pos)) {
      return err;
    }
    startNewNode(prevEdges, pos);
  }
  return {};
}

bool NodeCollector::isPartOfANode(size_t col) const {
  return !partialNode.empty() && prevNodes.count(col - 1) != 0 && prevNodes.count(col) != 0;
}

void NodeCollector::newLine() {
  prevNodes = std::move(currNodes);
  currNodes = {};
}

std::optional<ParseError> NodeCollector::checkRectangularNewNode(Position const& pos) {
  for (size_t p = pos.col - partialNode.size() + 1; p < pos.col; ++p) {
    if (auto iter = prevNodes.find(p); iter != prevNodes.end()) {
      return {ParseError{
        ParseError::Code::NonRectangularNode,
        "Node-line above started midway node-line below.",
        {pos.line, p}
      }};
    }
  }
  return {};
}

void NodeCollector::startNewNode(EdgesInFlight& prevEdges, Position const& pos) {
  size_t id = nodes.size();
  for (size_t p = pos.col - partialNode.size(); p < pos.col; ++p) {
    for (auto from : prevEdges.findNRemoveEdgesToNode(p)) {
      nodes[from].outEdges.push_back({id});
    }
    currNodes[p] = id;
  }
  nodes.push_back({});
  nodes[id].text = partialNode;
  partialNode.clear();
}

std::optional<ParseError>
NodeCollector::checkRectangularNodeLine(size_t nodeAbove, Position const& pos) {
  for (size_t p = pos.col - partialNode.size(); p < pos.col; ++p) {
    if (auto iter = prevNodes.find(p); iter != prevNodes.end()) {
      // Node above can change only after a gap,
      // Two nodes "AA" and "BB" cannot follow each other immediately.
      // "AABB" would have been treated as a single node.
      assert(nodeAbove == iter->second);
    } else {
      return {ParseError{
        ParseError::Code::NonRectangularNode,
        "Node-line above ended midway node-line below.",
        {pos.line, p}
      }};
    }
  }
  if (prevNodes.count(pos.col - 1 - partialNode.size()) != 0) {
    return ParseError{
      ParseError::Code::NonRectangularNode,
      "Previous node-line was longer on the left side.",
      {pos.line, pos.col - 1 - partialNode.size()}
    };
  }
  if (prevNodes.count(pos.col) != 0) {
    return {ParseError{
      ParseError::Code::NonRectangularNode,
      "Previous node-line was longer on the right side.",
      pos
    }};
  }
  return {};
}

void NodeCollector::addNodeLine(size_t nodeAbove, EdgesInFlight& prevEdges, Position const& pos) {
  for (size_t p = pos.col - partialNode.size(); p < pos.col; ++p) {
    currNodes[p] = nodeAbove;
  }
  for (auto edge : prevEdges.findNRemoveEdgesToNode(pos.col - partialNode.size())) {
    nodes[edge].outEdges.push_back({nodeAbove});
  }
  for (auto edge : prevEdges.findNRemoveEdgesToNode(pos.col - 1)) {
    nodes[edge].outEdges.push_back({nodeAbove});
  }
  nodes[nodeAbove].text += "\n" + partialNode;
  partialNode.clear();
}

std::optional<size_t> NodeCollector::findNodeAbove(size_t col) {
  if (auto iter = prevNodes.find(col - partialNode.size()); iter != prevNodes.end()) {
    return iter->second;
  }
  return {};
}

} // namespace

std::string renderDAG(DAG const& dag) {
  std::ostringstream ret;
  // TODO: draw edges
  // TODO: find proper order of nodes
  // TODO: find best horisontal position of nodes
  for (auto const& layer : dagLayers(dag)) {
    for (size_t n : layer) {
      ret << n << " ";
    }
    ret << "\n";
  }
  return ret.str();
}

std::optional<DAG> parseDAG(std::string str, ParseError& err) {
  NodeCollector collector;
  EdgesInFlight prevEdges;
  EdgesInFlight currEdges;
  err.code = ParseError::Code::None;
  Position pos{0, 0};
  for (char c : str) {
    ++pos.col;
    if (c == '\n') {
      if (auto nodeErr = collector.tryAddNode(prevEdges, pos)) {
        err = *nodeErr;
        return std::nullopt;
      }
      if (auto dangling = prevEdges.findDanglingEdge(pos.line - 1)) {
        err = *dangling;
        return std::nullopt;
      }
      prevEdges = std::move(currEdges);
      currEdges = {};
      collector.newLine();
      pos.col = 0;
      ++pos.line;
    } else if (collector.isPartOfANode(pos.col)) {
      // Keep accumulating at least for as long as the node-line above
      collector.addNodeChar(c);
    } else if (auto dir = EdgesInFlight::edgeChar(c)) {
      // Continue the edge, if possible, before attaching one to a node
      auto fromNodes = prevEdges.findNRemoveEdgesToEdge(*dir, collector.getPrevNodes(), pos.col);
      if (auto e = currEdges.updateOrError(fromNodes, *dir, pos)) {
        err = *e;
        return std::nullopt;
      }
      if (auto nodeErr = collector.tryAddNode(prevEdges, pos)) {
        err = *nodeErr;
        return std::nullopt;
      }
    } else if (c == ' ') {
      if (auto nodeErr = collector.tryAddNode(prevEdges, pos)) {
        err = *nodeErr;
        return std::nullopt;
      }
    } else {
      collector.addNodeChar(c);
    }
  }
  if (auto dangling = prevEdges.findDanglingEdge(pos.line - 1)) {
    err = *dangling;
    return std::nullopt;
  }
  if (auto dangling = currEdges.findDanglingEdge(pos.line)) {
    err = *dangling;
    return std::nullopt;
  }
  return std::move(collector).buildDAG();
}

std::string parseErrorCodeToStr(ParseError::Code code) {
  using Code = ParseError::Code;
  switch (code) {
    case Code::DanglingEdge:
      return "DanglingEdge";
    case Code::MergingEdge:
      return "MergingEdge";
    case Code::SuspendedEdge:
      return "SuspendedEdge";
    case Code::NonRectangularNode:
      return "NonRectangularNode";
    case Code::None:
      return "None";
  }
  assert(false);
  return "Unexpected ParseError::Code";
}

std::ostream& operator<<(std::ostream& os, Position const& pos) {
  return os << pos.line << ":" << pos.col;
}

std::ostream& operator<<(std::ostream& os, ParseError const& err) {
  return os
      << "ERROR: " << parseErrorCodeToStr(err.code) << " at " << err.pos << ":" << err.message;
}

} // namespace asciidag
