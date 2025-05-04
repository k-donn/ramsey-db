import subprocess

commands = [
    ["./gen-rMN", "3", "3", "<", "6-graphs.csv"],
    ["./gen-rMN", "3", "4", "<", "9-graphs.csv"],
    ["./gen-rMN", "3", "5", "<", "14-graphs.csv"],
    ["./gen-rMN", "4", "4", "<", "18-graphs.csv"],
    ["./gen-rMN", "3", "6", "<", "18-graphs.csv"]
]

for cmd in commands:
    # Join the command into a single string to handle input redirection
    cmd_str = " ".join(cmd)
    print(f"Running: {cmd_str}")
    subprocess.run(cmd_str, shell=True, check=True)