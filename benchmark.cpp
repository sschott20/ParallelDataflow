#include "parallel/liveness_color.hpp"
#include "sequential/checker.cpp"
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>

void run_benchmark(const std::string &filename)
{
    // Run sequential version
    {
        std::cout << "Running sequential version...\n";
        auto start = std::chrono::high_resolution_clock::now();

        Liveness live(0);
        live.read_from_file(filename);
        live.solve();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Sequential time: " << duration.count() << "ms\n";
    }

    // Run parallel version
    {
        std::cout << "Running parallel version...\n";
        auto start = std::chrono::high_resolution_clock::now();

        ParallelLivenessColor live;
        live.read_from_file(filename);
        live.solve();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Parallel time: " << duration.count() << "ms\n";
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    run_benchmark(argv[1]);
    return 0;
}