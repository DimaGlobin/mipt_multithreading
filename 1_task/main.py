import matplotlib
import matplotlib.pyplot as plt
matplotlib.use('Agg') 
import pandas as pd

def parse_file(filename):
    data = []
    with open(filename, 'r') as f:
        for line in f:
            parts = line.split(',')
            key = int(parts[0].split(':')[1].strip())
            time_str = parts[1].split(':')[1].strip().split('.')
            seconds = int(time_str[0])
            nanoseconds = int(time_str[1])
            duration = seconds + nanoseconds * 1e-9
            data.append([key, duration])
    return pd.DataFrame(data, columns=['Key', 'Duration'])

non_parallel_data = parse_file('build/non_parallel.txt')
parallel_data = parse_file('build/parallel.txt')

plt.figure(figsize=(10, 6))
plt.plot(non_parallel_data['Key'], non_parallel_data['Duration'], label='Non-parallel execution', color='blue')
plt.title('Execution Time vs Matrix Size (Non-parallel)')
plt.xlabel('Matrix Size')
plt.ylabel('Execution Time (seconds)')
plt.grid(True)
plt.legend()
plt.savefig('non_parallel_execution.png')
plt.show()

plt.figure(figsize=(10, 6))
plt.plot(parallel_data['Key'], parallel_data['Duration'], label='Parallel execution', color='red')
plt.title('Execution Time vs Number of Threads (Parallel)')
plt.xlabel('Number of Threads')
plt.ylabel('Execution Time (seconds)')
plt.grid(True)
plt.legend()
plt.savefig('parallel_execution.png')
plt.show()
