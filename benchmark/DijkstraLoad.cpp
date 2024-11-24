#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <limits>


const int INF = std::numeric_limits<int>::max();

struct Edge {
    int to, weight;
};

std::vector<std::vector<Edge>> createRandomGraph(int numNodes, int maxWeight) {
    std::vector<std::vector<Edge>> graph(numNodes);
    srand(time(nullptr));

    for (int i = 0; i < numNodes; ++i) {
        for (int j = 0; j < numNodes; ++j) {
            if (i != j && rand() % 2) {
                int weight = rand() % maxWeight + 1;
                graph[i].push_back({j, weight});
            }
        }
    }
    return graph;
}

std::vector<int> dijkstra(const std::vector<std::vector<Edge>>& graph, int startNode) {
    int n = graph.size();
    std::vector<int> distance(n, INF);
    distance[startNode] = 0;

    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;
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