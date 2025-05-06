import sys

def generate_linear_chain(n):
    """Generate a linear chain of n nodes"""
    print(n)
    for i in range(n):
        # Each node defines a variable and uses the previous one
        print(f"1 var_{i}")  # def
        if i > 0:
            print(f"1 var_{i-1}")  # use
        else:
            print("0")  # no use in first node
        if i < n-1:
            print(f"1 {i+1}")  # succ
        else:
            print("0")  # no succ in last node

def generate_binary_tree(n):
    """Generate a complete binary tree with n levels"""
    total_nodes = 2**n - 1
    print(total_nodes)
    for i in range(total_nodes):
        print(f"1 var_{i}")  # def
        if i > 0:
            parent = (i-1) // 2
            print(f"1 var_{parent}")  # use parent's variable
        else:
            print("0")  # no use in root
        if 2*i + 1 < total_nodes:
            print(f"2 {2*i+1} {2*i+2}")  # left and right children
        else:
            print("0")  # no children for leaves

def generate_mesh(n):
    """Generate an n x n mesh where each node connects to right and down neighbors"""
    total_nodes = n * n
    print(total_nodes)
    for i in range(total_nodes):
        row = i // n
        col = i % n
        print(f"1 var_{i}")  # def
        if row > 0:
            print(f"1 var_{i-n}")  # use from above
        elif col > 0:
            print(f"1 var_{i-1}")  # use from left
        else:
            print("0")  # no use in top-left corner
        
        succs = []
        if col < n-1:
            succs.append(str(i+1))  # right neighbor
        if row < n-1:
            succs.append(str(i+n))  # down neighbor
        if succs:
            print(f"{len(succs)} {' '.join(succs)}")
        else:
            print("0")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python generate_tests.py <type> <size>")
        print("Types: linear, binary, mesh")
        sys.exit(1)
    
    test_type = sys.argv[1]
    size = int(sys.argv[2])
    
    if test_type == "linear":
        generate_linear_chain(size)
    elif test_type == "binary":
        generate_binary_tree(size)
    elif test_type == "mesh":
        generate_mesh(size)
    else:
        print("Invalid test type. Use: linear, binary, mesh")
        sys.exit(1) 