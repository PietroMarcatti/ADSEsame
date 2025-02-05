#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <set>

using namespace std;

const int NUM_NODES = 196;
const int NUM_PATH_COLUMNS = 11;
const string EDGE_FILE = "./datasets/all_edges.csv";
const vector<string> PATH_FILES = { "./datasets/n1.csv", "./datasets/n2.csv", "./datasets/n3.csv", "./datasets/n4.csv" };
const string OUTPUT_FILE = "./datasets/path_matrix.csv";

unordered_map<int, int, hash<int>> loadEdgeSteps(const string& edgeFile) {
    unordered_map<int, int, hash<int>> edgeSteps;
    ifstream file(edgeFile);
    string line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        stringstream ss(line);
        int from, to, type, step, id;
        char comma;
        ss >> from >> comma >> to >> comma >> type >> comma >> step >> comma >> id;
        if (type == 4) {
            edgeSteps[id] = step;
        }
    }
    return edgeSteps;
}

vector<vector<int>> buildMatrix(const unordered_map<int, int, hash<int>>& edgeSteps) {
    vector<vector<int>> matrix;

    #pragma omp parallel for
    for (int i = 0; i < PATH_FILES.size(); i++) {
        string fileName = PATH_FILES[i];
        ifstream file(fileName);
        string line;
        int row_count = 0;
        getline(file, line); // Skip header
        while (getline(file, line)) {
            row_count++;
            stringstream ss(line);
            vector<int> row(NUM_NODES, 0);
            int from, to, cost, node, edge, lastNode;
            char comma;
            ss >> from >> comma >> to >> comma >> cost;

            unordered_set<int> visited;
            #pragma omp parallel for
            for (int i = 0; i < NUM_PATH_COLUMNS; ++i) {
                if (ss >> comma >> node && node != 0) {
                    visited.insert(node);
                    lastNode = node;
                }
            }
            #pragma omp parallel for
            for (int i = 0; i < NUM_PATH_COLUMNS - 1; ++i) {
                if (ss >> comma >> edge && edge != 0) {
                    auto it = edgeSteps.find(edge);
                    if (it != edgeSteps.end()) {
                        visited.insert(it->second);
                    }
                }
            }
            row[0] = to;
            for (int v : visited) {
                row[v] = 1;
            }
            #pragma omp critical
            matrix.push_back(row);
        }
    }
    return matrix;
}

void writeMatrixToCSV(const vector<vector<int>>& matrix) {
    ofstream outFile(OUTPUT_FILE);
    int row_count = 0;
    for (const auto& row : matrix) {
        for (size_t i = 0; i < NUM_NODES; ++i) {
            outFile << row[i] << (i < row.size() - 1 ? "," : "\n");
        }
    }
}

int main() {
    auto edgeSteps = loadEdgeSteps(EDGE_FILE);
    auto matrix = buildMatrix(edgeSteps);
    writeMatrixToCSV(matrix);
    return 0;
}