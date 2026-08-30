import subprocess
from pathlib import Path
import json

current_file_path = str(Path(__file__).resolve().parent)
p = subprocess.Popen([current_file_path+"/../build/citterfish"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
assert p.stdin is not None
assert p.stdout is not None

with open (current_file_path+"/../data/perft_data.json", "r") as file:
    data = json.load(file) 
    for i, group in enumerate(data):
        p.stdin.write("position fen "+group["fen"]+"\n")
        p.stdin.flush()
        p.stdin.write("go depth "+str(group["depth"])+"\n")
        p.stdin.flush()
        output = p.stdout.readline()
        while(not "Moves Generated: " in output):
            output = p.stdout.readline()
        res = int(output.strip().split(" ")[2])
        if (res != group["result"]):
            print(f"FEN: {group["fen"]} failed, expected {group["result"]} got {res}")
        else:
            print(f"FEN: {group["fen"]} succeeded, {res}")