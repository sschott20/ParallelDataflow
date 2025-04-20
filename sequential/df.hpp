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

    void solve()
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

            std::vector<int> old_out = out[n];
            std::vector<int> in = combine(n, preds[n]);
            out[n] = flow(n);
            if (out[n] != old_out)
            {
                for (int i = 0; i < succs[n].size(); i++)
                {
                    w.push(succs[n][i]);
                }
            }
        }
    }
    virtual std::vector<int> flow(int n)
    {
        return out[n];
    }

    virtual std::vector<int> combine(int n, std::vector<int> &preds)
    {
        return in[n];
    }

public:
    int N;

private:
    std::vector<std::vector<int>> preds;
    std::vector<std::vector<int>> succs;

    std::vector<std::vector<int>> in;
    std::vector<std::vector<int>> out;
};