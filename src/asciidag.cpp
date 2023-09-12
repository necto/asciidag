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

struct EdgesInFlight {
  EdgeMap straight;
  EdgeMap left;
  EdgeMap right;
};

std::vector<size_t> findNRemoveEdgesToNode(EdgesInFlight& prevEdges, size_t pos) {
  std::vector<size_t> ret;
  if (auto to = findAndEraseIf(prevEdges.straight, pos)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.left, pos + 1)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.right, pos - 1)) {
    ret.push_back(*to);
  }
  return ret;
}

std::vector<size_t>
findNRemoveEdgesToPipe(EdgesInFlight& prevEdges, EdgeMap const& prevNodes, size_t pos) {
  std::vector<size_t> ret;
  if (auto to = getIf(prevNodes, pos)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.straight, pos)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.left, pos)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.right, pos)) {
    ret.push_back(*to);
  }
  return ret;
}

std::vector<size_t>
findNRemoveEdgesToBackslash(EdgesInFlight& prevEdges, EdgeMap const& prevNodes, size_t pos) {
  std::vector<size_t> ret;
  if (auto to = getIf(prevNodes, pos - 1)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.straight, pos)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.left, pos)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.right, pos - 1)) {
    ret.push_back(*to);
  }
  return ret;
}

std::vector<size_t>
findNRemoveEdgesToSlash(EdgesInFlight& prevEdges, EdgeMap const& prevNodes, size_t pos) {
  std::vector<size_t> ret;
  if (auto to = getIf(prevNodes, pos + 1)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.straight, pos)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.left, pos + 1)) {
    ret.push_back(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.right, pos)) {
    ret.push_back(*to);
  }
  return ret;
}

std::ostream& operator<<(std::ostream& os, EdgesInFlight const& edges) {
  return os << " |" << edges.straight << " /" << edges.left << " \\" << edges.right;
}

std::optional<ParseError> findDanglingEdge(EdgesInFlight const& edges, size_t line) {
  std::optional<ParseError> ret;
  auto keepLeftmost = [&ret, line](EdgeMap const& edgeMap, std::string prefix) {
    for (auto const& [col, src] : edgeMap) {
      if (!ret || col < ret->pos.col) {
        ret = ParseError{ParseError::Code::DanglingEdge, prefix + std::to_string(src), {line, col}};
      }
    }
  };
  keepLeftmost(edges.straight, "Dangling edge | from ");
  keepLeftmost(edges.left, "Dangling edge / from ");
  keepLeftmost(edges.right, "Dangling edge \\ from ");
  return ret;
}

class NodeCollector {
  std::optional<ParseError> startNewNode(EdgesInFlight& prevEdges, Position const& pos) {
    size_t id = nodes.size();
    for (size_t p = pos.col - partialNode.size(); p < pos.col; ++p) {
      if (auto iter = prevNodes.find(p); iter != prevNodes.end()) {
        return {ParseError{
          ParseError::Code::NonRectangularNode,
          "Node-line above started midway node-line below.",
          {pos.line, p}
        }};
      }
      for (auto p : findNRemoveEdgesToNode(prevEdges, p)) {
        nodes[p].outEdges.push_back({id});
      }
      currNodes[p] = id;
    }
    nodes.push_back({});
    nodes[id].text = partialNode;
    partialNode.clear();
    return {};
  }

  std::optional<ParseError>
  addNodeLine(size_t nodeAbove, EdgesInFlight& prevEdges, Position const& pos) {
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
      currNodes[p] = nodeAbove;
    }
    for (auto edge : findNRemoveEdgesToNode(prevEdges, pos.col - partialNode.size())) {
      nodes[edge].outEdges.push_back({nodeAbove});
    }
    for (auto edge : findNRemoveEdgesToNode(prevEdges, pos.col - 1)) {
      nodes[edge].outEdges.push_back({nodeAbove});
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
    nodes[nodeAbove].text += "\n" + partialNode;
    partialNode.clear();
    return {};
  }

  std::optional<size_t> findNodeAbove(size_t col) {
    if (auto iter = prevNodes.find(col - partialNode.size()); iter != prevNodes.end()) {
      return iter->second;
    }
    return {};
  }

public:
  std::optional<ParseError> addNode(EdgesInFlight& prevEdges, Position const& pos) {
    if (partialNode.empty()) {
      return {};
    }
    if (auto nodeAbove = findNodeAbove(pos.col)) {
      return addNodeLine(*nodeAbove, prevEdges, pos);
    }
    return startNewNode(prevEdges, pos);
  }

  void addNodeChar(char c) { partialNode.push_back(c); }

  bool curLineNonEmpty() const { return !partialNode.empty(); }

  DAG buildDAG() { return {nodes}; }

  EdgeMap const& getPrevNodes() const { return prevNodes; }

  void newLine() {
    prevNodes = std::move(currNodes);
    currNodes = {};
  }

private:
  std::vector<DAG::Node> nodes = {};
  std::string partialNode = "";
  EdgeMap prevNodes;
  EdgeMap currNodes;
};

std::optional<DAG> parseDAG(std::string str, ParseError& err) {
  NodeCollector collector;
  EdgesInFlight prevEdges;
  EdgesInFlight currEdges;
  err.code = ParseError::Code::None;
  Position pos{0, 0};
  auto makeSuspendedError = [&pos](char edgeChar) {
    return ParseError{
      ParseError::Code::SuspendedEdge,
      "Edge"s + edgeChar + "is suspended (not attached to any source node)",
      pos
    };
  };
  auto makeMergeError = [&pos]() {
    return ParseError{ParseError::Code::MergingEdge, "Edges merged into one edge", pos};
  };
  for (char c : str) {
    ++pos.col;
    if (c != '\n' && collector.curLineNonEmpty() && collector.getPrevNodes().count(pos.col - 1) != 0 && collector.getPrevNodes().count(pos.col) != 0) {
      // Keep accumulating at least for as long as the node-line above
      collector.addNodeChar(c);
      continue;
    }
    switch (c) {
      case ' ': {
        if (auto nodeErr = collector.addNode(prevEdges, pos)) {
          err = *nodeErr;
          return std::nullopt;
        }
        break;
      }
      case '\n':
        if (auto nodeErr = collector.addNode(prevEdges, pos)) {
          err = *nodeErr;
          return std::nullopt;
        }
        if (auto dangling = findDanglingEdge(prevEdges, pos.line - 1)) {
          err = *dangling;
          return std::nullopt;
        }
        prevEdges = std::move(currEdges);
        currEdges = {};
        collector.newLine();
        pos.col = 0;
        ++pos.line;
        break;
      case '|': {
        if (auto nodeErr = collector.addNode(prevEdges, pos)) {
          err = *nodeErr;
          return std::nullopt;
        }
        auto fromNodes = findNRemoveEdgesToPipe(prevEdges, collector.getPrevNodes(), pos.col);
        if (fromNodes.size() == 1) {
          currEdges.straight[pos.col] = fromNodes.front();
        } else if (fromNodes.size() == 0) {
          err = makeSuspendedError(c);
          return std::nullopt;
        } else {
          err = makeMergeError();
          return std::nullopt;
        }
        break;
      }
      case '\\': {
        if (auto nodeErr = collector.addNode(prevEdges, pos)) {
          err = *nodeErr;
          return std::nullopt;
        }
        auto fromNodes = findNRemoveEdgesToBackslash(prevEdges, collector.getPrevNodes(), pos.col);
        if (fromNodes.size() == 1) {
          currEdges.right[pos.col] = fromNodes.front();
        } else if (fromNodes.size() == 0) {
          err = makeSuspendedError(c);
          return std::nullopt;
        } else {
          err = makeMergeError();
          return std::nullopt;
        }
        break;
      }
      case '/': {
        if (auto nodeErr = collector.addNode(prevEdges, pos)) {
          err = *nodeErr;
          return std::nullopt;
        }
        auto fromNodes = findNRemoveEdgesToSlash(prevEdges, collector.getPrevNodes(), pos.col);
        if (fromNodes.size() == 1) {
          currEdges.left[pos.col] = fromNodes.front();
        } else if (fromNodes.size() == 0) {
          err = makeSuspendedError(c);
          return std::nullopt;
        } else {
          err = makeMergeError();
          return std::nullopt;
        }
        break;
      }
      default:
        collector.addNodeChar(c);
        break;
    }
  }
  if (auto dangling = findDanglingEdge(prevEdges, pos.line - 1)) {
    err = *dangling;
    return std::nullopt;
  }
  if (auto dangling = findDanglingEdge(currEdges, pos.line)) {
    err = *dangling;
    return std::nullopt;
  }
  return collector.buildDAG();
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
