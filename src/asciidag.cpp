#include "asciidag.h"

#include "asciidagImpl.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace asciidag {

namespace {

using namespace std::string_literals;

std::vector<std::vector<size_t>> dagLayers(DAG const& dag) {
  // TODO: optimize using a queue (linear in edges)
  // instead of a fix-point (quadratic in edges)
  std::vector<size_t> rank(dag.nodes.size(), 0);
  bool changed = false;
  do {
    changed = false;
    for (size_t n = 0; n < dag.nodes.size(); ++n) {
      for (auto const& e : dag.nodes[n].succs) {
        if (rank[e] < rank[n] + 1) {
          changed = true;
          rank[e] = rank[n] + 1;
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

std::optional<RenderError> insertEdgeWaypoints(DAG& dag, std::vector<std::vector<size_t>>& layers) {
  size_t const realNodeN = dag.nodes.size();
  std::vector<size_t> rank(realNodeN, 0);
  for (size_t layerI = 0; layerI < layers.size(); ++layerI) {
    for (size_t n : layers[layerI]) {
      rank[n] = layerI;
    }
  }
  for (size_t layerI = 0; layerI < layers.size(); ++layerI) {
    for (size_t n : layers[layerI]) {
      if (realNodeN <= n) {
        // This is a waypoint that by construction has its edge targeting the next layer
        assert(dag.nodes[n].succs.size() == 1);
        assert(realNodeN <= dag.nodes[n].succs[0] || rank[dag.nodes[n].succs[0]] == layerI + 1);
        continue;
      }
      for (size_t& e : dag.nodes[n].succs) {
        assert(layerI < rank[e]);
        // TODO: optimization:
        // if (layerI + 1 == rank[e]) {
        //   continue;
        // }
        size_t finalSucc = e;
        size_t* lastEdge = &e;
        for (auto l = layerI + 1; l < rank[finalSucc]; ++l) {
          size_t nodeId = dag.nodes.size();
          *lastEdge = nodeId;
          dag.nodes.push_back({{0}, "."});
          layers[l].push_back(nodeId);
          lastEdge = &dag.nodes.back().succs.back();
          // No need to add it to rank
        }
        *lastEdge = finalSucc;
      }
    }
  }
  return {};
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
  static std::optional<Direction> edgeChar(char c);
  static char edgeChar(Direction dir);

  std::vector<size_t> findNRemoveEdgesToNode(size_t col);
  std::optional<size_t> findNRemoveEdgeToEdge(Direction dir, EdgeMap const& prevNodes, size_t col);
  std::optional<ParseError> findDanglingEdge(size_t line) const;
  friend std::ostream& operator<<(std::ostream& os, EdgesInFlight const& edges);

  std::optional<ParseError>
  updateOrError(std::optional<size_t> fromNode, Direction dir, Position const& pos);

private:

  /// edges[0] stores an empty map (just for padding)
  /// The edges[1]..edges[3] corresponds to the Direction options
  std::array<EdgeMap, 4> edges;
};

std::optional<Direction> EdgesInFlight::edgeChar(char c) {
  switch (c) {
    case '|':
      return Direction::Straight;
    case '\\':
      return Direction::Right;
    case '/':
      return Direction::Left;
    default:
      return {};
  }
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

int toInt(Direction dir) {
  return static_cast<int>(dir);
}

std::optional<ParseError>
EdgesInFlight::updateOrError(std::optional<size_t> fromNodes, Direction dir, Position const& pos) {
  if (fromNodes) {
    edges[toInt(dir)][pos.col] = *fromNodes;
    return {};
  }
  return ParseError{
    ParseError::Code::SuspendedEdge,
    "Edge "s + edgeChar(dir) + " is suspended (not attached to any source node)",
    pos
  };
}

/// [Node, Left, Straight, Right]
static std::array<std::array<int, /* line above */ 4>, /* line below */ 4> const columnShift = {{
  {0, +1, 0, -1}, // no format
  {+1, +1, 0, 0}, // no format
  {0, 0, 0, 0}, // no format
  {-1, 0, 0, -1} // no format
}};

std::vector<size_t> EdgesInFlight::findNRemoveEdgesToNode(size_t col) {
  std::vector<size_t> ret;
  for (auto dir : {toInt(Direction::Left), toInt(Direction::Straight), toInt(Direction::Right)}) {
    if (auto to = findAndEraseIf(edges[dir], col + columnShift[dir][0])) {
      ret.push_back(*to);
    }
  }
  return ret;
}

std::optional<size_t>
EdgesInFlight::findNRemoveEdgeToEdge(Direction dirBelow, EdgeMap const& prevNodes, size_t col) {
  // Important to order them in order they can appear on the previous line,
  // from left to right, i.e., \(Right), |(Straight), /(Left)
  for (auto dirAbove :
       {toInt(Direction::Right), toInt(Direction::Straight), toInt(Direction::Left)}) {
    if (auto from = findAndEraseIf(edges[dirAbove], col + columnShift[dirAbove][toInt(dirBelow)])) {
      return *from;
    }
  }
  // The early return above guarantees that an edge is not connected to a node
  // if it is already connected to an edge
  if (auto to = getIf(prevNodes, col + columnShift[0][toInt(dirBelow)])) {
    return *to;
  }
  return std::nullopt;
}

[[maybe_unused]]
std::ostream&
operator<<(std::ostream& os, EdgesInFlight const& edges) {
  bool first = true;
  for (auto dir : {Direction::Left, Direction::Straight, Direction::Right}) {
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

  DAG buildDAG() && {
    assert(finalized);
    return {std::move(nodes)};
  }

  EdgeMap const& getPrevNodes() const { return prevNodes; }

  void newLine();

  std::optional<ParseError> finalize();

private:
  std::optional<ParseError> checkRectangularNewNode(Position const& pos);
  void startNewNode(EdgesInFlight& prevEdges, Position const& pos);
  std::optional<ParseError> checkRectangularNodeLine(size_t nodeAbove, Position const& pos);
  void addNodeLine(size_t nodeAbove, EdgesInFlight& prevEdges, Position const& pos);
  std::optional<size_t> findNodeAbove(size_t col);

  std::vector<DAG::Node> nodes = {};
  std::vector<Position> nodePositions = {};
  std::string partialNode = "";
  EdgeMap prevNodes;
  EdgeMap currNodes;
  bool finalized = false;
};

bool hasCrossEdges(std::vector<DAG::Node> const& nodes) {
  return std::any_of(nodes.begin(), nodes.end(), [](DAG::Node const& n) { return n.text == "X"; });
}

void replace(std::vector<size_t>& edges, size_t dated, size_t updated) {
  for (auto& e : edges) {
    if (e == dated) {
      e = updated;
    }
  }
}

std::optional<ParseError> validateEdgeCrossings(
  std::vector<DAG::Node> const& nodes,
  std::vector<Position> const& nodePositions
) {
  assert(nodes.size() == nodePositions.size());
  std::unordered_map<size_t, std::vector<size_t>> preds; // TODO: change to vector of vectors
  for (size_t nodeId = 0; nodeId < nodes.size(); ++nodeId) {
    if (nodes[nodeId].text == "X") {
      auto fromIter = preds.find(nodeId);
      if (fromIter == preds.end()) {
        return {
          {ParseError::Code::SuspendedEdge,
           "Edge crossing misses both incoming edges.",
           nodePositions[nodeId]}
        };
      }
      auto& from = fromIter->second;
      if (from.size() == 1 || from.size() < nodes[nodeId].succs.size()) {
        return {
          {ParseError::Code::SuspendedEdge,
           "Edge crossing misses an incoming edge.",
           nodePositions[nodeId]}
        };
      }
      if (nodes[nodeId].succs.size() == 1 || nodes[nodeId].succs.size() < from.size()) {
        return {
          {ParseError::Code::DanglingEdge,
           "Edge crossing misses an outgoing edge.",
           nodePositions[nodeId]}
        };
      }
      assert(from.size() == 2 || from.size() == 3);
      assert(nodes[nodeId].succs.size() == 2 || nodes[nodeId].succs.size() == 3);
    }
    for (auto const& e : nodes[nodeId].succs) {
      preds[e].push_back(nodeId);
    }
  }
  return {};
}

std::vector<DAG::Node> resolveCrossEdges(std::vector<DAG::Node> nodes) {
  size_t nSkipped = 0;
  std::unordered_map<size_t, std::vector<size_t>> preds; // TODO: change to vector of vectors
  std::vector<size_t> idMap(nodes.size());
  for (size_t i = 0; i < nodes.size(); ++i) {
    if (nodes[i].text == "X") {
      // Assertions are ensured by "validateEdgeCrossings"
      assert(preds.count(i) != 0);
      auto& from = preds[i];
      if (from.size() == 2) {
        assert(nodes[i].succs.size() == 2);
        replace(nodes[from[0]].succs, i, nodes[i].succs[1]);
        replace(nodes[from[1]].succs, i, nodes[i].succs[0]);
      } else {
        assert(from.size() == 3);
        assert(nodes[i].succs.size() == 3);

        replace(nodes[from[0]].succs, i, nodes[i].succs[2]);
        replace(nodes[from[1]].succs, i, nodes[i].succs[1]);
        replace(nodes[from[2]].succs, i, nodes[i].succs[0]);
      }
      ++nSkipped;
    }
    for (auto const& e : nodes[i].succs) {
      preds[e].push_back(i);
    }
    idMap[i] = i - nSkipped;
  }
  nodes.erase(
    std::remove_if(nodes.begin(), nodes.end(), [](DAG::Node const& n) { return n.text == "X"; }),
    nodes.end()
  );
  for (auto& n : nodes) {
    for (auto& e : n.succs) {
      e = idMap[e];
    }
  }
  return nodes;
}

std::optional<ParseError> NodeCollector::finalize() {
  if (hasCrossEdges(nodes)) {
    if (auto err = validateEdgeCrossings(nodes, nodePositions)) {
      return err;
    }
    nodes = resolveCrossEdges(std::move(nodes));
  }
  finalized = true;
  return {};
}

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
      nodes[from].succs.push_back({id});
    }
    currNodes[p] = id;
  }
  nodes.emplace_back();
  nodes[id].text = partialNode;
  partialNode.clear();
  nodePositions.push_back(pos);
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
    nodes[edge].succs.push_back({nodeAbove});
  }
  for (auto edge : prevEdges.findNRemoveEdgesToNode(pos.col - 1)) {
    nodes[edge].succs.push_back({nodeAbove});
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

struct Connectivity {
  struct Edge {
    size_t from;
    Direction exitAngle;
    size_t to;
    Direction entryAngle;
  };

  struct Valency {
    bool topLeft = false;
    bool topRight = false;
    bool bottomLeft = false;
    bool bottomRight = false;
  };

  std::vector<Edge> edges;
  std::vector<Valency> nodeValencies;
};

size_t absDiff(size_t a, size_t b) {
  return a > b ? a - b : b - a;
}

int directionShift(Direction dir) {
  switch (dir) {
    case Direction::Left:
      return -1;
    case Direction::Straight:
      return 0;
    case Direction::Right:
      return +1;
  }
  assert(false && "Unexpected Direction");
  return 0;
}

size_t minEdgeHeight(Connectivity::Edge const& edge, std::vector<Position> const& positions) {
  return absDiff(
           positions[edge.from].col + directionShift(edge.entryAngle),
           positions[edge.to].col - directionShift(edge.exitAngle)
         )
       + 3;
}

size_t minDistBetweenLayers(
  Connectivity const& conn,
  std::vector<size_t> const& edges,
  std::vector<Position> const& positions
) {
  size_t ret = 1; // At least 1 '|' must separate any two connected nodes
  for (auto eId : edges) {
    size_t hight = minEdgeHeight(conn.edges[eId], positions);
    if (ret < hight) {
      ret = hight;
    }
  }
  return ret;
}

std::pair<size_t, size_t> choseStraightAndRight(size_t zero, size_t one) {
  if (zero <= one) {
    return {0, 1};
  }
  return {1, 0};
}

bool inOrder(size_t a, size_t b, size_t c) {
  return a <= b && b <= c;
}

std::tuple<size_t, size_t, size_t> choseLeftStraightRight(size_t zero, size_t one, size_t two) {
  if (inOrder(zero, one, two)) {
    return {0, 1, 2};
  }
  if (inOrder(zero, two, one)) {
    return {0, 2, 1};
  }
  if (inOrder(one, zero, two)) {
    return {1, 0, 2};
  }
  if (inOrder(one, two, zero)) {
    return {1, 2, 0};
  }
  if (inOrder(two, zero, one)) {
    return {1, 2, 0};
  }
  assert(inOrder(two, one, zero));
  return {2, 1, 0};
}

void setEntryAngles(
  Connectivity& conn,
  std::vector<std::vector<size_t>> predEdges,
  std::vector<Position> const& coords
) {
  for (size_t i = 0; i < predEdges.size(); ++i) {
    auto const& edgeIds = predEdges[i];
    switch (edgeIds.size()) {
      case 0:
        break;
      case 1: {
        conn.edges[edgeIds[0]].entryAngle = Direction::Straight;
        break;
      }
      case 2: {
        auto [straight, right] = choseStraightAndRight(
          coords[conn.edges[edgeIds[0]].from].col,
          coords[conn.edges[edgeIds[1]].from].col
        );
        conn.edges[edgeIds[straight]].entryAngle = Direction::Straight;
        conn.edges[edgeIds[right]].entryAngle = Direction::Left;
        conn.nodeValencies[i].topRight = true;
        break;
      }
      case 3: {
        auto [left, straight, right] = choseLeftStraightRight(
          coords[conn.edges[edgeIds[0]].from].col,
          coords[conn.edges[edgeIds[1]].from].col,
          coords[conn.edges[edgeIds[2]].from].col
        );
        conn.edges[edgeIds[left]].entryAngle = Direction::Right;
        conn.edges[edgeIds[straight]].entryAngle = Direction::Straight;
        conn.edges[edgeIds[right]].entryAngle = Direction::Left;
        conn.nodeValencies[i].topLeft = true;
        conn.nodeValencies[i].topRight = true;
        break;
      }
      default:
        assert(false && "Overcrowded node");
        break;
    }
  }
}

void setExitAngles(
  Connectivity& conn,
  std::vector<std::vector<size_t>> succEdges,
  std::vector<Position> const& coords
) {
  for (size_t i = 0; i < succEdges.size(); ++i) {
    auto const& edgeIds = succEdges[i];
    switch (edgeIds.size()) {
      case 0:
        break;
      case 1: {
        conn.edges[edgeIds[0]].exitAngle = Direction::Straight;
        break;
      }
      case 2: {
        auto [straight, right] = choseStraightAndRight(
          coords[conn.edges[edgeIds[0]].to].col,
          coords[conn.edges[edgeIds[1]].to].col
        );
        conn.edges[edgeIds[straight]].exitAngle = Direction::Straight;
        conn.edges[edgeIds[right]].exitAngle = Direction::Right;
        conn.nodeValencies[i].bottomRight = true;
        break;
      }
      case 3: {
        auto [left, straight, right] = choseLeftStraightRight(
          coords[conn.edges[edgeIds[0]].to].col,
          coords[conn.edges[edgeIds[1]].to].col,
          coords[conn.edges[edgeIds[2]].to].col
        );
        conn.edges[edgeIds[left]].exitAngle = Direction::Left;
        conn.edges[edgeIds[straight]].exitAngle = Direction::Straight;
        conn.edges[edgeIds[right]].exitAngle = Direction::Right;
        conn.nodeValencies[i].bottomLeft = true;
        conn.nodeValencies[i].bottomRight = true;
        break;
      }
      default:
        assert(false && "Overcrowded node");
        break;
    }
  }
}

Connectivity computeConnectivity(DAG const& dag, std::vector<Position> const& coords) {
  size_t const N = dag.nodes.size();
  std::vector<std::vector<size_t>> preds(N);
  std::vector<std::vector<size_t>> predEdges(N);
  std::vector<std::vector<size_t>> succEdges(N);
  Connectivity ret;
  ret.nodeValencies.resize(N);
  for (size_t i = 0; i < N; ++i) {
    for (size_t e : dag.nodes[i].succs) {
      preds[e].push_back(i);
      size_t edgeId = ret.edges.size();
      predEdges[e].push_back(edgeId);
      succEdges[i].push_back(edgeId);
      // The angles is not correct there yet
      ret.edges.push_back({i, Direction::Straight, e, Direction::Straight});
    }
  }
  for (auto& edge : ret.edges) {
    assert(dag.nodes[edge.from].succs.size() <= 3 && "Overcrowded node");
    assert(preds[edge.to].size() <= 3 && "Overcrowded node");
    assert(1 <= dag.nodes[edge.from].succs.size() && "Fanthom edge");
    assert(1 <= preds[edge.to].size() && "Fanthom edge");
  }
  setEntryAngles(ret, predEdges, coords);
  setExitAngles(ret, succEdges, coords);
  return ret;
}

std::vector<Position>
computeNodeCoordinates(DAG const& dag, std::vector<std::vector<size_t>> const& layers) {
  std::vector<Position> ret(dag.nodes.size(), Position{0, 0});
  size_t line = 0;
  for (auto const& layer : layers) {
    size_t col = 0;
    for (size_t n : layer) {
      assert(dag.nodes[n].text.size() == 1);
      ret[n].col = col;
      ret[n].line = line;
      // 1 for space + 1 for the node text (single-character)
      col += 2;
    }
    ++line;
  }
  return ret;
}

std::vector<std::vector<size_t>>
groupEdgesByLayer(Connectivity const& conn, std::vector<std::vector<size_t>> const& layers) {
  size_t const N = conn.nodeValencies.size();
  std::vector<std::vector<size_t>> ret(layers.size());
  std::vector<size_t> nodeLayer(N);
  for (size_t i = 0; i < layers.size(); ++i) {
    for (auto n : layers[i]) {
      nodeLayer[n] = i;
    }
  }
  for (size_t i = 0; i < conn.edges.size(); ++i) {
    ret[nodeLayer[conn.edges[i].from]].push_back(i);
  }
  return ret;
}

void adjustCoordsWithValencies(
  std::vector<Position>& coords,
  Connectivity const& conn,
  std::vector<std::vector<size_t>> const& layers
) {
  for (auto const& layer : layers) {
    size_t lastCol = 0;
    for (auto node : layer) {
      if (conn.nodeValencies[node].bottomLeft || conn.nodeValencies[node].topLeft) {
        lastCol += 1; // Accomodate left edge (incoming or outgoing)
      }
      coords[node].col = lastCol;
      lastCol += 2; // Accomodate node width and mandatory space
      if (conn.nodeValencies[node].bottomRight || conn.nodeValencies[node].topRight) {
        lastCol += 1; // Accomodate right edge (incoming or outgoing)
      }
    }
  }
  auto interLayerEdges = groupEdgesByLayer(conn, layers);
  assert(interLayerEdges.size() == layers.size());
  size_t line = 0;
  for (size_t i = 0; i < layers.size(); ++i) {
    for (auto n : layers[i]) {
      coords[n].line = line;
    }
    // 1 for the node height
    line += 1 + minDistBetweenLayers(conn, interLayerEdges[i], coords);
  }
}

void placeNodes(
  DAG const& dag,
  std::vector<Position> const& coordinates,
  std::vector<std::string>& canvas
) {
  for (size_t n = 0; n < dag.nodes.size(); ++n) {
    assert(dag.nodes[n].text.size() == 1);
    assert(canvas[coordinates[n].line][coordinates[n].col] == ' ' && "Nodes overlay.");
    canvas[coordinates[n].line][coordinates[n].col] = dag.nodes[n].text[0];
  }
}

std::string rtrim(std::string s) {
  s.erase(s.find_last_not_of(' ') + 1);
  return s;
}

void placeEdges(
  std::vector<Position> const& coordinates,
  std::vector<Connectivity::Edge> const& edges,
  std::vector<std::string>& canvas
) {
  for (auto const& e : edges) {
    drawEdge(coordinates[e.from], e.exitAngle, coordinates[e.to], e.entryAngle, canvas);
  }
}

std::vector<std::string> createCanvas(std::vector<Position> const& coordinates) {
  Position max{0, 0};
  for (auto const& p : coordinates) {
    if (max.line < p.line) {
      max.line = p.line;
    }
    if (max.col < p.col) {
      max.col = p.col;
    }
  }
  // line + 1 - to accomodate the node height
  // col + 2 - to accomodate the node width + potential top/bottom-right edge
  return std::vector<std::string>(max.line + 1, std::string(max.col + 2, ' '));
}

std::optional<RenderError> checkDAGCompat(DAG const& dag) {
  for (size_t n = 0; n < dag.nodes.size(); ++n) {
    if (dag.nodes[n].text.size() != 1) {
      return {
        {RenderError::Code::Unsupported, "Zero- or multi-character nodes are not supported.", n}
      };
    }
  }
  return {};
}

std::optional<RenderError> checkIfEdgesFitOnNodes(DAG const& dag) {
  size_t const N = dag.nodes.size();
  std::vector<size_t> incomingEdgesPerNode(N, 0);

  for (size_t i = 0; i < N; ++i) {
    auto const& n = dag.nodes[i];
    if (3 < n.succs.size()) {
      return {
        {RenderError::Code::Overcrowded, "Too many outgoing edges from a node, they don't fit.", i}
      };
    }
    for (auto const& s : n.succs) {
      ++incomingEdgesPerNode[s];
    }
  }
  for (size_t i = 0; i < N; ++i) {
    if (3 < incomingEdgesPerNode[i]) {
      return {
        {RenderError::Code::Overcrowded, "Too many incoming edges to a node, they don't fit.", i}
      };
    }
  }
  return {};
}

Direction chooseNextDirection(
  Position const& cur,
  Direction startDir,
  Position const& to,
  Direction finishDir
) {
  auto targetCol = to.col - directionShift(finishDir);
  if (cur.col == targetCol) {
    return Direction::Straight;
  }
  if (startDir == Direction::Straight &&
        absDiff(cur.col, targetCol - directionShift(finishDir)) < to.line - 3 - cur.line) {
    return Direction::Straight;
  }
  if (cur.col < targetCol) {
    return Direction::Right;
  }
  return Direction::Left;
}

} // namespace

std::string renderCanvas(std::vector<std::string> const& canvas) {
  std::string ret;
  ret.reserve(canvas.size() * (canvas[0].size() + 1));
  for (auto const& line : canvas) {
    ret += rtrim(line) + "\n";
  }
  return ret;
}

void drawEdge(
  Position cur,
  Direction curDir,
  Position to,
  Direction finishDir,
  std::vector<std::string>& canvas
) {
  cur.line += 1;
  cur.col += directionShift(curDir);
  // TODO: enable this when lines are guaranteed to not intersect
  // assert(canvas[cur.line][cur.col] == ' ');
  canvas[cur.line][cur.col] = EdgesInFlight::edgeChar(curDir);

  assert(cur.line < to.line && to.line < canvas.size());
  assert(cur.col < canvas[cur.line].size() && to.col < canvas[to.line].size());

  if (cur.line + 1 == to.line) {
    assert(curDir == finishDir);
    return;
  }

  while (cur.line + 2 < to.line) {
    auto nextDir = chooseNextDirection(cur, curDir, to, finishDir);
    if (curDir == nextDir) {
      // Advance when continuing in the same direction such as:
      //  \         /
      //   \  and  /
      cur.col += directionShift(curDir);
    }
    curDir = nextDir;
    cur.line += 1;
    // TODO: enable this when lines are guaranteed to not intersect
    // assert(canvas[cur.line][cur.col] == ' ');
    canvas[cur.line][cur.col] = EdgesInFlight::edgeChar(curDir);
  }

  assert(cur.line + 2 == to.line);
  to.line -= 1;
  to.col -= directionShift(finishDir);
  // TODO: assert that lines meet
  // TODO: enable this when lines are guaranteed to not intersect
  // assert(canvas[to.line][to.col] == ' ');
  canvas[to.line][to.col] = EdgesInFlight::edgeChar(finishDir);
}

size_t findIndex(std::vector<size_t> const list, size_t val) {
  // This linear search might better be replaced with a lookup table
  for (size_t pos = 0; pos < list.size(); ++pos) {
    if (list[pos] == val) {
      return pos;
    }
  }
  assert(false && "The node must be in this list");
  return 0;
}

void minimizeCrossings(std::vector<std::vector<size_t>> &layers, DAG const& dag) {
  std::vector<std::vector<size_t>> preds(dag.nodes.size());
  for (size_t i = 0; i < dag.nodes.size(); ++i) {
    for (auto succ : dag.nodes[i].succs) {
      preds[succ].push_back(i);
    }
  }
  std::vector<size_t> targetPos(dag.nodes.size());
  for (size_t i = 1; i < layers.size(); ++i) {
    for (size_t nId : layers[i]) {
      size_t nPreds = preds[nId].size();
      // We've skipped the layer 0 - the only layer with "roots" - nodes with no predecessors
      assert(0 < nPreds && "After insertion of waypoints, all edges span a single layer");
      targetPos[nId] = 0; // Reset potential previous pass
      for (auto pred : preds[nId]) {
        targetPos[nId] += findIndex(layers[i - 1], pred);
      }
      targetPos[nId] /= nPreds;
    }
    std::stable_sort(layers[i].begin(), layers[i].end(), [&targetPos](size_t n1id, size_t n2id) {
      return targetPos[n1id] < targetPos[n2id];
    });
  }
}

std::optional<std::string> renderDAG(DAG dag, RenderError& err) {
  err.code = RenderError::Code::None;
  // TODO: find best horisontal position of nodes
  if (auto compatErr = checkDAGCompat(dag)) {
    err = *compatErr;
    return {};
  }
  if (auto crowdedErr = checkIfEdgesFitOnNodes(dag)) {
    err = *crowdedErr;
    return {};
  }
  auto layers = dagLayers(dag);
  if (auto waypointErr = insertEdgeWaypoints(dag, layers)) {
    err = *waypointErr;
    return {};
  }
  minimizeCrossings(layers, dag);
  // TODO: insert intermediate layers to layout the edge crossings
  auto coords = computeNodeCoordinates(dag, layers);
  auto const connectivity = computeConnectivity(dag, coords);
  adjustCoordsWithValencies(coords, connectivity, layers);
  auto canvas = createCanvas(coords);
  placeNodes(dag, coords, canvas);
  placeEdges(coords, connectivity.edges, canvas);
  return renderCanvas(canvas);
}

std::optional<DAG> parseDAG(std::string_view str, ParseError& err) {
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
      auto fromNode = prevEdges.findNRemoveEdgeToEdge(*dir, collector.getPrevNodes(), pos.col);
      if (auto e = currEdges.updateOrError(fromNode, *dir, pos)) {
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
  if (auto inconsistent = collector.finalize()) {
    err = *inconsistent;
    return std::nullopt;
  }
  return std::move(collector).buildDAG();
}

std::string parseErrorCodeToStr(ParseError::Code code) {
  using Code = ParseError::Code;
  switch (code) {
    case Code::DanglingEdge:
      return "DanglingEdge";
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
