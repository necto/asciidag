#include "asciidag.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

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

// enum class Momentum { Straight, Left, Right, Still };

// struct EdgeInFlight {
//   Momentum momentum;
//   size_t node;
// };

template <typename K, typename V>
V const* getIf(std::unordered_map<K, V> const& map, K const& k) {
  if (auto iter = map.find(k); iter != map.end()) {
    return &iter->second;
  }
  return nullptr;
}

struct EdgesInFlight {
  std::unordered_map<size_t, size_t> straight;
  std::unordered_map<size_t, size_t> left;
  std::unordered_map<size_t, size_t> right;
  std::unordered_map<size_t, size_t> still;
};

std::set<size_t> convergentOnNode(EdgesInFlight const& prevEdges, size_t pos) {
  std::set<size_t> ret;
  if (auto to = getIf(prevEdges.straight, pos)) {
    ret.insert(*to);
  }
  if (auto to = getIf(prevEdges.still, pos)) {
    ret.insert(*to);
  }
  if (auto to = getIf(prevEdges.left, pos + 1)) {
    ret.insert(*to);
  }
  if (auto to = getIf(prevEdges.right, pos - 1)) {
    ret.insert(*to);
  }
  return ret;
}

std::optional<size_t> convergentOnPipe(EdgesInFlight const& prevEdges, size_t pos) {
  if (auto to = getIf(prevEdges.straight, pos)) {
    return *to;
  }
  if (auto to = getIf(prevEdges.still, pos)) {
    return *to;
  }
  if (auto to = getIf(prevEdges.left, pos)) {
    return *to;
  }
  if (auto to = getIf(prevEdges.right, pos)) {
    return *to;
  }
  return {};
}

std::optional<size_t> convergentOnBackslash(EdgesInFlight const& prevEdges, size_t pos) {
  if (auto to = getIf(prevEdges.straight, pos)) {
    return *to;
  }
  if (auto to = getIf(prevEdges.still, pos - 1)) {
    return *to;
  }
  if (auto to = getIf(prevEdges.left, pos)) {
    return *to;
  }
  if (auto to = getIf(prevEdges.right, pos - 1)) {
    return *to;
  }
  return {};
}

std::optional<size_t> convergentOnSlash(EdgesInFlight const& prevEdges, size_t pos) {
  if (auto to = getIf(prevEdges.straight, pos)) {
    return *to;
  }
  if (auto to = getIf(prevEdges.still, pos + 1)) {
    return *to;
  }
  if (auto to = getIf(prevEdges.left, pos + 1)) {
    return *to;
  }
  if (auto to = getIf(prevEdges.right, pos)) {
    return *to;
  }
  return {};
}

std::ostream& operator<<(std::ostream& os, EdgesInFlight const& edges) {
  return os
      << "'.'" << edges.still << "'|'" << edges.straight << "'/'" << edges.left << "'\\'"
      << edges.right;
}

DAG parseDAG(std::string str) {
  std::vector<DAG::Node> nodes;
  std::vector<char> partialNode;
  EdgesInFlight prevEdges;
  EdgesInFlight currEdges;
  auto addNode = [&nodes, &partialNode, &prevEdges, &currEdges](size_t col) {
    if (partialNode.empty()) {
      return;
    }
    size_t id = nodes.size();
    nodes.push_back({});
    for (auto p : convergentOnNode(prevEdges, col)) {
      nodes[p].outEdges.push_back({id});
    }
    for (size_t p = 0; p < partialNode.size(); ++p) {
      currEdges.still[col + p] = id;
    }
    partialNode.clear();
  };
  size_t col = 0;
  for (char c : str) {
    ++col;
    switch (c) {
      case ' ':
        addNode(col - 1);
        break;
      case '\n':
        addNode(col - 1);
        prevEdges = std::move(currEdges);
        currEdges = {};
        col = 0;
        break;
      case '|':
        if (auto p = convergentOnPipe(prevEdges, col)) {
          currEdges.straight[col] = *p;
        }
        break;
      case '\\':
        if (auto p = convergentOnBackslash(prevEdges, col)) {
          currEdges.right[col] = *p;
        }
        break;
      case '/':
        if (auto p = convergentOnSlash(prevEdges, col)) {
          currEdges.left[col] = *p;
        }
        break;
      default:
        partialNode.push_back(c);
        break;
    }
  }
  return {nodes};
}
