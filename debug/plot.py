import sys
import re
import matplotlib.pyplot as plt

# Helper to plot points spat out by the program (usually for visualising the output of rdp/ms)

def parse_multiple_sets(input_string):
    datasets = []

    # Find all groups inside square brackets
    group_matches = re.findall(r'\[(.*?)\]', input_string)

    for group in group_matches:
        points = []

        # Extract (x, y) pairs inside each group
        matches = re.findall(r'\(\s*([-+]?\d*\.?\d+)\s*,*\s*([-+]?\d*\.?\d+)\s*\)', group)

        for x_str, y_str in matches:
            x = float(x_str)
            y = float(y_str)
            points.append((x, y))

        if points:
            datasets.append(points)

    return datasets

def plot_datasets(datasets):
    if not datasets:
        print("No valid datasets found.")
        return

    for i, points in enumerate(datasets):
        x_vals = [p[0] for p in points]
        y_vals = [p[1] for p in points]

        plt.scatter(x_vals, y_vals, label=f"Set {i+1}")
        plt.plot(x_vals, y_vals)

    plt.xlabel("X")
    plt.ylabel("Y")
    plt.title("Plot of Multiple Point Sets")
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    # Example:
    # python script.py "[(1, 2), (3, 4)], [(5, 6), (7, 8)]"

    if len(sys.argv) < 2:
        print("Please provide the input string.")
        sys.exit(1)

    input_string = sys.argv[1]
    datasets = parse_multiple_sets(input_string)
    plot_datasets(datasets)
