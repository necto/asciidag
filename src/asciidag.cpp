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

struct EdgesInFlight {
  using EdgeMap = std::unordered_map<size_t, size_t>;

  EdgeMap straight;
  EdgeMap left;
  EdgeMap right;
  EdgeMap still;
};

std::set<size_t> findNRemoveEdgesToNode(EdgesInFlight& prevEdges, size_t pos) {
  std::set<size_t> ret;
  if (auto to = getIf(prevEdges.still, pos)) {
    ret.insert(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.straight, pos)) {
    ret.insert(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.left, pos + 1)) {
    ret.insert(*to);
  }
  if (auto to = findAndEraseIf(prevEdges.right, pos - 1)) {
    ret.insert(*to);
  }
  return ret;
}

std::optional<size_t> findNRemoveEdgesToPipe(EdgesInFlight& prevEdges, size_t pos) {
  if (auto to = getIf(prevEdges.still, pos)) {
    return *to;
  }
  if (auto to = findAndEraseIf(prevEdges.straight, pos)) {
    return *to;
  }
  if (auto to = findAndEraseIf(prevEdges.left, pos)) {
    return *to;
  }
  if (auto to = findAndEraseIf(prevEdges.right, pos)) {
    return *to;
  }
  return {};
}

std::optional<size_t> findNRemoveEdgesToBackslash(EdgesInFlight& prevEdges, size_t pos) {
  if (auto to = getIf(prevEdges.still, pos - 1)) {
    return *to;
  }
  if (auto to = findAndEraseIf(prevEdges.straight, pos)) {
    return *to;
  }
  if (auto to = findAndEraseIf(prevEdges.left, pos)) {
    return *to;
  }
  if (auto to = findAndEraseIf(prevEdges.right, pos - 1)) {
    return *to;
  }
  return {};
}

std::optional<size_t> findNRemoveEdgesToSlash(EdgesInFlight& prevEdges, size_t pos) {
  if (auto to = getIf(prevEdges.still, pos + 1)) {
    return *to;
  }
  if (auto to = findAndEraseIf(prevEdges.straight, pos)) {
    return *to;
  }
  if (auto to = findAndEraseIf(prevEdges.left, pos + 1)) {
    return *to;
  }
  if (auto to = findAndEraseIf(prevEdges.right, pos)) {
    return *to;
  }
  return {};
}

std::ostream& operator<<(std::ostream& os, EdgesInFlight const& edges) {
  return os
      << "." << edges.still << " |" << edges.straight << " /" << edges.left << " \\" << edges.right;
}

std::optional<ParseError> findDanglingEdge(EdgesInFlight const& edges, size_t line) {
  std::optional<ParseError> ret;
  auto keepLeftmost = [&ret, line](EdgesInFlight::EdgeMap const& edgeMap, std::string prefix) {
    for (auto const& [col, src] : edgeMap) {
      if (!ret || col < ret->col) {
        ret = ParseError{ParseError::Code::DanglingEdge, prefix + std::to_string(src), line, col};
      }
    }
  };
  keepLeftmost(edges.straight, "Dangling edge | from ");
  keepLeftmost(edges.left, "Dangling edge / from ");
  keepLeftmost(edges.right, "Dangling edge \\ from ");
  return ret;
}

std::optional<DAG> parseDAG(std::string str, ParseError& err) {
  // TODO: add error detection:
  // - merging edges:
  //       \|
  //        \
  // TODO: support multi-line nodes
  // - detect and report when a multi-line node isn't aligned
  // e.g.:   ###
  //        ###
  // should now be considered an error and not two linked nodes
  std::vector<DAG::Node> nodes;
  std::vector<char> partialNode;
  EdgesInFlight prevEdges;
  EdgesInFlight currEdges;
  size_t line = 0;
  auto addNode = [&nodes, &partialNode, &prevEdges, &currEdges](size_t col) {
    if (partialNode.empty()) {
      return;
    }
    size_t id = nodes.size();
    nodes.push_back({});
    for (size_t p = 0; p < partialNode.size(); ++p) {
      for (auto p : findNRemoveEdgesToNode(prevEdges, col - p)) {
        nodes[p].outEdges.push_back({id});
      }
      currEdges.still[col - p] = id;
    }
    partialNode.clear();
  };
  size_t col = 0;
  auto makeSuspendedError = [&line, &col, &err](char edgeChar) {
    return ParseError{
      ParseError::Code::SuspendedEdge,
      "Edge"s + edgeChar + "is suspended (not attached to any source node)",
      line,
      col};
  };
  for (char c : str) {
    ++col;
    switch (c) {
      case ' ':
        addNode(col - 1);
        break;
      case '\n':
        addNode(col - 1);
        if (auto dangling = findDanglingEdge(prevEdges, line - 1)) {
          err = *dangling;
          return std::nullopt;
        }
        prevEdges = std::move(currEdges);
        currEdges = {};
        col = 0;
        ++line;
        break;
      case '|':
        if (auto p = findNRemoveEdgesToPipe(prevEdges, col)) {
          currEdges.straight[col] = *p;
        } else {
          err = makeSuspendedError(c);
          return std::nullopt;
        }
        break;
      case '\\':
        if (auto p = findNRemoveEdgesToBackslash(prevEdges, col)) {
          currEdges.right[col] = *p;
        } else {
          err = makeSuspendedError(c);
          return std::nullopt;
        }
        break;
      case '/':
        if (auto p = findNRemoveEdgesToSlash(prevEdges, col)) {
          currEdges.left[col] = *p;
        } else {
          err = makeSuspendedError(c);
          return std::nullopt;
        }
        break;
      default:
        partialNode.push_back(c);
        break;
    }
  }
  if (auto dangling = findDanglingEdge(prevEdges, line - 1)) {
    err = *dangling;
    return std::nullopt;
  }
  if (auto dangling = findDanglingEdge(currEdges, line)) {
    err = *dangling;
    return std::nullopt;
  }
  return {{nodes}};
}

std::string parseCodeToStr(ParseError::Code code) {
  using Code = ParseError::Code;
  switch (code) {
    case Code::DanglingEdge:
      return "DanglingEdge";
  }
  assert(false);
  return "Unexpected ParseError::Code";
}

std::ostream& operator<<(std::ostream& os, ParseError const& err) {
  return os
      << "ERROR: " << parseCodeToStr(err.code) << " at " << err.line << ": " << err.col << ":"
      << err.message;
}
