#include "liveness_color.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

void ParallelLivenessColor::build_interference_graph()
{
    // First, collect all variables and assign indices
    std::set<std::string> all_vars;
    for (int i = 0; i < N; i++)
    {
        for (const auto &v : def[i])
            all_vars.insert(v);
        for (const auto &v : use[i])
            all_vars.insert(v);
    }

    // Create variable mapping
    int idx = 0;
    for (const auto &v : all_vars)
    {
        var_to_idx[v] = idx;
        idx_to_var.push_back(v);
        idx++;
    }

    // Initialize interference graph
    int num_vars = all_vars.size();
    interference_graph.resize(num_vars);
    colors.resize(num_vars, -1);

    // Build interference graph in parallel
    std::vector<std::set<std::pair<int, int>>> thread_local_edges(N);
    parlay::parallel_for(0, N, [&](int i)
                         {
        // Variables that are live at the same time interfere
        for (const auto& v1 : in[i]) {
            for (const auto& v2 : in[i]) {
                if (v1 < v2) {  // Avoid duplicate edges
                    int idx1 = var_to_idx[v1];
                    int idx2 = var_to_idx[v2];
                    thread_local_edges[i].insert({idx1, idx2});
                }
            }
        } });

    // Merge edges from all threads
    for (int i = 0; i < N; i++)
    {
        for (const auto &edge : thread_local_edges[i])
        {
            interference_graph[edge.first].push_back(edge.second);
            interference_graph[edge.second].push_back(edge.first);
        }
    }

    // Remove duplicates from interference graph
    parlay::parallel_for(0, num_vars, [&](int i)
                         {
        std::sort(interference_graph[i].begin(), interference_graph[i].end());
        interference_graph[i].erase(
            std::unique(interference_graph[i].begin(), interference_graph[i].end()),
            interference_graph[i].end()
        ); });
}

void ParallelLivenessColor::parallel_color_graph()
{
    int num_vars = idx_to_var.size();

    // Sort vertices by degree
    std::vector<int> vertex_order(num_vars);
    for (int i = 0; i < num_vars; i++)
    {
        vertex_order[i] = i;
    }

    std::sort(vertex_order.begin(), vertex_order.end(),
              [&](int a, int b)
              {
                  return interference_graph[a].size() > interference_graph[b].size();
              });

    // Color vertices in parallel
    parlay::parallel_for(0, num_vars, [&](int i)
                         {
        int v = vertex_order[i];
        std::vector<bool> used_colors(num_vars, false);
        
        // Mark colors used by neighbors
        for (int neighbor : interference_graph[v]) {
            if (colors[neighbor] != -1) {
                used_colors[colors[neighbor]] = true;
            }
        }
        
        // Find first available color
        for (int c = 0; c < num_vars; c++) {
            if (!used_colors[c]) {
                colors[v] = c;
                break;
            }
        } });
}

void ParallelLivenessColor::read_from_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }

    file >> N;

    // Initialize vectors with correct size
    preds.resize(N);
    succs.resize(N);
    def.resize(N);
    use.resize(N);
    in.resize(N);
    out.resize(N);

    // Read data sequentially since file operations cannot be parallelized safely
    for (int i = 0; i < N; i++)
    {
        int def_size;
        file >> def_size;
        def[i].reserve(def_size);
        for (int j = 0; j < def_size; j++)
        {
            std::string s;
            file >> s;
            def[i].push_back(s);
        }

        int use_size;
        file >> use_size;
        use[i].reserve(use_size);
        for (int j = 0; j < use_size; j++)
        {
            std::string s;
            file >> s;
            use[i].push_back(s);
        }

        int succ_size;
        file >> succ_size;
        succs[i].reserve(succ_size);
        for (int j = 0; j < succ_size; j++)
        {
            int x;
            file >> x;
            succs[i].push_back(x);
            preds[x].push_back(i);
        }
    }
}

void ParallelLivenessColor::solve()
{
    // First perform liveness analysis
    do
    {
        changed.store(false);

        // Create thread-local copies for in/out sets
        std::vector<std::vector<std::string>> new_in(N);
        std::vector<std::vector<std::string>> new_out(N);

        // Compute new in/out sets in parallel
        parlay::parallel_for(0, N, [&](int i)
                             {
            // Compute out[i] = union of in[j] for all successors j
            std::vector<std::string> local_out;
            {
                std::lock_guard<std::mutex> lock(mtx);
                for (int j : succs[i]) {
                    for (const auto& v : in[j]) {
                        local_out.push_back(v);
                    }
                }
            }
            std::sort(local_out.begin(), local_out.end());
            local_out.erase(std::unique(local_out.begin(), local_out.end()), local_out.end());

            // Compute in[i] = use[i] union (out[i] - def[i])
            std::vector<std::string> local_in = use[i];
            for (const auto& v : local_out) {
                if (std::find(def[i].begin(), def[i].end(), v) == def[i].end()) {
                    local_in.push_back(v);
                }
            }
            std::sort(local_in.begin(), local_in.end());
            local_in.erase(std::unique(local_in.begin(), local_in.end()), local_in.end());

            new_in[i] = local_in;
            new_out[i] = local_out; });

        // Update global in/out sets and check for changes
        bool local_changed = false;
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (int i = 0; i < N; i++)
            {
                if (new_in[i] != in[i] || new_out[i] != out[i])
                {
                    local_changed = true;
                    in[i] = std::move(new_in[i]);
                    out[i] = std::move(new_out[i]);
                }
            }
            if (local_changed)
            {
                changed.store(true);
            }
        }
    } while (changed.load());

    // Build and color interference graph
    build_interference_graph();
    parallel_color_graph();
}

void ParallelLivenessColor::print_results()
{
    // Print variable coloring results
    for (int i = 0; i < idx_to_var.size(); i++)
    {
        std::cout << idx_to_var[i] << ": color " << colors[i] << "\n";
    }
}
// int main(int argc, char **argv)
// {
//     if (argc != 2)
//     {
//         std::cerr << "Usage: " << argv[0] << " <filename>\n";
//         return 1;
//     }

//     ParallelLivenessColor live;
//     live.read_from_file(argv[1]);
//     live.solve();
//     live.print_results();

//     return 0;
// }
