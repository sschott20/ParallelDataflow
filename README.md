# Liveness Checker

A C++ implementation of a liveness analysis tool that determines which variables are live at each program point in a control flow graph. The tool reads a control flow graph description and outputs which variables interfere with each other (are live simultaneously).

## Input Format

The input file describes a control flow graph where each node contains:

- Defined variables (def)
- Used variables (use)
- Successor nodes (succs)

The format is as follows:

```
N
def_size def1 def2 ...
use_size use1 use2 ...
succ_size succ1 succ2 ...
...
```

Where:

- `N` is the total number of nodes
- `def_size` is the number of variables defined in this node
- `use_size` is the number of variables used in this node
- `succ_size` is the number of successor nodes

## Test Cases

### Test 1: Linear Flow

File: `test1.txt`

```
3
1 a
1 b
1 1
1 b
1 c
1 2
1 c
1 d
0
```

This represents a simple linear flow:

- Node 0: Defines 'a', uses 'b', goes to node 1
- Node 1: Defines 'b', uses 'c', goes to node 2
- Node 2: Defines 'c', uses 'd', no successors

Expected output:

```
d: b c d
c: b c d
b: b c d
```

This shows that variables 'b', 'c', and 'd' all interfere with each other as they are live across multiple nodes.

### Test 2: Branching Flow

File: `test2.txt`

```
4
1 x
1 a
2 1 2
1 y
1 x
0
1 z
1 x
0
0
0
0
```

This represents a branching control flow:

- Node 0: Defines 'x', uses 'a', branches to nodes 1 and 2
- Node 1: Defines 'y', uses 'x', no successors
- Node 2: Defines 'z', uses 'x', no successors
- Node 3: Empty node (no defs/uses/successors)

Expected output:

```
x: x
a: a
```

This shows that 'x' interferes with itself (used in both branches) and 'a' only interferes with itself.

### Test 3: Loop Structure

File: `test3.txt`

```
3
1 i
1 n
1 1
2 i j
1 i
2 1 2
1 result
2 i j
0
```

This represents a loop structure:

- Node 0: Defines 'i', uses 'n', goes to node 1 (loop initialization)
- Node 1: Defines 'i' and 'j', uses 'i', can go to nodes 1 or 2 (loop body)
- Node 2: Defines 'result', uses 'i' and 'j' (loop exit)

Expected output:

```
j: i j
i: i j
n: n
```

This shows that 'i' and 'j' interfere with each other (both live in the loop) while 'n' is separate (only used in initialization).

## Test Generation

The repository includes a Python script `generate_tests.py` that can generate various types of test cases for benchmarking and testing purposes. The script supports three types of test cases:

1. **Linear Chain** (`linear`): Generates a linear sequence of nodes where each node defines a variable and uses the previous node's variable.

   ```bash
   python generate_tests.py linear <size>
   ```

2. **Binary Tree** (`binary`): Generates a complete binary tree where each node defines a variable and uses its parent's variable.

   ```bash
   python generate_tests.py binary <size>
   ```

3. **Mesh** (`mesh`): Generates an n×n mesh where each node connects to its right and down neighbors.
   ```bash
   python generate_tests.py mesh <size>
   ```

The generated test cases follow the same input format as described above and can be used to test the liveness checker's performance on different graph structures and sizes.

## Building and Running

To compile and run the liveness checker:

```bash
g++ -std=c++11 sequential/checker.cpp -o liveness_checker
./liveness_checker <test_file>
```

## Output Format

The output shows which variables interfere with each other. Each line shows a variable and all variables it interferes with. For example:

```
j: i j
i: i j
n: n
```

This means:

- 'j' interferes with both 'i' and itself
- 'i' interferes with both 'j' and itself
- 'n' only interferes with itself

## Implementation Details

The liveness checker uses a worklist algorithm to propagate liveness information through the control flow graph. It:

1. Computes the set of live variables at each program point
2. Propagates this information through the control flow graph
3. Identifies which variables are live simultaneously
4. Outputs the interference relationships between variables

The sequential implementation uses a standard worklist algorithm where:

- A queue maintains nodes that need to be processed
- For each node, it computes the OUT set by combining IN sets of successors
- Then computes the IN set using the OUT set and the node's USE/DEF sets
- If the IN set changes, predecessors are added to the worklist
- This continues until no more changes occur

The parallel implementation uses a graph coloring approach to identify interfering variables, which can be more efficient for large control flow graphs.
