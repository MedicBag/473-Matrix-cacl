import random
# simple matrix generator
def generate_matrix_file(filename, rows, cols):
    with open(filename, 'w') as f:
        # Write dimensions first
        f.write(f"{rows} {cols}\n")
        # Write random data
        for _ in range(rows):
            row_data = [str(random.randint(0, 9)) for _ in range(cols)]
            f.write(" ".join(row_data) + "\n")
    print(f"Created {filename} ({rows}x{cols})")

# Generate the 100, 200, 100 set
generate_matrix_file("A.txt", 100, 200)
generate_matrix_file("B.txt", 200, 100)