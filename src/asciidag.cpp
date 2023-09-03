#include "asciidag.h"

#include <algorithm>
#include <iostream>
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

template<typename T>
void append(std::vector<T> &to, std::vector<T> const& from) {
  for (auto const& x : from) {
    to.push_back(x);
  }
}

template<typename T>
void add(std::set<T> &to, std::set<T> const& from) {
  for (auto const& x : from) {
    to.insert(x);
  }
}

DAG parseDAG(std::string str) {
  std::vector<DAG::Node> nodes;
  std::vector<char> partialNode;
  std::unordered_map<size_t, std::set<size_t>> prevPositions;
  std::unordered_map<size_t, std::set<size_t>> curPositions;
  std::unordered_map<size_t, std::set<size_t>> prevObliquePositions;
  std::unordered_map<size_t, std::set<size_t>> curObliquePositions;
  std::unordered_map<size_t, std::set<size_t>> prevPipePositions;
  std::unordered_map<size_t, std::set<size_t>> curPipePositions;
  auto addNode =
    [&nodes, &partialNode, &prevPositions, &curPositions](
      size_t col
    ) {
      if (partialNode.empty()) {
        return;
      }
      size_t id = nodes.size();
      nodes.push_back({});
      for (auto p : prevPositions[col]) {
        nodes[p].outEdges.push_back({id});
      }
      for (size_t p = 0; p < partialNode.size(); ++p) {
        curPositions[col + p].insert(id);
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
        std::swap(prevPositions, curPositions);
        std::swap(prevObliquePositions, curObliquePositions);
        std::swap(prevPipePositions, curPipePositions);
        curPositions.clear();
        curObliquePositions.clear();
        curPipePositions.clear();
        col = 0;
        break;
      case '|':
        add(curPositions[col], prevPositions[col]);
        add(curPositions[col], prevObliquePositions[col]);
        add(curPositions[col], prevPipePositions[col]);

        add(curPipePositions[col], prevPositions[col]);
        add(curPipePositions[col], prevObliquePositions[col]);
        add(curPipePositions[col], prevPipePositions[col]);
        break;
      case '\\':
        add(curPositions[col + 1], prevPositions[col - 1]);
        add(curPositions[col + 1], prevPipePositions[col]);
        add(curPositions[col + 1], prevObliquePositions[col - 1]);

        add(curObliquePositions[col], prevPositions[col - 1]);
        add(curObliquePositions[col], prevObliquePositions[col - 1]);
        add(curObliquePositions[col], prevPipePositions[col]);
        break;
      case '/':
        add(curPositions[col - 1], prevPositions[col + 1]);
        add(curPositions[col - 1], prevPipePositions[col]);
        add(curPositions[col - 1], prevObliquePositions[col + 1]);

        add(curObliquePositions[col], prevPositions[col + 1]);
        add(curObliquePositions[col], prevObliquePositions[col + 1]);
        add(curObliquePositions[col], prevPipePositions[col]);
        break;
      default:
        partialNode.push_back(c);
        break;
    }
  }
  return {nodes};
}
