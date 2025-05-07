import subprocess
import sys
import time
import os
from generate_tests import generate_linear_chain, generate_binary_tree, generate_mesh
import pandas as pd
from tabulate import tabulate

def compile_benchmark():
    """Compile the benchmark program"""
    cmd = [
        "g++",
        "-O3",
        "-std=c++17",
        "-pthread",
        "-I./parlaylib",
        "-o",
        "benchmark",
        "benchmark.cpp"
    ]
    try:
        subprocess.run(cmd, check=True)
        return True
    except subprocess.CalledProcessError:
        print("Failed to compile benchmark program")
        return False

def run_benchmark(test_file):
    """Run the benchmark program on a test file and return the results"""
    try:
        result = subprocess.run(["./benchmark", test_file], 
                              capture_output=True, 
                              text=True, 
                              check=True)
        output = result.stdout
        
        # Parse the output to get timing results
        sequential_time = None
        parallel_time = None
        
        for line in output.split('\n'):
            if "Sequential time:" in line:
                sequential_time = int(line.split(':')[1].strip().replace('ms', ''))
            elif "Parallel time:" in line:
                parallel_time = int(line.split(':')[1].strip().replace('ms', ''))
        
        return sequential_time, parallel_time
    except subprocess.CalledProcessError:
        print(f"Failed to run benchmark on {test_file}")
        return None, None

def generate_and_run_test(test_type, size, output_file):
    """Generate a test case and run the benchmark on it"""
    # Generate the test case
    with open(output_file, 'w') as f:
        if test_type == "linear":
            generate_linear_chain(size)
        elif test_type == "binary":
            generate_binary_tree(size)
        elif test_type == "mesh":
            generate_mesh(size)
    
    # Run the benchmark
    return run_benchmark(output_file)

def main():
    if not compile_benchmark():
        return

    # Test configurations
    test_configs = [
        ("linear", 1000),
        ("linear", 10000),
        ("linear", 100000),
        ("binary", 10),  # 2^10 - 1 nodes
        ("binary", 12),  # 2^12 - 1 nodes
        ("binary", 14),  # 2^14 - 1 nodes
        ("mesh", 32),    # 32x32 mesh
        ("mesh", 64),    # 64x64 mesh
        ("mesh", 128),   # 128x128 mesh
    ]

    results = []

    for test_type, size in test_configs:
        print(f"\nRunning {test_type} test with size {size}...")
        
        # Calculate actual number of nodes
        if test_type == "binary":
            actual_size = 2**size - 1
        elif test_type == "mesh":
            actual_size = size * size
        else:
            actual_size = size

        seq_time, par_time = generate_and_run_test(test_type, size, "temp_test.txt")
        
        if seq_time is not None and par_time is not None:
            speedup = seq_time / par_time if par_time > 0 else float('inf')
            results.append({
                "Test Type": test_type,
                "Size": actual_size,
                "Sequential (ms)": seq_time,
                "Parallel (ms)": par_time,
                "Speedup": f"{speedup:.2f}x"
            })

    # Clean up
    if os.path.exists("temp_test.txt"):
        os.remove("temp_test.txt")

    # Display results
    df = pd.DataFrame(results)
    print("\nBenchmark Results:")
    print(tabulate(df, headers='keys', tablefmt='grid', showindex=False))

if __name__ == "__main__":
    main() 