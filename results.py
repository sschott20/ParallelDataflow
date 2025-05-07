import matplotlib.pyplot as plt

# Data from the test results
test_cases = [
    "4 nodes",
    "10^3 nodes",
    "10^4 nodes",
    "10^5 nodes",
    "10^6 nodes (k=100)",
    "10^6 nodes (k=1000)",
]

parallel_times = [
    0.015519142150878906,
    0.28262805938720703,
    0.21445655822753906,
    2.4128098487854004,
    23.41976261138916,
    14.871882200241089
]

seq_times = [
    0.003223896026611328,
    0.1785275936126709,
    0.22353363037109375,
    2.6026220321655273,
    30.22650694847107,
    20.189003944396973
]

# Create the bar graph
x = range(len(test_cases))  # X-axis positions
width = 0.35  # Width of the bars

plt.bar(x, parallel_times, width, label='Parallel Time', color='blue')
plt.bar([p + width for p in x], seq_times, width, label='Sequential Time', color='orange')

# Add labels, title, and legend
plt.xlabel('Test Cases')
plt.ylabel('Time (seconds)')
plt.title('Comparison of Parallel and Sequential Times')
plt.xticks([p + width / 2 for p in x], test_cases, rotation=45, ha='right')
plt.legend()

# Show the plot
plt.tight_layout()
plt.savefig("results/parallel_vs_sequential.png")
# plt.show()