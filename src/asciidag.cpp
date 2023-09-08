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

std::vector<size_t> findNRemoveEdgesToNode(EdgesInFlight& prevEdges, size_t pos) {
  std::vector<size_t> ret;
  if (auto to = getIf(prevEdges.still, pos)) {
    // TODO: this should be forbidden
    ret.push_back(*to);
  }
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

std::vector<size_t> findNRemoveEdgesToPipe(EdgesInFlight& prevEdges, size_t pos) {
  std::vector<size_t> ret;
  if (auto to = getIf(prevEdges.still, pos)) {
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

std::vector<size_t> findNRemoveEdgesToBackslash(EdgesInFlight& prevEdges, size_t pos) {
  std::vector<size_t> ret;
  if (auto to = getIf(prevEdges.still, pos - 1)) {
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

std::vector<size_t> findNRemoveEdgesToSlash(EdgesInFlight& prevEdges, size_t pos) {
  std::vector<size_t> ret;
  if (auto to = getIf(prevEdges.still, pos + 1)) {
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
  // TODO: support multi-line nodes
  // - detect and report when a multi-line node isn't aligned
  // e.g.:   ###
  //        ###
  // should now be considered an error and not two linked nodes
  std::vector<DAG::Node> nodes;
  std::string partialNode;
  EdgesInFlight prevEdges;
  EdgesInFlight currEdges;
  err.code = ParseError::Code::None;
  size_t line = 0;
  auto addNode = [&nodes, &partialNode, &prevEdges, &currEdges](size_t col) {
    std::optional<ParseError> ret;
    if (partialNode.empty()) {
      return ret;
    }
    size_t id = nodes.size();
    std::optional<size_t> nodeAbove;
    for (size_t p = col - partialNode.size() + 1; p <= col; ++p) {
      if (auto iter = prevEdges.still.find(p); iter != prevEdges.still.end()) {
        if (nodeAbove && *nodeAbove != iter->second) {
          // TODO: ERROR! node above changed surprisingly. but this can't happen with no gap though
        }
        nodeAbove = iter->second;
      } else if (nodeAbove) {
        // TODO: ERROR! was was and suddnly no node above
      }
      if (!nodeAbove) {
        for (auto p : findNRemoveEdgesToNode(prevEdges, p)) {
          nodes[p].outEdges.push_back({id});
        }
      }
      if (nodeAbove) {
        currEdges.still[p] = *nodeAbove;
      } else {
        currEdges.still[p] = id;
      }
    }
    if (nodeAbove) {
      if (prevEdges.still.count(col - partialNode.size()) != 0) {
        // TODO: ERROR! previous node line was longer on left
      }
      if (prevEdges.still.count(col + 1) != 0) {
        // TODO: ERROR! previous node line was longer on right
      }
      nodes[*nodeAbove].text += "\n" + partialNode;
    } else {
      nodes.push_back({});
      nodes[id].text = partialNode;
    }
    partialNode.clear();
    return ret;
  };
  size_t col = 0;
  auto makeSuspendedError = [&line, &col, &err](char edgeChar) {
    return ParseError{
      ParseError::Code::SuspendedEdge,
      "Edge"s + edgeChar + "is suspended (not attached to any source node)",
      line,
      col};
  };
  auto makeMergeError = [&line, &col, &err]() {
    return ParseError{ParseError::Code::MergingEdge, "Edges merged into one edge", line, col};
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
      case '|': {
        auto fromNodes = findNRemoveEdgesToPipe(prevEdges, col);
        if (fromNodes.size() == 1) {
          currEdges.straight[col] = fromNodes.front();
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
        auto fromNodes = findNRemoveEdgesToBackslash(prevEdges, col);
        if (fromNodes.size() == 1) {
          currEdges.right[col] = fromNodes.front();
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
        auto fromNodes = findNRemoveEdgesToSlash(prevEdges, col);
        if (fromNodes.size() == 1) {
          currEdges.left[col] = fromNodes.front();
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
