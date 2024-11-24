// Copyright 2024 KASYM-TECH

#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <vector>

const int INF = std::numeric_limits<int>::max();

struct Edge {
  int to, weight;
};

std::vector<std::vector<Edge>> createRandomGraph(int numNodes, int maxWeight) {
  std::vector<std::vector<Edge>> graph(numNodes);

  // Initialize random number generators
  std::mt19937 rng(
      static_cast<unsigned int>(std::time(nullptr)));  // Mersenne Twister RNG
  std::uniform_int_distribution<int> coinFlip(0,
                                              1);  // For 0 or 1 random values
  std::uniform_int_distribution<int> weightDist(
      1, maxWeight);  // For weights in range [1, maxWeight]

  for (int i = 0; i < numNodes; ++i) {
    for (int j = 0; j < numNodes; ++j) {
      if (i != j && coinFlip(rng)) {   // Randomly decide to add an edge
        int weight = weightDist(rng);  // Generate a random weight
        graph[i].push_back({j, weight});
      }
    }
  }
  return graph;
}

std::vector<int> dijkstra(const std::vector<std::vector<Edge>>& graph,
                          int startNode) {
  int n = graph.size();
  std::vector<int> distance(n, INF);
  distance[startNode] = 0;

  std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
                      std::greater<>>
      pq;
  pq.push({0, startNode});

  while (!pq.empty()) {
    int dist = pq.top().first;
    int node = pq.top().second;
    pq.pop();

    if (dist > distance[node]) continue;

    for (const Edge& edge : graph[node]) {
      int newDist = dist + edge.weight;
      if (newDist < distance[edge.to]) {
        distance[edge.to] = newDist;
        pq.push({newDist, edge.to});
      }
    }
  }
  return distance;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <nodes_count>\n";
    return 1;
  }

  int nodes_count = std::stoi(argv[1]);
  std::vector<std::vector<Edge>> graph = createRandomGraph(nodes_count, 100);
  std::vector<int> distances = dijkstra(graph, 0);
  return 0;
}
