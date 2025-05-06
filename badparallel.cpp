#include "df.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <set>
#include <algorithm>
#include <iterator>
#include <unordered_map>

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/slice.h>

template <typename T>
void print_vector(const std::vector<T> &v)
{
    for (const auto &s : v)
    {
        std::cout << s << " ";
    }
    std::cout << "\n";
}

class Liveness : public DataFlowHelper
{
public:
    Liveness(int N) : DataFlowHelper(N)
    {
    }

    std::vector<std::string> flow(int n)
    {
        std::vector<std::string> new_out;
        std::vector<std::string> dest;

        std::set_difference(this->out[n].begin(), this->out[n].end(),
                            this->def[n].begin(), this->def[n].end(),
                            std::back_inserter(dest));
        std::set_union(dest.begin(), dest.end(),
                       this->use[n].begin(), this->use[n].end(),
                       std::back_inserter(new_out));

        return new_out;
    }
    std::vector<std::string> combine(int n)
    {
        std::vector<std::string> new_in;

        for (int i = 0; i < this->succs[n].size(); i++)
        {

            std::vector<std::string> dest;
            std::set_union(new_in.begin(), new_in.end(),
                           this->in[this->succs[n][i]].begin(), this->in[this->succs[n][i]].end(),
                           std::back_inserter(dest));
            std::sort(dest.begin(), dest.end());
            new_in = dest;
        }
        return new_in;
    }
    void solve()
    {
        int threads = 16;

        std::vector<std::vector<int>> frontiers;
        for (int i = 0; i < threads; i++)
        {
            frontiers.push_back(std::vector<int>());
        }
        std::vector<int> frontier;

        for (int i = 0; i < this->N; i++)
        {
            frontier.push_back(i);
        }

        while (frontier.size() > 0)
        {
            auto size = frontier.size();
            if (size < 32)
            {
                threads = 1;
            }
#pragma omp parallel for
            for (int id = 0; id < threads; id++)
            {
                for (int i = id * size / threads; i < (id + 1) * size / threads; i++)
                {
                    frontiers[id].clear();
                    int n = frontier[i];
                    std::vector<std::string> old_in = this->in[n];
                    auto newout = combine(n);
                    auto newin = flow(n);
                    // print_vector(newin);
                    // print_vector(old_in);
                    // std::cout << "are they equal? " << (newin == old_in) << "\n";
                    if (newin != old_in)
                    {
                        this->in[n] = newin;
                        this->out[n] = newout;
                        for (int i = 0; i < this->preds[n].size(); i++)
                        {
                            frontiers[id].push_back(this->preds[n][i]);
                        }
                    }
                }
            }

            frontier.clear();

            // for (int i = 0; i < threads; i++)
            // {
            //     print_vector(frontiers[i]);
            // }

            // merge frontiers into a single frontier, removing duplicates
            for (int i = 0; i < threads; i++)
            {
                std::vector<int> temp = frontiers[i];
                frontier.insert(frontier.end(), temp.begin(), temp.end());
            }
            // std::sort(frontier.begin(), frontier.end());
            frontier.erase(std::unique(frontier.begin(), frontier.end()), frontier.end());

            // std::cout << "Frontier: ";
            // for (int i = 0; i < frontier.size(); i++)
            // {
            //     std::cout << frontier[i] << " ";
            // }
            // std::cout << "\n";
        }

        // std::queue<int> w;
        // for (int i = 0; i < N; i++)
        // {
        //     w.push(i);
        // }

        // while (w.size() > 0)
        // {
        //     int n = w.front();
        //     w.pop();

        //     std::vector<std::string> old_in = this->in[n];
        //     this->out[n] = combine(n);
        //     this->in[n] = flow(n);
        //     if (this->in[n] != old_in)
        //     {
        //         for (int i = 0; i < this->preds[n].size(); i++)
        //         {
        //             w.push(this->preds[n][i]);
        //         }
        //     }
        // }

        // gather the results and print them out

        std::unordered_map<std::string, std::set<std::string>> solution;
        for (int i = 0; i < this->N; i++)
        {
            for (int j = 0; j < this->in[i].size(); j++)
            {
                for (int k = j; k < this->in[i].size(); k++)
                {
                    if (solution.find(this->in[i][j]) == solution.end())
                    {
                        solution[this->in[i][j]] = std::set<std::string>(); // Or appropriate initialization
                    }
                    if (solution.find(this->in[i][k]) == solution.end())
                    {
                        solution[this->in[i][k]] = std::set<std::string>();
                    }
                    solution[this->in[i][j]].insert(this->in[i][k]);
                    solution[this->in[i][k]].insert(this->in[i][j]);
                }
            }
        }
        // print out unordered map
        for (const auto &pair : solution)
        {
            std::cout << pair.first << ": ";
            for (const auto &s : pair.second)
            {
                std::cout << s << " ";
            }
            std::cout << "\n";
        }
    }
    void read_from_file(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error opening file: " << filename << "\n";
            return;
        }
        file >> this->N;

        this->use = std::vector<std::vector<std::string>>(N);
        this->def = std::vector<std::vector<std::string>>(N);

        this->succs = std::vector<std::vector<int>>(N);
        this->preds = std::vector<std::vector<int>>(N);

        this->in = std::vector<std::vector<std::string>>(N);
        this->out = std::vector<std::vector<std::string>>(N);

        int def_size, use_size, succ_size;

        std::string s;
        int x;
        for (int i = 0; i < this->N; i++)
        {
            // std::cout << "Node " << i << "\n";
            this->use.push_back(std::vector<std::string>());
            this->def.push_back(std::vector<std::string>());
            this->succs.push_back(std::vector<int>());

            file >> def_size;
            // std::cout << "Def size: " << def_size << "\n";
            for (int j = 0; j < def_size; j++)
            {
                file >> s;
                this->def[i].push_back(s);
            }

            file >> use_size;
            // std::cout << "Use size: " << use_size << "\n";
            for (int j = 0; j < use_size; j++)
            {
                file >> s;
                this->use[i].push_back(s);
            }

            file >> succ_size;
            // std::cout << "Succ size: " << succ_size << "\n";
            for (int j = 0; j < succ_size; j++)
            {
                file >> x;
                // std::cout << "Succ: " << x << "\n";
                this->succs[i].push_back(x);
                if (this->preds[x].size() == 0)
                {
                    this->preds[x] = std::vector<int>();
                }
                this->preds[x].push_back(i);
            }
        }
    }

    void print_debug()
    {
        std::cout << "Liveness: \n";
        for (int i = 0; i < this->N; i++)
        {
            std::cout << "Node " << i << ": \n";
            std::cout << "Def   : ";
            for (int j = 0; j < this->def[i].size(); j++)
            {
                std::cout << this->def[i][j] << " ";
            }
            std::cout << "\nUse   : ";
            for (int j = 0; j < this->use[i].size(); j++)
            {
                std::cout << this->use[i][j] << " ";
            }

            std::cout << "\nSuccs : ";
            for (int j = 0; j < this->succs[i].size(); j++)
            {
                std::cout << this->succs[i][j] << " ";
            }

            std::cout << "\nPreds : ";
            for (int j = 0; j < this->preds[i].size(); j++)
            {
                std::cout << this->preds[i][j] << " ";
            }

            std::cout << "\nIn    : ";
            for (int j = 0; j < this->in[i].size(); j++)
            {
                std::cout << this->in[i][j] << " ";
            }

            std::cout << "\nOut   : ";
            for (int j = 0; j < this->out[i].size(); j++)
            {
                std::cout << this->out[i][j] << " ";
            }

            std::cout << "\n\n";
        }
    }

private:
    std::vector<std::vector<std::string>> use;
    std::vector<std::vector<std::string>> def;
};

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    Liveness live = Liveness(0);
    live.read_from_file(argv[1]);
    live.solve();
}
