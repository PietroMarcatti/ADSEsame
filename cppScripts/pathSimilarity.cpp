#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <set>
#include <cstdlib>

using namespace std;

constexpr int NODES = 11;
constexpr int EDGES = 10;

// Structure to store path data efficiently
struct PathData {
    bitset<NODES> nodes;
    bitset<EDGES> edges;
};

double computeIoU(const bitset<NODES>& a, const bitset<NODES>& b) {
    int intersection = (a & b).count();
    int union_count = (a | b).count();
    return (union_count == 0) ? 0.0 : static_cast<double>(intersection) / union_count;
}

double computeIoU(const bitset<EDGES>& a, const bitset<EDGES>& b) {
    int intersection = (a & b).count();
    int union_count = (a | b).count();
    return (union_count == 0) ? 0.0 : static_cast<double>(intersection) / union_count;
}

int main() {
    ifstream file("./datasets/all_paths.csv");
    if (!file.is_open()) {
        cerr << "Error opening file.\n";
        return 1;
    }

    string line;
    getline(file, line);  // Skip header

    unordered_map<int, vector<PathData>> to_group; // Group paths by "to"

    // Read CSV file
    while (getline(file, line)) {
        stringstream ss(line);
        string cell;

        int from, to, cost;
        bitset<NODES> nodes;
        bitset<EDGES> edges;

        // Parse first three columns
        getline(ss, cell, ','); from = stoi(cell);
        getline(ss, cell, ','); to = stoi(cell);
        getline(ss, cell, ','); cost = stoi(cell);

        // Parse nodes (exclude 0 values)
        for (int i = 0; i < NODES; ++i) {
            getline(ss, cell, ',');
            if (!cell.empty() && stoi(cell) != 0) nodes.set(i);
        }

        // Parse edges (exclude 0 values)
        for (int i = 0; i < EDGES; ++i) {
            getline(ss, cell, ',');
            if (!cell.empty() && stoi(cell) != 0) edges.set(i);
        }

        to_group[to].push_back({ nodes, edges });
    }
    file.close();

    ofstream output("./datasets/path_similarity.csv");
    if (!output.is_open()) {
        cerr << "Error opening output file.\n";
        return 1;
    }
    short fatti = 0;
    // Write CSV header
    output << "to,avg_node_iou,avg_edge_iou\n";
    // Compute IoU for each "to" group
#pragma omp parallel for
    for (int i = 1; i < 196; i++) {
        int to = i;
        vector<PathData> paths = to_group.at(i);
        int count = paths.size();

        long double totalIoU_nodes = 0.0, totalIoU_edges = 0.0;
        long pairs = 0;


        // Compute pairwise IoU
        #pragma omp parallel for
        for (int i = 0; i < paths.size(); ++i) {
            #pragma omp parallel for
            for (int j = i + 1; j < paths.size(); ++j) {
                #pragma omp critical
                {
                    totalIoU_nodes += computeIoU(paths[i].nodes, paths[j].nodes);
                    totalIoU_edges += computeIoU(paths[i].edges, paths[j].edges);
                    pairs++;
                }
            }
        }
        fatti++;
        system("cls");
        cout << fatti << "/195\n";
        // Output results
        if (pairs > 0) {
            output << to << ","
                << (totalIoU_nodes / pairs) << ","
                << (totalIoU_edges / pairs) << "\n";
        }
    }
    output.close();
    return 0;
}