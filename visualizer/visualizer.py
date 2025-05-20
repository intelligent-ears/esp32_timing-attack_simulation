import re
import matplotlib.pyplot as plt
import pandas as pd

LOG_FILE = 'sample_output.txt'  

def parse_logs(filename):
    with open(LOG_FILE, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    all_data = []
    position = -1
    char_times = []

    for line in lines:
        line = line.strip()

        # Detect character set header
        if "Finding character at position" in line:
            if char_times:
                all_data.append((position, pd.DataFrame(char_times)))
                char_times = []

            match = re.search(r'position (\d+)', line)
            position = int(match.group(1))

        # Extract timing data for each character
        elif re.match(r"^\s*[a-z0-9]\s+\|", line):
            parts = re.split(r'\s+\|\s+', line.strip())
            if len(parts) >= 6:
                char = parts[0].strip()
                avg_time = int(parts[1].strip())
                min_time = int(parts[2].strip())
                max_time = int(parts[3].strip())
                std_dev = float(parts[4].strip())
                samples = int(parts[5].strip())

                char_times.append({
                    'char': char,
                    'avg_time': avg_time,
                    'min': min_time,
                    'max': max_time,
                    'std_dev': std_dev,
                    'samples': samples
                })

    # Append final collected block
    if char_times:
        all_data.append((position, pd.DataFrame(char_times)))

    return all_data

def plot_timing(all_data):
    for position, df in all_data:
        df = df.sort_values(by='avg_time', ascending=False)

        plt.figure(figsize=(10, 5))
        bars = plt.bar(df['char'], df['avg_time'], color='skyblue')
        plt.title(f"Timing Analysis at Position {position}")
        plt.xlabel("Character")
        plt.ylabel("Average Response Time (μs)")

        # Highlight best guess in red
        bars[0].set_color('red')
        best_char = df.iloc[0]['char']
        best_time = df.iloc[0]['avg_time']

        plt.axhline(best_time, color='red', linestyle='--', label=f"Best Guess: '{best_char}'")
        plt.legend()
        plt.tight_layout()
        plt.show()

if __name__ == "__main__":
    data = parse_logs(LOG_FILE)
    plot_timing(data)
