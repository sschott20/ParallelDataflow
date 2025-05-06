import subprocess
import sys
import time
import random
import re
import os

VARIABLES = [i for i in "abcdefghijkl"]


def generate_input_morevar(n, k):
    nodes = [f"{n}"]
    for i in range(n):
        d = []
        u = []
        s = []

        d.append(random.choice(VARIABLES) + str(int(i / k)))

        for j in range(random.randint(1, 3)):
            u.append(random.choice(VARIABLES) + str(int(i / k)))

        for j in range(random.randint(1, 2)):
            x = int(random.normalvariate(i, 5))
            if x != i and x > 0 and x < n - 1:
                s.append(x)
        d = sorted(list(set(d)))
        u = sorted(list(set(u)))
        s = sorted(list(set(s)))

        nodes.append(f"{len(d)} {' '.join(d)}")
        nodes.append(f"{len(u)} {' '.join(u)}")
        nodes.append(f"{len(s)} {' '.join(map(str, s))}")

    return "\n".join(nodes) + "\n"


def generate_input(n):
    nodes = [f"{n}"]
    for i in range(n):
        define = []
        use = []
        jump = []

        define.append(random.choice(VARIABLES))

        for i in range(random.randint(1, 6)):
            use.append(random.choice(VARIABLES))

        for i in range(random.randint(1, 4)):
            x = random.randint(1, n)
            if x != i:
                jump.append(random.randint(1, n - 1))
        define = sorted(list(set(define)))
        use = sorted(list(set(use)))
        jump = sorted(list(set(jump)))

        nodes.append(f"{len(define)} {' '.join(define)}")
        nodes.append(f"{len(use)} {' '.join(use)}")
        nodes.append(f"{len(jump)} {' '.join(map(str, jump))}")

    return "\n".join(nodes) + "\n"


TEST_CASES = [
    {
        "name": "Test 1: Simple 3 node cfg",
        "input": "10\n0\n0\n0\n1 e\n0\n1 2\n0\n1 x\n2 3 4\n1 z\n1 e\n1 5\n0\n1 x\n0\n1 y\n2 e x \n1 6\n1 x\n1 x\n1 7\n0\n1 x\n2 8 9\n1 e\n1 z\n1 2\n1 e\n1 y\n1 2\n",
        "timeout": 5,
    },
]

TEST_CASES += [
    {
        "name": "Small Generated test 2: 4 nodes",
        "input": generate_input(3),
        "timeout": 30,
    },
    {
        "name": "Generated test: 10^3 nodes, k = 10",
        "input": generate_input_morevar(10**3, 10),
        "timeout": 30,
    },
    {
        "name": "Generated test: 10^4 nodes, k = 100",
        "input": generate_input_morevar(10**4, 100),
        "timeout": 60,
    },
    {
        "name": "Generated test: 10^5 nodes, k = 100",
        "input": generate_input_morevar(10**5, 100),
        "timeout": 120,
    },
    {
        "name": "Generated test: 10^6 nodes, k = 100",
        "input": generate_input_morevar(10**6, 100),
        "timeout": 120,
    },
    {
        "name": "Generated test: 10^6 nodes, k = 1000",
        "input": generate_input_morevar(10**6, 1000),
        "timeout": 120,
    },

]


def compile_cpp_source():
    """
    Attempt to compile 'liveness.cpp' into an executable 'liveness'.
    Return True if successful, False otherwise.
    """
    if not os.path.exists("liveness.cpp"):
        print("[Error] template.cpp not found.")
        return False

    compile_cmd = [
        "g++",
        "-O3",
        "-std=c++17",
        "-pthread",
        "-fopenmp",
        "-o",
        "liveness",
        "liveness.cpp",
    ]
    try:
        result = subprocess.run(compile_cmd, capture_output=True, text=True, check=True)
        print("[Info] Compilation succeeded.")
        return True
    except subprocess.CalledProcessError as e:
        print("[Error] Compilation failed.")
        print("stdout:", e.stdout)
        print("stderr:", e.stderr)
        return False


def write_file(filename, content):
    """Write the provided content to a file."""
    with open(filename, "w") as f:
        f.write(content)


def compute_expected_output(test_case):
    """
    Given data and query strings plus k, compute the expected output lines.
    Example logic that prints all queries, each with its k nearest neighbors.
    Adjust to match your program's logic as needed.
    """
    out_lines = []
    write_file("input.txt", test_case["input"])
    cmd = ["./checker", "input.txt"]
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True)
    except subprocess.TimeoutExpired:
        return ["[TIMEOUT]"]
    out_lines = proc.stdout.strip().splitlines()
    out_lines = [ln.strip() for ln in out_lines if ln.strip()]
    return out_lines


def run_parallel_program(test_case):
    """
    Run the compiled kd_tree program on the given test case.
    Return the list of stripped, non-empty output lines or ["[TIMEOUT]"] if it times out.
    """
    write_file("input.txt", test_case["input"])
    cmd = ["./liveness", "input.txt"]
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True)
    except subprocess.TimeoutExpired:
        return ["[TIMEOUT]"]

    # Clean output lines
    out_lines = proc.stdout.strip().splitlines()
    out_lines = [ln.strip() for ln in out_lines if ln.strip()]
    return out_lines


def run_tests():
    """
    Compile once, then run all tests, printing results to stdout.
    Exit code 0 if all pass, 1 if any fail.
    """
    # 1. Compile
    if not compile_cpp_source():
        sys.exit(1)

    # 2. Run tests
    all_passed = True
    for i, test in enumerate(TEST_CASES, start=1):
        print(f"\n=== Running Test {i}: {test['name']} ===")
        start = time.time()

        actual_lines = run_parallel_program(test)
        print("Parallel time: ", time.time() - start)
        start2 = time.time()
        expected_lines = compute_expected_output(test)
        print("Seq time: ", time.time() - start2)

        duration = time.time() - start 
        if actual_lines == ["[TIMEOUT]"]:
            print("[FAIL] Program timed out.")
            all_passed = False
            continue
        # sort actual lines and expected lines
        actual_lines.sort()
        expected_lines.sort()

        # print(actual_lines)
        # print(expected_lines)

        # Compare line counts
        if len(actual_lines) != len(expected_lines):
            print(
                f"[FAIL] Expected {len(expected_lines)} lines, got {len(actual_lines)}."
            )
            all_passed = False
        else:
            for i in range(len(expected_lines)):
                if actual_lines[i] != expected_lines[i]:
                    print(f"[FAIL] Line {i + 1} does not match.")
                    all_passed = False
                    break

        print(f"[Info] Test finished in {duration:.2f}s")

    if all_passed:
        print("\nAll tests passed.")
        sys.exit(0)
    else:
        print("\nSome tests failed.")
        sys.exit(1)


if __name__ == "__main__":
    run_tests()
