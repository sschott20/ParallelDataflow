#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <queue>
#include <set>

// function which computes out[n] given some n, using in[n]
typedef std::vector<int> flowfptr(int n);

// function which computes in[n] for some n, using out[n'] for all predisessors n' of n
typedef std::vector<int>(combinefptr)(int n, std::vector<int> &preds);

class DataFlowHelper
{
public:
    DataFlowHelper(int N)
    {
        this->N = N;
    }

    virtual void solve()
    {
        std::queue<int> w;
        for (int i = 0; i < N; i++)
        {
            w.push(i);
        }

        while (w.size() > 0)
        {
            int n = w.front();
            w.pop();

            std::vector<std::string> old_in = this->in[n];
            this->out[n] = combine(n);
            this->in[n] = flow(n);
            if (this->in[n] != old_in)
            {
                for (int i = 0; i < this->preds[n].size(); i++)
                {
                    w.push(this->preds[n][i]);
                }
            }
        }
    }

    /// computes out[n] using in[n']
    virtual std::vector<std::string> flow(int n)
    {
        return out[n];
    }

    /// computes in[n] using out[n'] for all predecessors n'
    virtual std::vector<std::string> combine(int n)
    {
        return in[n];
    }

public:
    int N;
    std::vector<std::vector<int>> preds;
    std::vector<std::vector<int>> succs;

    std::vector<std::vector<std::string>> in;
    std::vector<std::vector<std::string>> out;
};