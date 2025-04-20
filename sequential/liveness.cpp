#include "df.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <set>
#include <algorithm>
#include <iterator>

class Liveness : public DataFlowHelper
{
public:
    Liveness(int N) : DataFlowHelper(N)
    {
    }

    std::vector<std::string> flow(int n)
    {
        std::cout << "flowing node " << n << "\n";
        std::vector<std::string> new_out;
        std::sort(this->in[n].begin(), this->in[n].end());

        std::set_difference(this->in[n].begin(), this->in[n].end(),
                            this->def[n].begin(), this->def[n].end(),
                            std::back_inserter(new_out));

        std::set_union(new_out.begin(), new_out.end(),
                       this->use[n].begin(), this->use[n].end(),
                       std::back_inserter(new_out));
        return new_out;
    }
    std::vector<std::string> combine(int n, std::vector<int> &preds)
    {

        std::vector<std::string> new_in;
        std::vector<std::string> dest;
        for (int i = 0; i < this->succs[n].size(); i++)
        {
            // loop over all out[n'] for all predecessors n'
            std::set_union(new_in.begin(), new_in.end(),
                           this->out[preds[i]].begin(), this->out[preds[i]].end(),
                           std::back_inserter(dest));
            std::sort(dest.begin(), dest.end());
            new_in = dest;
        }
        return new_in;
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
            this->use.push_back(std::vector<std::string>());
            this->def.push_back(std::vector<std::string>());
            this->succs.push_back(std::vector<int>());

            file >> def_size;
            for (int j = 0; j < def_size; j++)
            {
                file >> s;
                this->def[i].push_back(s);
            }

            file >> use_size;
            for (int j = 0; j < use_size; j++)
            {
                file >> s;
                this->use[i].push_back(s);
            }

            file >> succ_size;
            for (int j = 0; j < succ_size; j++)
            {
                file >> x;
                this->succs[i].push_back(x);
                this->preds[x].push_back(i);
            }

            std::sort(this->def[i].begin(), this->def[i].end());
            std::sort(this->use[i].begin(), this->use[i].end());
            std::sort(this->succs[i].begin(), this->succs[i].end());
            std::sort(this->preds[i].begin(), this->preds[i].end());
        }
    }
    void print()
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
    Liveness live = Liveness(9);
    live.read_from_file("../easyliveness.txt");
    live.solve();
    live.print();
}
