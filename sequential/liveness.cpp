#include "df.hpp"

class Liveness : public DataFlowHelper
{
    std::vector<int> flow(int n)
    {
    }

    void read_from_file(std::string filename)
    {
        
    }

private:
    std::vector<std::vector<int>> use;
    std::vector<std::vector<int>> def;
};

int main(int argc, char **argv)
{
    DataFlowHelper liveness = DataFlowHelper(100);
}
