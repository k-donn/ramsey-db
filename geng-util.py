import subprocess

def run_geng(command, output_file, mode = 'w', limit= None, start_idx=0):
    write_idx = start_idx
    with open(output_file, mode) as f:
        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
        if limit:
            for i, line in enumerate(proc.stdout):
                if i >= limit:
                    break
                f.write(str(write_idx) + "," + line.decode())
                write_idx += 1

        else:
            for i, line in enumerate(proc.stdout):
                f.write(str(write_idx) + "," + line.decode())
                write_idx += 1
    return write_idx

run_geng("geng 6", "6-graphs.csv")

run_geng("geng 9", "9-graphs.csv")

line_idx = 0
for x in range(92):
    line_idx = run_geng(f"geng 14 {x}:{x}", "14-graphs.csv", mode='a', limit=15000, start_idx=line_idx)

line_idx = 0
for x in range(154):
    line_idx = run_geng(f"geng 18 {x}:{x}", "18-graphs.csv", mode='a', limit=10000, start_idx=line_idx)