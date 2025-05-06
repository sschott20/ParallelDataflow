#pragma once

#include "../parlaylib/include/parlay/sequence.h"
#include "../parlaylib/include/parlay/parallel.h"
#include "../parlaylib/include/parlay/primitives.h"
#include "../parlaylib/include/parlay/delayed.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <iostream>
#include <atomic>
#include <mutex>
#include <map>

class ParallelLivenessColor
{
private:
    int N;                                     // Number of nodes
    std::vector<std::vector<int>> preds;       // Predecessors for each node
    std::vector<std::vector<int>> succs;       // Successors for each node
    std::vector<std::vector<std::string>> def; // Defined variables at each node
    std::vector<std::vector<std::string>> use; // Used variables at each node
    std::vector<std::vector<std::string>> in;  // Live variables at entry of each node
    std::vector<std::vector<std::string>> out; // Live variables at exit of each node

    // Interference graph representation
    std::vector<std::vector<int>> interference_graph;
    std::vector<int> colors; // Colors assigned to variables

    // Helper function to get variable index from name
    std::map<std::string, int> var_to_idx;
    std::vector<std::string> idx_to_var;

    // Synchronization
    std::mutex mtx;            // Mutex for synchronization
    std::atomic<bool> changed; // Flag to track changes in liveness analysis

    void build_interference_graph();
    void parallel_color_graph();

public:
    ParallelLivenessColor() : N(0), changed(false) {}

    void read_from_file(const std::string &filename);
    void solve();
    void print_results();
};