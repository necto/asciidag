#include "asciidag.h"

#include "asciidagImpl.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace asciidag {

constexpr auto sketchMode = false;
auto waypointText = "|";

namespace {
std::stringstream log;

using namespace asciidag::detail;

size_t findIndex(Vec<size_t> const list, size_t val) {
  // This linear search might better be replaced with a lookup table
  for (size_t pos = 0; pos < list.size(); ++pos) {
    if (list[pos] == val) {
      return pos;
    }
  }
  assert(false && "The node must be in this list");
  return 0;
}

Vec2<size_t> dagLayers(DAG const& dag) {
  // TODO: optimize using a queue (linear in edges)
  // instead of a fix-point (quadratic in edges)
  Vec<size_t> rank(dag.nodes.size(), 0);
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
  Vec2<size_t> ret(maxRank + 1);
  for (size_t n = 0; n < dag.nodes.size(); ++n) {
    ret[rank[n]].push_back(n);
  }
  return ret;
}

void replace(Vec<size_t>& values, size_t dated, size_t updated) {
  for (auto& v : values) {
    if (v == dated) {
      v = updated;
    }
  }
}

std::optional<RenderError> insertEdgeWaypoints(DAG& dag, Vec2<size_t>& layers) {
  size_t const preexistingCount = dag.nodes.size();
  Vec<size_t> rank(preexistingCount, 0);
  for (size_t layerI = 0; layerI < layers.size(); ++layerI) {
    for (size_t n : layers[layerI]) {
      rank[n] = layerI;
    }
  }
  for (size_t layerI = 0; layerI < layers.size(); ++layerI) {
    for (size_t n : layers[layerI]) {
      if (preexistingCount <= n) {
        // This is a waypoint that by construction has its edge targeting the next layer
        assert(dag.nodes[n].succs.size() == 1);
        assert(
          preexistingCount <= dag.nodes[n].succs[0] || rank[dag.nodes[n].succs[0]] == layerI + 1
        );
        continue;
      }
      for (size_t& e : dag.nodes[n].succs) {
        assert(layerI < rank[e]);
        if (layerI + 1 == rank[e]) {
          continue;
        }
        size_t finalSucc = e;
        size_t* lastEdge = &e;
        for (auto l = layerI + 1; l < rank[finalSucc]; ++l) {
          size_t nodeId = dag.nodes.size();
          *lastEdge = nodeId;
          dag.nodes.push_back({{0}, waypointText});
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

struct SimpleEdge {
  size_t from;
  size_t to;

  bool operator==(SimpleEdge const& other) const {
    return std::tie(from, to) == std::tie(other.from, other.to);
  }
};

struct SimpleEdgeHash {
  size_t operator()(SimpleEdge const& e) const noexcept { return e.from ^ (e.to << 1); }
};

template<class Collection, class elem>
bool contains(Collection const& cont, elem el) {
  return std::find(std::begin(cont), std::end(cont), el) != std::end(cont);
}

size_t insertCrossNode(DAG& dag, CrossingPair const& crossing) {
  log
    << "inserting crossing " << crossing.fromLeft << "->" << crossing.toRight << "; "
    << crossing.fromRight << "<-" << crossing.toLeft << "\n";
  size_t fromLeftIdx = findIndex(dag.nodes[crossing.fromLeft].succs, crossing.toRight);
  size_t fromRightIdx = findIndex(dag.nodes[crossing.fromRight].succs, crossing.toLeft);
  size_t xid = dag.nodes.size();
  dag.nodes.emplace_back();
  dag.nodes[xid].text = "X";
  dag.nodes[xid].succs.push_back(crossing.toLeft);
  dag.nodes[xid].succs.push_back(crossing.toRight);
  dag.nodes[crossing.fromLeft].succs[fromLeftIdx] = xid;
  dag.nodes[crossing.fromRight].succs[fromRightIdx] = xid;
  log <<"inserted " <<xid <<"\n";
  return xid;
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

template <typename A, typename B>
std::ostream& operator<<(std::ostream& os, std::pair<A, B> p) {
  return os << "(" << p.first << ", " << p.second << ")";
}

template <typename A>
std::ostream& operator<<(std::ostream& os, Vec<A> v) {
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
void append(Vec<T>& to, std::vector<T> const& from) {
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

std::optional<Direction> edgeChar(char c);
char edgeChar(Direction dir);

struct ConnToNode {
  size_t nId;
  Direction exitAngle;
  Direction entryAngle;
};

std::ostream& operator<<(std::ostream& os, ConnToNode const& conn) {
  return os << edgeChar(conn.exitAngle) << edgeChar(conn.entryAngle) << conn.nId;
}

using EdgeMap = std::unordered_map<size_t, ConnToNode>;

// Map from position to a node id, if any
using NodeMap = Vec<std::optional<size_t>>;

class EdgesInFlight {
public:

  Vec<ConnToNode> findNRemoveEdgesToNode(size_t col);
  std::optional<ConnToNode>
  findNRemoveEdgeToEdge(Direction dir, NodeMap const& prevNodes, size_t col);
  std::optional<ParseError> findDanglingEdge(size_t line) const;
  friend std::ostream& operator<<(std::ostream& os, EdgesInFlight const& edges);

  std::optional<ParseError>
  updateOrError(std::optional<ConnToNode> fromNode, Direction dir, Position const& pos);

private:

  /// edges[0] stores an empty map (just for padding)
  /// The edges[1]..edges[3] corresponds to the Direction options
  std::array<EdgeMap, 4> edges;
};

std::optional<Direction> edgeChar(char c) {
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

char edgeChar(Direction dir) {
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

std::optional<ParseError> EdgesInFlight::
  updateOrError(std::optional<ConnToNode> fromNodes, Direction dir, Position const& pos) {
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

Vec<ConnToNode> EdgesInFlight::findNRemoveEdgesToNode(size_t col) {
  Vec<ConnToNode> ret;
  for (auto dir : {toInt(Direction::Left), toInt(Direction::Straight), toInt(Direction::Right)}) {
    if (auto to = findAndEraseIf(edges[dir], col + columnShift[dir][0])) {
      ret.emplace_back(*to);
    }
  }
  return ret;
}

std::optional<ConnToNode>
EdgesInFlight::findNRemoveEdgeToEdge(Direction dirBelow, NodeMap const& prevNodes, size_t col) {
  // Important to order them in the order they can appear on the previous line,
  // from left to right, i.e., \(Right), |(Straight), /(Left)
  for (auto dirAbove :
       {toInt(Direction::Right), toInt(Direction::Straight), toInt(Direction::Left)}) {
    if (auto from = findAndEraseIf(edges[dirAbove], col + columnShift[dirAbove][toInt(dirBelow)])) {
      from->entryAngle = dirBelow;
      return *from;
    }
  }
  // The early return above guarantees that an edge is not connected to a node
  // if it is already connected to an edge
  if (auto to = prevNodes[col + columnShift[0][toInt(dirBelow)]]) {
    return {{*to, dirBelow, dirBelow}};
  }
  return std::nullopt;
}

[[maybe_unused]]
std::ostream&
operator<<(std::ostream& os, EdgesInFlight const& edges) {
  bool first = true;
  for (auto dir : {Direction::Left, Direction::Straight, Direction::Right}) {
    os << edgeChar(dir) << ' ' << edges.edges[toInt(dir)];
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
          "Dangling edge "s + edgeChar(dir) + " from " + std::to_string(src.nId),
          {line, col}
        };
      }
    }
  }
  return ret;
}

class NodeCollector {
public:
  NodeCollector(size_t maxWidth) : prevNodes(maxWidth + 1), currNodes(maxWidth + 1) {
    // Allocate 1 extra because columns are counted from 1
  }

  struct Edge {
    size_t fromNode;
    Direction exitDir;
    size_t toNode;
    Direction entryDir;
  };

  struct Node {
    string text;
    Position pos;
    Vec<size_t> succEdges;
    Vec<size_t> predEdges;
  };

  std::optional<ParseError> tryAddNode(EdgesInFlight& prevEdges, Position const& pos);

  void addNodeChar(char c) { partialNode.push_back(c); }

  bool isPartOfANode(size_t col) const;

  DAG buildDAG() &&;

  NodeMap const& getPrevNodes() const { return prevNodes; }

  void newLine();

  std::optional<ParseError> finalize();

private:
  std::optional<ParseError> checkRectangularNewNode(Position const& pos);
  void startNewNode(EdgesInFlight& prevEdges, Position const& pos);
  std::optional<ParseError> checkRectangularNodeLine(size_t nodeAbove, Position const& pos);
  void addNodeLine(size_t nodeAbove, EdgesInFlight& prevEdges, Position const& pos);

  void resolveCrossEdges();

  void addEdge(Edge&& e);

  Vec<Node> nodes = {};
  Vec<Edge> edges = {};
  string partialNode = "";
  NodeMap prevNodes;
  NodeMap currNodes;
  bool finalized = false;
};

DAG NodeCollector::buildDAG() && {
  DAG ret;
  ret.nodes.reserve(nodes.size());
  for (auto&& node : nodes) {
    ret.nodes.emplace_back();
    ret.nodes.back().text = std::move(node.text);
    auto& succs = ret.nodes.back().succs;
    for (auto const& edgeId : node.succEdges) {
      succs.push_back(edges[edgeId].toNode);
    }
  }
  return ret;
}

bool hasCrossEdges(Vec<NodeCollector::Node> const& nodes) {
  return std::any_of(nodes.begin(), nodes.end(), [](auto const& n) { return n.text == "X"; });
}

std::optional<ParseError> validateEdgeCrossings(Vec<NodeCollector::Node> const& nodes) {
  for (auto const& node : nodes) {
    if (node.text == "X") {
      auto nPreds = node.predEdges.size();
      auto nSuccs = node.succEdges.size();
      if (nPreds < 2 || nPreds < nSuccs) {
        return {
          {ParseError::Code::SuspendedEdge,
           "Edge crossing misses one or two incoming edges.",
           node.pos}
        };
      }
      assert(nPreds < 4); // Single node has only 3 possible edge entries
      if (nSuccs < 2 || nSuccs < nPreds) {
        return {
          {ParseError::Code::DanglingEdge,
           "Edge crossing misses one or two outgoing edges.",
           node.pos}
        };
      }
    }
  }
  return {};
}

bool inOrder(size_t a, size_t b, size_t c) {
  return a <= b && b <= c;
}

std::pair<size_t, size_t> increasingOrder(size_t zero, size_t one) {
  if (zero <= one) {
    return {0, 1};
  }
  return {1, 0};
}

std::tuple<size_t, size_t, size_t> increasingOrder(size_t zero, size_t one, size_t two) {
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
    return {2, 0, 1};
  }
  assert(inOrder(two, one, zero));
  return {2, 1, 0};
}

std::pair<size_t, size_t> chooseLeftRightDirs(Direction dir0, Direction dir1) {
  assert(dir0 != dir1);
  return increasingOrder(toInt(dir0), toInt(dir1));
}

std::tuple<size_t, size_t, size_t>
chooseLeftMiddleRightDirs(Direction dir0, Direction dir1, Direction dir2) {
  assert(dir0 != dir1);
  assert(dir0 != dir2);
  assert(dir1 != dir2);
  return increasingOrder(toInt(dir0), toInt(dir1), toInt(dir2));
}

void joinEdges(
  Vec<NodeCollector::Edge>& edges,
  Vec<NodeCollector::Node>& nodes,
  size_t primary,
  size_t secondary
) {
  assert(primary != secondary);
  edges[primary].toNode = edges[secondary].toNode;
  edges[primary].entryDir = edges[secondary].entryDir;
  replace(nodes[edges[primary].toNode].predEdges, secondary, primary);
}

void untangleTwoEdgeCrossing(
  Vec<size_t> const& preds,
  Vec<size_t> const& succs,
  Vec<NodeCollector::Node>& nodes,
  Vec<NodeCollector::Edge>& edges
) {
  assert(preds.size() == 2);
  assert(succs.size() == 2);
  auto [succLeft, succRight] =
    chooseLeftRightDirs(edges[succs[0]].exitDir, edges[succs[1]].exitDir);
  auto [predRight, predLeft] =
    chooseLeftRightDirs(edges[preds[0]].entryDir, edges[preds[1]].entryDir);
  joinEdges(edges, nodes, preds[predLeft], succs[succRight]);
  joinEdges(edges, nodes, preds[predRight], succs[succLeft]);
}

void untangleThreeEdgeCrossing(
  Vec<size_t> const& preds,
  Vec<size_t> const& succs,
  Vec<NodeCollector::Node>& nodes,
  Vec<NodeCollector::Edge>& edges
) {
  assert(preds.size() == 3);
  assert(succs.size() == 3);
  auto [succLeft, succMid, succRight] = chooseLeftMiddleRightDirs(
    edges[succs[0]].exitDir,
    edges[succs[1]].exitDir,
    edges[succs[2]].exitDir
  );
  auto [predRight, predMid, predLeft] = chooseLeftMiddleRightDirs(
    edges[preds[0]].entryDir,
    edges[preds[1]].entryDir,
    edges[preds[2]].entryDir
  );
  joinEdges(edges, nodes, preds[predLeft], succs[succRight]);
  joinEdges(edges, nodes, preds[predMid], succs[succMid]);
  joinEdges(edges, nodes, preds[predRight], succs[succLeft]);
}

void removeXnodes(Vec<NodeCollector::Node>& nodes, Vec<NodeCollector::Edge>& edges) {
  size_t nSkipped = 0;
  Vec<size_t> nodeIdMap(nodes.size());
  for (size_t i = 0; i < nodes.size(); ++i) {
    if (nodes[i].text == "X") {
      ++nSkipped;
    }
    nodeIdMap[i] = i - nSkipped;
  }
  for (auto& edge : edges) {
    edge.fromNode = nodeIdMap[edge.fromNode];
    edge.toNode = nodeIdMap[edge.toNode];
  }
  nodes.erase(
    std::remove_if(nodes.begin(), nodes.end(), [](auto const& n) { return n.text == "X"; }),
    nodes.end()
  );
}

void NodeCollector::resolveCrossEdges() {
  for (auto const& node : nodes) {
    if (node.text == "X") {
      // Assertions are ensured by "validateEdgeCrossings"
      size_t nPreds = node.predEdges.size();
      assert(2 <= nPreds);
      if (nPreds == 2) {
        untangleTwoEdgeCrossing(node.predEdges, node.succEdges, nodes, edges);
      } else {
        untangleThreeEdgeCrossing(node.predEdges, node.succEdges, nodes, edges);
      }
    }
  }
  removeXnodes(nodes, edges);
  // Note here some edges that were connected the "X" nodes
  // are preserved and are inconsistent with the new array of nodes,
  // but they are "dead" - not referenced from this array of nodes
}

std::optional<ParseError> NodeCollector::finalize() {
  if (hasCrossEdges(nodes)) {
    if (auto err = validateEdgeCrossings(nodes)) {
      return err;
    }
    resolveCrossEdges();
  }
  finalized = true;
  return {};
}

std::optional<ParseError> NodeCollector::tryAddNode(EdgesInFlight& prevEdges, Position const& pos) {
  if (partialNode.empty()) {
    return {};
  }
  if (auto nodeAbove = prevNodes[pos.col - partialNode.size()]) {
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
  return !partialNode.empty() && prevNodes[col - 1].has_value() && prevNodes[col].has_value() != 0;
}

void NodeCollector::newLine() {
  prevNodes = std::move(currNodes);
  currNodes = NodeMap(prevNodes.size());
}

std::optional<ParseError> NodeCollector::checkRectangularNewNode(Position const& pos) {
  for (size_t p = pos.col - partialNode.size() + 1; p < pos.col; ++p) {
    if (prevNodes[p].has_value()) {
      return {ParseError{
        ParseError::Code::NonRectangularNode,
        "Node-line above started midway node-line below.",
        {pos.line, p}
      }};
    }
  }
  return {};
}

void NodeCollector::addEdge(NodeCollector::Edge&& e) {
  size_t edgeId = edges.size();
  nodes[e.fromNode].succEdges.push_back(edgeId);
  nodes[e.toNode].predEdges.push_back(edgeId);
  edges.emplace_back(std::move(e));
}

void NodeCollector::startNewNode(EdgesInFlight& prevEdges, Position const& pos) {
  size_t id = nodes.size();
  nodes.emplace_back();
  nodes[id].text = partialNode;
  nodes[id].pos = pos;
  for (size_t p = pos.col - partialNode.size(); p < pos.col; ++p) {
    for (auto from : prevEdges.findNRemoveEdgesToNode(p)) {
      addEdge({from.nId, from.exitAngle, id, from.entryAngle});
    }
    currNodes[p] = id;
  }
  partialNode.clear();
}

std::optional<ParseError>
NodeCollector::checkRectangularNodeLine(size_t nodeAbove, Position const& pos) {
  assert(partialNode.size() < pos.col);
  for (size_t p = pos.col - partialNode.size(); p < pos.col; ++p) {
    if (auto prevNode = prevNodes[p]) {
      // Node above can change only after a gap,
      // Two nodes "AA" and "BB" cannot follow each other immediately.
      // "AABB" would have been treated as a single node.
      assert(nodeAbove == *prevNode);
    } else {
      return {ParseError{
        ParseError::Code::NonRectangularNode,
        "Node-line above ended midway node-line below.",
        {pos.line, p}
      }};
    }
  }
  if (prevNodes[pos.col - 1 - partialNode.size()].has_value()) {
    return ParseError{
      ParseError::Code::NonRectangularNode,
      "Previous node-line was longer on the left side.",
      {pos.line, pos.col - 1 - partialNode.size()}
    };
  }
  if (prevNodes[pos.col].has_value()) {
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
    assert(partialNode.size() == 1 || edge.entryAngle == Direction::Right);
    addEdge({edge.nId, edge.exitAngle, nodeAbove, edge.entryAngle});
  }
  for (auto edge : prevEdges.findNRemoveEdgesToNode(pos.col - 1)) {
    assert(partialNode.size() == 1 || edge.entryAngle == Direction::Left);
    addEdge({edge.nId, edge.exitAngle, nodeAbove, edge.entryAngle});
  }
  nodes[nodeAbove].text += "\n" + partialNode;
  partialNode.clear();
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

  Vec<Edge> edges;
  Vec<Valency> nodeValencies;
};

bool operator<(Connectivity::Edge const& a, Connectivity::Edge const& b) {
  return std::tie(a.from, a.exitAngle, a.to, a.entryAngle)
       < std::tie(b.from, b.exitAngle, b.to, b.entryAngle);
}

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

size_t minEdgeHeight(Connectivity::Edge const& edge, Vec<Position> const& positions) {
  return absDiff(
           positions[edge.from].col + directionShift(edge.entryAngle),
           positions[edge.to].col - directionShift(edge.exitAngle)
         )
       + 3;
}

size_t minDistBetweenLayers(
  Connectivity const& conn,
  Vec<size_t> const& edges,
  Vec<Position> const& positions
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

void setEntryAngles(Connectivity& conn, Vec2<size_t> predEdges, Vec<Position> const& coords) {
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
        auto [straight, right] = increasingOrder(
          coords[conn.edges[edgeIds[0]].from].col,
          coords[conn.edges[edgeIds[1]].from].col
        );
        conn.edges[edgeIds[straight]].entryAngle = Direction::Straight;
        conn.edges[edgeIds[right]].entryAngle = Direction::Left;
        conn.nodeValencies[i].topRight = true;
        break;
      }
      case 3: {
        auto [left, straight, right] = increasingOrder(
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

void setExitAngles(Connectivity& conn, Vec2<size_t> succEdges, Vec<Position> const& coords) {
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
        auto [straight, right] = increasingOrder(
          coords[conn.edges[edgeIds[0]].to].col,
          coords[conn.edges[edgeIds[1]].to].col
        );
        conn.edges[edgeIds[straight]].exitAngle = Direction::Straight;
        conn.edges[edgeIds[right]].exitAngle = Direction::Right;
        conn.nodeValencies[i].bottomRight = true;
        break;
      }
      case 3: {
        auto [left, straight, right] = increasingOrder(
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

Connectivity computeConnectivity(DAG const& dag, Vec<Position> const& coords) {
  size_t const N = dag.nodes.size();
  Vec2<size_t> preds(N);
  Vec2<size_t> predEdges(N);
  Vec2<size_t> succEdges(N);
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
  std::sort(ret.edges.begin(), ret.edges.end());
  return ret;
}

Vec<Position> computeNodeCoordinates(DAG const& dag, Vec2<size_t> const& layers) {
  Vec<Position> ret(dag.nodes.size(), Position{0, 0});
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

Vec2<size_t> groupEdgesByLayer(Connectivity const& conn, Vec2<size_t> const& layers) {
  size_t const N = conn.nodeValencies.size();
  Vec2<size_t> ret(layers.size());
  Vec<size_t> nodeLayer(N);
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
  Vec<Position>& coords,
  Connectivity const& conn,
  Vec2<size_t> const& layers
) {
  for (auto const& layer : layers) {
    size_t lastCol = 0;
    bool prevNodeHadFullTop = false;
    bool prevNodeHadFullBottom = false;
    for (auto node : layer) {
      auto const& valencies = conn.nodeValencies[node];
      if (valencies.bottomLeft || valencies.topLeft) {
        lastCol += 1; // Accomodate left edge (incoming or outgoing)
        if ((prevNodeHadFullBottom && valencies.bottomLeft) || (prevNodeHadFullTop && valencies.topLeft)) {
          // Otherwise it might get too crowded for all the edges of previous node
          lastCol += 1;
        }
      }
      coords[node].col = lastCol;
      lastCol += 2; // accomodate node width and mandatory space
      if (valencies.bottomRight || valencies.topRight) {
        lastCol += 1; // accomodate right edge (incoming or outgoing)
      }
      prevNodeHadFullBottom = valencies.bottomRight && valencies.bottomLeft;
      prevNodeHadFullTop = valencies.topRight && valencies.topLeft;
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

void placeNodes(DAG const& dag, Vec<Position> const& coordinates, Canvas& canvas) {
  for (size_t n = 0; n < dag.nodes.size(); ++n) {
    assert(dag.nodes[n].text.size() == 1);
    canvas.newMark(coordinates[n], dag.nodes[n].text[0]);
  }
}

string rtrim(string s) {
  s.erase(s.find_last_not_of(' ') + 1);
  return s;
}

template <typename T>
bool isSorted(T list) {
  if (list.begin() == list.end()) {
    return true;
  }
  for (auto i = list.begin(), j = i + 1; j < list.end(); i = j++) {
    if (*j < *i) {
      return false;
    }
  }
  return true;
}

void placeEdges(
  Vec<Position> const& coordinates,
  Vec<Connectivity::Edge> const& edges,
  Canvas& canvas
) {
  assert(isSorted(edges));
  for (auto const& e : edges) {
    // TODO: assert that it returns true = success
    drawEdge(coordinates[e.from], e.exitAngle, coordinates[e.to], e.entryAngle, canvas);
  }
}

struct EdgeStep {
  Position initialPos;
  Position markedPos;
  Direction initialDir;
  std::optional<Direction> nextDir;
};

std::pair<Direction, Direction> chooseNextDirection(
  Position const& cur,
  Direction curDir,
  Position const& to,
  Direction finishDir
) {
  auto targetCol = to.col - directionShift(finishDir);
  if (cur.col == targetCol) {
    return {Direction::Straight, Direction::Right};
  }
  if (cur.col < targetCol) {
    if (curDir == Direction::Straight &&
        absDiff(cur.col, targetCol - directionShift(finishDir)) < to.line - 3 - cur.line) {
      return {Direction::Straight, Direction::Right};
    }
    return {Direction::Right, Direction::Straight};
  }
  if (curDir == Direction::Straight &&
        absDiff(cur.col, targetCol - directionShift(finishDir)) < to.line - 3 - cur.line) {
    return {Direction::Straight, Direction::Left};
  }
  return {Direction::Left, Direction::Straight};
}

Position nextPosInDir(Position curPos, Direction curDir, Direction nextDir) {
  curPos.line += 1;
  if (curDir == nextDir) {
    // Advance when continuing in the same direction such as:
    //  \         /
    //   \  and  /
    curPos.col += directionShift(nextDir);
  }
  return curPos;
}

void eraseAndBacktrackToLastChoice(Vec<EdgeStep>& drawnPath, Canvas& canvas) {
  while (!drawnPath.empty() && !drawnPath.back().nextDir.has_value()) {
    canvas.clearPos(drawnPath.back().markedPos);
    drawnPath.pop_back();
  }
  if (!drawnPath.empty()) {
    canvas.clearPos(drawnPath.back().markedPos);
  }
}

bool tryDrawLine(
  Position const& to,
  Direction const entryDir,
  Canvas& canvas,
  Vec<EdgeStep>& drawnPath
) {
  if (drawnPath.empty()) {
    return false;
  }
  EdgeStep& lastStep = drawnPath.back();
  Direction curDir = *lastStep.nextDir;
  lastStep.initialPos = nextPosInDir(lastStep.initialPos, lastStep.initialDir, curDir);
  canvas.newMark(lastStep.initialPos, edgeChar(curDir));
  lastStep.markedPos = lastStep.initialPos;
  lastStep.nextDir = std::nullopt;

  Position cur = lastStep.initialPos;
  while (cur.line + 2 < to.line) {
    EdgeStep step;
    auto [nextDir, alternativeNextDir] = chooseNextDirection(cur, curDir, to, entryDir);
    step.initialPos = cur;
    step.initialDir = curDir;
    step.nextDir = std::nullopt;
    auto nextPos = nextPosInDir(cur, curDir, nextDir);
    auto altPos = nextPosInDir(cur, curDir, alternativeNextDir);
    if (canvas.inBounds(nextPos) && canvas.isEmpty(nextPos)) {
      cur = nextPos;
      curDir = nextDir;
      if (canvas.inBounds(altPos) && canvas.isEmpty(altPos)) {
        step.nextDir = alternativeNextDir;
      }
    } else if (canvas.inBounds(altPos) && canvas.isEmpty(altPos)) {
      // Pivot immediately
      cur = altPos;
      curDir = alternativeNextDir;
    } else {
      return false;
    }
    step.markedPos = cur;
    canvas.newMark(cur, edgeChar(curDir));
    drawnPath.push_back(step);
  }
  assert(cur.line + 2 == to.line);
  return cur.col - columnShift[toInt(curDir)][toInt(entryDir)] == to.col - directionShift(entryDir);
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
  Vec<size_t> incomingEdgesPerNode(N, 0);

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

size_t findTargetPosTimes6(Vec<size_t> linkedNodes, std::vector<size_t> layer) {
  size_t const count = linkedNodes.size();
  assert(0 < count && "Leaf or root node on a non-first layer");
  size_t sum = 0;
  for (auto pred : linkedNodes) {
    sum += findIndex(layer, pred);
  }
  return sum * 6 / count;
}

template <typename Callable>
void swapEquipotentialNeighbors(
  Vec<size_t> const& targetPos,
  Vec<size_t>& curLayer,
  Callable const& penalty
) {
  size_t nCrossings = penalty(curLayer);
  if (0 < nCrossings) {
    for (size_t nodePos = 1; nodePos < curLayer.size(); ++nodePos) {
      if (targetPos[curLayer[nodePos - 1]] == targetPos[curLayer[nodePos]]) {
        // If the two nodes compete for the same position, try to swapt them
        std::swap(curLayer[nodePos - 1], curLayer[nodePos]);
        size_t newNCrossings = penalty(curLayer);
        if (newNCrossings < nCrossings) {
          nCrossings = newNCrossings;
        } else {
          // Not useful, put the nodes back
          std::swap(curLayer[nodePos - 1], curLayer[nodePos]);
        }
      }
    }
  }
}

size_t countAllCrossings(Vec2<size_t> const& layers, DAG const& dag) {
  size_t ret = 0;
  size_t const nLayers = layers.size();
  for (size_t layerI = 1; layerI < nLayers; ++layerI) {
    auto const& prevLayer = layers[layerI - 1];
    auto& curLayer = layers[layerI];
    ret += countCrossings(dag, prevLayer, curLayer);
  }
  return ret;
}

Vec2<size_t>
findForcedLeftNodesBecauseOfCrossings(DAG const& dag, Vec2<size_t> const& preds) {
  size_t const N = dag.nodes.size();
  Vec2<size_t> leftNodes(N);
  for (size_t nodeId = 0; nodeId < N; ++nodeId) {
    if (dag.nodes[nodeId].text == "X") {
      // Triple-crossings can be supported if needed
      assert(preds[nodeId].size() == 2);
      assert(dag.nodes[nodeId].succs.size() == 2);
      leftNodes[preds[nodeId][1]].push_back(preds[nodeId][0]);
      leftNodes[dag.nodes[nodeId].succs[1]].push_back(dag.nodes[nodeId].succs[0]);
    }
  }
  return leftNodes;
}

void keepOrderOf(Vec<size_t> const& layer, Vec<size_t>& targetPos6, Vec2<size_t> const& leftNodes) {
  for (auto nId : layer) {
    // Make sure the position of the right-most node is larger than the positions of its left counterparts
    size_t& pos = targetPos6[nId];
    for (auto left : leftNodes[nId]) {
      if (pos <= targetPos6[left]) {
        pos = targetPos6[left] + 1;
      }
    }
  }
}

void minimizeCrossingsForward(
  Vec2<size_t>& layers,
  DAG const& dag,
  Vec2<size_t> const& preds,
  Vec2<size_t> const& leftNodes
) {
  size_t const nLayers = layers.size();
  Vec<size_t> targetPos6(dag.nodes.size());
  log <<leftNodes <<"\n";
  for (size_t layerI = 1; layerI < nLayers; ++layerI) {
    auto const& prevLayer = layers[layerI - 1];
    auto& curLayer = layers[layerI];
    for (size_t nId : curLayer) {
      assert(0 < preds[nId].size() && "Root node can only be on the 0-th layer.");
      targetPos6[nId] = findTargetPosTimes6(preds[nId], prevLayer);
    }
    keepOrderOf(curLayer, targetPos6, leftNodes);
    auto layerCopy = curLayer;
    size_t totCrossings =
      countCrossings(dag, prevLayer, curLayer)
      + (layerI + 1 < nLayers ? countCrossings(dag, curLayer, layers[layerI + 1]) : 0);
    std::stable_sort(curLayer.begin(), curLayer.end(), [&targetPos6](size_t n1id, size_t n2id) {
      return targetPos6[n1id] < targetPos6[n2id];
    });
    swapEquipotentialNeighbors(targetPos6, curLayer, [&dag, &prevLayer](auto const& curLayer) {
      return countCrossings(dag, prevLayer, curLayer);
    });
    size_t newCrossings =
      countCrossings(dag, prevLayer, curLayer)
      + (layerI + 1 < nLayers ? countCrossings(dag, curLayer, layers[layerI + 1]) : 0);
    if (totCrossings < newCrossings) {
      curLayer = layerCopy;
    }
  }
}

void minimizeCrossingsBackward(
  Vec2<size_t>& layers,
  DAG const& dag,
  Vec2<size_t> const& preds,
  Vec2<size_t> const& leftNodes
) {
  size_t const nLayers = layers.size();
  Vec<size_t> targetPos6(dag.nodes.size());

  for (size_t i = 1; i < nLayers; ++i) {
    auto& curLayer = layers[nLayers - i - 1];
    auto const& nextLayer = layers[nLayers - i];
    for (size_t position = 0; position < curLayer.size(); ++position) {
      size_t nId = curLayer[position];
      auto const& succs = dag.nodes[nId].succs;
      if (succs.empty()) {
        if (i + 1 < nLayers) {
          // No successors, look at your predecessors
          assert(!preds[nId].empty());
          auto& prevLayer = layers[nLayers - i - 2];
          // Scale the nextLayer width to be comparable
          // with positions of other nodes that are defined by nextLayers
          targetPos6[nId] = findTargetPosTimes6(preds[nId], prevLayer) * nextLayer.size() / prevLayer.size();
        } else {
          // Complete orphan, stay where you are
          targetPos6[nId] = position * 6;
        }
      } else {
        targetPos6[nId] = findTargetPosTimes6(succs, nextLayer);
      }
    }
    keepOrderOf(curLayer, targetPos6, leftNodes);
    auto layerCopy = curLayer;
    size_t totCrossings =
      countCrossings(dag, curLayer, nextLayer)
      + (i + 1 < nLayers ? countCrossings(dag, layers[nLayers - i - 2], curLayer) : 0);
    std::stable_sort(curLayer.begin(), curLayer.end(), [&targetPos6](size_t n1id, size_t n2id) {
      return targetPos6[n1id] < targetPos6[n2id];
    });
    swapEquipotentialNeighbors(targetPos6, curLayer, [&dag, &nextLayer](auto const& curLayer) {
      return countCrossings(dag, curLayer, nextLayer);
    });
    size_t newCrossings =
      countCrossings(dag, curLayer, nextLayer)
      + (i + 1 < nLayers ? countCrossings(dag, layers[nLayers - i - 2], curLayer) : 0);
    if (totCrossings < newCrossings) {
      curLayer = layerCopy;
    }
  }
}

string escapeForDOTlabel(string_view str) {
  string ret;
  ret.reserve(str.size());
  for (char c : str) {
    switch (c) {
      case '\n':
        ret += "\\n";
        break;
      case '\t':
        ret += "\\t";
        break;
      case '"':
      case '{':
      case '}':
        ret += '\\';
        ret += c;
        break;
      default:
        ret += c;
    }
  }
  return ret;
}

} // namespace

namespace detail {

Vec<CrossingPair>
findNonConflictingCrossings(DAG const& dag, Vec<size_t> const& lAbove, Vec<size_t> const& lBelow) {
  Vec<CrossingPair> ret;
  std::unordered_set<SimpleEdge, SimpleEdgeHash> takenEdges;
  // TODO: These 5 nested loops can definitely be optmized
  // TODO: reverse this top-level loop: it should scan right-to-left to find the highest crossing first
  for (size_t leftTopPos = 0; leftTopPos < lAbove.size(); ++leftTopPos) {
    auto leftTop = lAbove[leftTopPos];
    // Starting at the other end to find the highest crossing first
    for (size_t rightBottomPos = lBelow.size() - 1; rightBottomPos != 0; --rightBottomPos) {
      auto rightBottom = lBelow[rightBottomPos];
      SimpleEdge leftRightEdge{leftTop, rightBottom};
      if (!contains(dag.nodes[leftTop].succs, rightBottom) ||
          takenEdges.count(leftRightEdge) != 0) {
        continue;
      }
      for (size_t rightTopPos = leftTopPos + 1; rightTopPos < lAbove.size(); ++rightTopPos) {
        auto rightTop = lAbove[rightTopPos];
        bool found = false;
        for (size_t leftBottomPos = 0; leftBottomPos < rightBottomPos; ++leftBottomPos) {
          auto leftBottom = lBelow[leftBottomPos];
          if (!contains(dag.nodes[rightTop].succs, leftBottom)) {
            continue;
          }
          SimpleEdge rightLeftEdge{rightTop, leftBottom};
          if (takenEdges.count(rightLeftEdge) == 0) {
            takenEdges.insert(rightLeftEdge);
            takenEdges.insert(leftRightEdge);
            ret.push_back({leftTop, rightTop, leftBottom, rightBottom});
            found = true;
            break;
          }
        }
        if (found) {
          // For any edge
          // can only resolve one crossing at a time
          break;
        }
      }
    }
  }
  return ret;
}

size_t countCrossings(DAG const& dag, Vec<size_t> const& lAbove, Vec<size_t> const& lBelow) {
  size_t ret = 0;
  // TODO: These 5 nested loops can definitely be optmized
  for (size_t leftTopPos = 0; leftTopPos < lAbove.size(); ++leftTopPos) {
    for (size_t rightTopPos = leftTopPos + 1; rightTopPos < lAbove.size(); ++rightTopPos) {
      for (size_t rightBottom : dag.nodes[lAbove[leftTopPos]].succs) {
        for (size_t leftBottom : dag.nodes[lAbove[rightTopPos]].succs) {
          if (findIndex(lBelow, leftBottom) < findIndex(lBelow, rightBottom)) {
            ++ret;
          }
        }
      }
    }
  }
  return ret;
}


size_t insertEdgeWaypoint(DAG& dag, size_t from, size_t to) {
  size_t nodeId = dag.nodes.size();
  dag.nodes.push_back({{to}, waypointText});
  replace(dag.nodes[from].succs, to, nodeId);
  return nodeId;
}

void padWithWaypointToPreserveCrossPredOrder(
  DAG& dag,
  Vec<size_t>& layer,
  Vec<size_t> const& predLayer,
  size_t from,
  size_t to
) {
  assert(contains(dag.nodes[from].succs, to));
  if (dag.nodes[to].text == "X") {
    auto leftNodeI = std::find_if(predLayer.begin(), predLayer.end(), [&](size_t id) {
      auto const& succs = dag.nodes[id].succs;
      return std::find(succs.begin(), succs.end(), to) != succs.end();
    });
    assert(leftNodeI != predLayer.end());
    auto rightNodeI = std::find(predLayer.begin(), predLayer.end(), from);
    assert(rightNodeI != predLayer.end());
    if (leftNodeI < rightNodeI) {
      layer.push_back(insertEdgeWaypoint(dag, *leftNodeI, to));
    }
  }
}

Vec2<size_t> insertCrossNodes(DAG& dag, Vec2<size_t> const& layers) {
  Vec2<size_t> newLayers;
  newLayers.push_back(layers[0]);
  for (size_t layerI = 1; layerI < layers.size(); ++layerI) {
    Vec<size_t> insertedNodes;
    for (auto const& crossing :
         findNonConflictingCrossings(dag, layers[layerI - 1], layers[layerI])) {
      // Order (right->left then left->right) avoids introducing unnecessary crossing
      padWithWaypointToPreserveCrossPredOrder(
        dag,
        insertedNodes,
        layers[layerI - 1],
        crossing.fromRight,
        crossing.toLeft
      );
      padWithWaypointToPreserveCrossPredOrder(
        dag,
        insertedNodes,
        layers[layerI - 1],
        crossing.fromLeft,
        crossing.toRight
      );
      insertedNodes.push_back(insertCrossNode(dag, crossing));
    }
    // This does not preserve the order of predecessors, which matters
    // for cross nodes in the next layer.
    // It should be fine as long as it always inserts the highest crossing
    if (!insertedNodes.empty()) {
      newLayers.emplace_back(std::move(insertedNodes));
    }
    newLayers.push_back(layers[layerI]);
  }
  auto waypointErr = insertEdgeWaypoints(dag, newLayers);
  assert(!waypointErr.has_value() && "It's safe, I promise");
  return newLayers;
}

void minimizeCrossings(Vec2<size_t>& layers, DAG const& dag) {
  Vec2<size_t> preds(dag.nodes.size());
  // Enumerating nodes by layer to make sure preds[*] for each node have
  // the same order as the layer they are on
  for (auto const& layer : layers) {
    for (auto nId : layer) {
      for (auto succ : dag.nodes[nId].succs) {
        preds[succ].push_back(nId);
      }
    }
  }
  // Keep track of the nodes connected to the "X" cross nodes
  // so that this shuffling does not accidentally change the meaning of the crossing
  Vec2<size_t> const leftNodes = findForcedLeftNodesBecauseOfCrossings(dag, preds);
  minimizeCrossingsForward(layers, dag, preds, leftNodes);
  log << "--- after first forward ---\n";
  log << renderDAGWithLayers(dag, layers) << "\n-----\n";
  minimizeCrossingsBackward(layers, dag, preds, leftNodes);
  log << "--- after backward ---\n";
  log << renderDAGWithLayers(dag, layers) << "\n-----\n";
  minimizeCrossingsForward(layers, dag, preds, leftNodes);
}

bool drawEdge(
  Position fromPos,
  Direction exitDir,
  Position to,
  Direction entryDir,
  Canvas& canvas
) {
  assert(fromPos.line + 1 < to.line && to.line < canvas.height());
  assert(fromPos.col < canvas.width() && to.col < canvas.width());

  Vec<EdgeStep> drawnPath;
  drawnPath.emplace_back();
  drawnPath.back().initialPos = fromPos;
  drawnPath.back().initialDir = exitDir;
  drawnPath.back().nextDir = exitDir;
  fromPos = nextPosInDir(fromPos, exitDir, exitDir);
  canvas.newMark(fromPos, edgeChar(exitDir));
  drawnPath.back().markedPos = fromPos;

  if (fromPos.line + 1 == to.line) {
    return exitDir == entryDir;
  }

  bool succeded = false;
  do {
    eraseAndBacktrackToLastChoice(drawnPath, canvas);
    succeded = tryDrawLine(to, entryDir, canvas, drawnPath);
  } while (!succeded && !drawnPath.empty());

  to.line -= 1;
  to.col -= directionShift(entryDir);
  canvas.newMark(to, edgeChar(entryDir));
  return succeded;
}

std::string renderDAGWithLayers(DAG const& dag, std::vector<std::vector<size_t>> layers) {
  // TODO: find best horisontal position of nodes
  auto coords = computeNodeCoordinates(dag, layers);
  auto const connectivity = computeConnectivity(dag, coords);
  adjustCoordsWithValencies(coords, connectivity, layers);
  auto canvas = Canvas::create(coords);
  placeNodes(dag, coords, canvas);
  placeEdges(coords, connectivity.edges, canvas);
  return canvas.render();
}

} // namespace detail

std::optional<string> renderDAG(DAG dag, RenderError& err) {
  err.code = RenderError::Code::None;
  if (dag.nodes.empty()) {
    return "";
  }
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
  log << "--- before min crossings ---\n";
  log << renderDAGWithLayers(dag, layers) << "\n-----\n";
  minimizeCrossings(layers, dag);
  log << "--- after min crossings ---\n";
  log << renderDAGWithLayers(dag, layers) << "\n-----\n";

  for (int i = 0; i < 5; ++i) {
    if (countAllCrossings(layers, dag) == 0) {
      break;
    }
    layers = insertCrossNodes(dag, layers);
    log << "--- after insert X ---\n";
    log << renderDAGWithLayers(dag, layers) << "\n-----\n";
    minimizeCrossings(layers, dag);
    log << "--- after min crossing in the loop ---\n";
    log << renderDAGWithLayers(dag, layers) << "\n-----\n";
  }

  return renderDAGWithLayers(dag, layers);
}

size_t maxLineWidth(string_view str) {
  size_t ret = 0;
  size_t curLine = 0;
  for (char c : str) {
    ++curLine;
    if (c == '\n') {
      ret = std::max(ret, curLine);
      curLine = 0;
    }
  }
  return std::max(ret, curLine);
}

std::optional<DAG> parseDAG(string_view str, ParseError& err) {
  NodeCollector collector(maxLineWidth(str));
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
    } else if (auto dir = edgeChar(c)) {
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

string parseErrorCodeToStr(ParseError::Code code) {
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

std::ostream& operator<<(std::ostream& os, DAG::Node const& node) {
  return os << "'" << node.text << "'->" << node.succs;
}

std::ostream& operator<<(std::ostream& os, ParseError const& err) {
  return os
      << "ERROR: " << parseErrorCodeToStr(err.code) << " at " << err.pos << ":" << err.message;
}

std::ostream& operator<<(std::ostream& os, DAG const& dag) {
  os << "DAG{";
  bool first = true;
  for (auto const& node : dag.nodes) {
    if (!first) {
      os << ", ";
    }
    os << node.text << "->[";
    bool firstInner = true;
    for (size_t succ : node.succs) {
      if (!firstInner) {
        os << ", ";
      }
      os << dag.nodes[succ].text;
      firstInner = false;
    }
    os << "]";
    first = false;
  }
  return os << "}";
}

string toDOT(DAG const& dag) {
  static string const idPrefix = "n";
  static string const indent = "  ";
  string ret = "digraph \"DAG\" {\n";
  for (size_t i = 0; i < dag.nodes.size(); ++i) {
    auto iStr = std::to_string(i);
    ret +=
      indent + idPrefix + iStr + "[shape=record,label=\"" + escapeForDOTlabel(dag.nodes[i].text)
      + "\"];\n";
    for (size_t succ : dag.nodes[i].succs) {
      ret += indent + idPrefix + iStr + " -> " + idPrefix + std::to_string(succ) + ";\n";
    }
    ret += "\n";
  }
  return ret + "}\n";
}

bool Canvas::inBounds(Position const& pos) const {
  return pos.line < lines.size() && pos.col < lines[0].size();
}

void Canvas::newMark(Position const& pos, char c) {
  assert(inBounds(pos));
  assert(lines[pos.line][pos.col] == ' ');
  assert(sketchMode || c != ' ');
  lines[pos.line][pos.col] = c;
}

void Canvas::clearPos(Position const& pos) {
  assert(inBounds(pos));
  assert(lines[pos.line][pos.col] != ' ');
  lines[pos.line][pos.col] = ' ';
}

char Canvas::getChar(Position const& pos) const {
  assert(inBounds(pos));
  return lines[pos.line][pos.col];
}

size_t Canvas::width() const {
  return lines[0].size();
}

size_t Canvas::height() const {
  return lines.size();
}

string Canvas::render() const {
  string ret;
  ret.reserve(lines.size() * (lines[0].size() + 1));
  for (auto const& line : lines) {
    ret += rtrim(line) + "\n";
  }
  return ret;
}

Canvas Canvas::create(Vec<Position> const& coordinates) {
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
  Canvas ret;
  ret.lines = Vec<string>(max.line + 1, string(max.col + 2, ' '));
  return ret;
}

Canvas Canvas::fromString(string const& rendered) {
  Canvas ret;
  string curLine;
  std::istringstream ss(rendered);
  size_t width = 0;
  while (getline(ss, curLine, '\n')) {
    width = std::max(width, curLine.size());
    ret.lines.push_back(curLine);
  }
  for (auto& line : ret.lines) {
    line.resize(width, ' ');
  }
  return ret;
}

} // namespace asciidag
