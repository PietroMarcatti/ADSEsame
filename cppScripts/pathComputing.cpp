#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <tuple>
#include <limits>
#include <unordered_map>
#include <set>
#include <string>
#include <omp.h>
#include <cstdio>  
typedef unsigned char u8;
typedef unsigned short u16;

struct Edge {
    u8 to;
    u8 type;
    u16 id;
};

class Graph {
public:
    std::vector<std::vector<Edge>> adjacency_list;

    Graph(u8 num_nodes) {
        adjacency_list.resize(num_nodes);
    }

    void addEdge(u8 from, u8 to, u8 type, u16 id) {
        adjacency_list[from].push_back({to, type, id});
        adjacency_list[to].push_back({from, type, id});
    }

    const std::vector<Edge>& getEdges(u8 node) const {
        return adjacency_list[node];
    }
};

struct Path {
    std::vector<u8> nodes;  
    std::vector<u16> edges;
    u8 length;              
    u8 type3_count;         
    u8 type4_count;         
};

Graph readGraphFromCSV(const std::string& filename, u16& max_node) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file.");
    }

    std::string line;
    Graph graph(196);

    max_node = -1;
    bool first = true;
    while (std::getline(file, line)) {

        std::istringstream ss(line);
        if (first) {
            first = false;
            continue;
        }
        std::vector<u16> row;
        u16 num;

        while (ss >> num) { 
            row.push_back(num);

            
            if (ss.peek() == ',') {
                ss.ignore();
            }
        }
        if (row[0] > 200) continue;
        graph.addEdge(row[0], row[1], row[2], row[4]);
    }

    std::istringstream iss(line);
    

    return graph;
}

std::vector<std::vector<Path>> findPathsWithConstraints(
    const Graph& graph, u8 start_node, u8 max_type3, u8 max_type4) {
    std::vector<std::vector<Path>> shortest(196, { {} });
    std::queue<Path> queue;
    std::vector<u8> shortestDistances;
    for (u8 i=0; i<196; i++){
        shortestDistances.push_back(65534);
    }

    queue.push({{start_node},{}, 0, 0, 0});
    while (!queue.empty()) {
        Path current = queue.front();
        queue.pop();
        std::vector<Edge> edge_list = graph.getEdges(current.nodes.back());
        for (int edge_num = 0; edge_num < edge_list.size();edge_num++) {
            Edge edge = edge_list[edge_num];
            if (current.length + 1 > shortestDistances[edge.to]) {
                continue;
            }
            u8 new_type3_count = current.type3_count + (edge.type == 3);
            u8 new_type4_count = current.type4_count + (edge.type == 4);

            if (new_type3_count > max_type3 || new_type4_count > max_type4) {
                continue;
            }
            if (std::find(current.nodes.begin(), current.nodes.end(), edge.to) !=
                current.nodes.end()) {
                continue;
            }

            Path new_path = current;
            new_path.nodes.push_back(edge.to);
            new_path.edges.push_back(edge.id);
            new_path.length++;
            new_path.type3_count = new_type3_count;
            new_path.type4_count = new_type4_count;            

            if (edge.type == 0){
                if (new_path.length < shortestDistances[edge.to]) {
                    
                    shortestDistances[edge.to] = new_path.length;
                    shortest[edge.to] = { new_path };
                }
                else if (new_path.length == shortestDistances[edge.to]) {
                    shortest[edge.to].push_back(new_path);
                }
            }
            queue.push(new_path);
        }
    }
    return shortest;
}

void writePathsToCSV(const std::string& filename, const std::vector<std::vector<Path>>& nodes_paths, const bool header) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file.");
    }
    if (header)
        file <<"from,to,cost,node1,node2,node3,node4,node5,node6,node7,node8,node9,node10,node11,edge1,edge2,edge3,edge4,edge5,edge6,edge7,edge8,edge9,edge10\n";
    for (const auto& node_paths : nodes_paths){
        for (const auto& path : node_paths) {
            if (path.length == 0) continue;
            u8 start_node = path.nodes.front();
            file << static_cast<int>(start_node) << ",";
            file << static_cast<int>(path.nodes.back()) << ",";
            file << static_cast<int>(path.length) << ",";

            for (size_t i = 0; i < 11; ++i) {
                if (i < path.nodes.size()) {
                    file << static_cast<int>(path.nodes[i]);
                } else {
                    file << "0";
                }
                file << ",";
            }

            for (size_t i = 0; i < 10; ++i) {
                if (i < path.edges.size()) {
                    file << static_cast<int>(path.edges[i]);
                } else {
                    file << "0";
                }
                if (i < 9) {
                    file << ",";
                }
            }

            file << "\n";
        }
    }

    file.close();
}

void deleteFile(std::string fileName) {
    if (remove(fileName.c_str()) == 0) {
       std::cout << "Existing file deleted successfully\n";
    }
}

int main() {
    try {
        std::string filename = "./datasets/all_edges.csv";
        u16 max_node;
        Graph graph = readGraphFromCSV(filename, max_node);

        u8 max_type3 = 3;
        u8 max_type4 = 3;
        bool header = false;

        std::vector<u8> start_nodes = {3, 21, 27, 65, 84, 147, 149, 158};
        std::vector<std::string> file_names = { "./datasets/n1.csv","./datasets/n2.csv" ,"./datasets/n3.csv" ,"./datasets/n4.csv" };
        for (std::string name : file_names) {
            deleteFile(name);
        }
        for (u8 night = 0; night < 4; night++) {
            if (night % 2 == 0)max_type3--;
            if (night % 2 == 1)max_type4--;
            #pragma omp parallel for
            for (int start_node = 0; start_node < 8; start_node++) {
                std::vector<std::vector<Path>> paths = findPathsWithConstraints(graph, start_nodes[start_node], max_type3 , max_type4 );
                #pragma omp critical
                if (header == false) {
                    header = true;
                    writePathsToCSV(file_names[night], paths, true);
                }
                else {
                    writePathsToCSV(file_names[night], paths, false);
                }
            }
            header = false;
            std::cout << "Notte " << night + 1 << " completata \n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}