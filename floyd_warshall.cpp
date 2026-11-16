// Floyd-Warshall (all-pairs shortest paths) - implementation and running-time experiment
// DAA Assignment (CIC-311) - Experimental Analysis of Algorithms
// Compile: g++ -O2 -std=c++17 floyd_warshall.cpp -o floyd_warshall

#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <iomanip>
using namespace std;

const int INF = 1000000000;   // means "no edge"

// ---------- simple random number generator (same numbers on every computer) ----------
unsigned long long seed = 12345;
int nextRandom() {
    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)((seed >> 33) % 1000000);
}

// ---------- the algorithm ----------
void floydWarshall(vector<vector<int>>& d) {
    int V = d.size();
    for (int k = 0; k < V; k++) {            // allow vertex k as an intermediate vertex
        for (int i = 0; i < V; i++) {
            if (d[i][k] == INF) continue;     // no path i -> k, nothing to improve
            for (int j = 0; j < V; j++) {
                if (d[k][j] != INF && d[i][k] + d[k][j] < d[i][j])
                    d[i][j] = d[i][k] + d[k][j];
            }
        }
    }
}

// ---------- random directed graph as an adjacency matrix ----------
// each edge exists with probability 50%, weight 1..100
vector<vector<int>> makeGraph(int V) {
    vector<vector<int>> d(V, vector<int>(V, INF));
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (i == j) d[i][j] = 0;
            else if (nextRandom() % 2 == 0) d[i][j] = 1 + nextRandom() % 100;
        }
    }
    return d;
}

// ---------- Bellman-Ford from one source, used only to check correctness ----------
vector<int> bellmanFord(const vector<vector<int>>& g, int src) {
    int V = g.size();
    vector<int> dist(V, INF);
    dist[src] = 0;
    for (int pass = 1; pass < V; pass++) {
        for (int u = 0; u < V; u++) {
            if (dist[u] == INF) continue;
            for (int v = 0; v < V; v++) {
                if (g[u][v] != INF && u != v && dist[u] + g[u][v] < dist[v])
                    dist[v] = dist[u] + g[u][v];
            }
        }
    }
    return dist;
}

// ---------- one timed run ----------
// Very fast runs (less than 50 ms) are repeated and averaged so the clock can measure them.
double timeOneRun(const vector<vector<int>>& graph) {
    double total = 0;
    int reps = 0;
    while (total < 50.0) {
        vector<vector<int>> d = graph;              // fresh copy, copying is not timed
        auto start = chrono::steady_clock::now();
        floydWarshall(d);
        auto stop = chrono::steady_clock::now();
        total += chrono::duration<double, milli>(stop - start).count();
        reps++;
    }
    return total / reps;
}

int main() {
    // ----- 1. correctness check: compare with Bellman-Ford from every vertex -----
    bool ok = true;
    for (int t = 0; t < 5; t++) {
        vector<vector<int>> g = makeGraph(20 + t * 10);
        vector<vector<int>> d = g;
        floydWarshall(d);
        for (int s = 0; s < (int)g.size(); s++)
            if (bellmanFord(g, s) != d[s]) ok = false;
    }
    cout << "Floyd-Warshall correctness check (vs Bellman-Ford): " << (ok ? "PASSED" : "FAILED") << "\n\n";
    if (!ok) return 1;

    // ----- 2. timing experiment (n = number of vertices) -----
    int sizes[] = {50, 100, 200, 400, 800, 1600};
    ofstream csv("results_floyd.csv");
    csv << "algorithm,case,n,run1_ms,run2_ms,run3_ms,avg_ms\n";
    cout << setw(8) << "V" << setw(12) << "Run1(ms)" << setw(12) << "Run2(ms)"
         << setw(12) << "Run3(ms)" << setw(12) << "Avg(ms)" << setw(10) << "T2n/Tn" << "\n";
    double prevAvg = 0;
    for (int V : sizes) {
        vector<vector<int>> g = makeGraph(V);
        double r[3];
        for (int k = 0; k < 3; k++) r[k] = timeOneRun(g);
        double avg = (r[0] + r[1] + r[2]) / 3;
        cout << fixed << setprecision(4) << setw(8) << V << setw(12) << r[0] << setw(12) << r[1]
             << setw(12) << r[2] << setw(12) << avg;
        if (prevAvg > 0) cout << setw(10) << setprecision(2) << avg / prevAvg;
        else cout << setw(10) << "-";
        cout << "\n";
        csv << fixed << setprecision(6) << "Floyd-Warshall,dense random graph," << V << ","
            << r[0] << "," << r[1] << "," << r[2] << "," << avg << "\n";
        prevAvg = avg;
    }
    csv.close();
    cout << "\nResults saved to results_floyd.csv\n";
    return 0;
}
