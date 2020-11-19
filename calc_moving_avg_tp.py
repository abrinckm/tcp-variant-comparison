import sys
import csv
import decimal
from collections import deque

prefix = sys.argv[1]
prob = sys.argv[2]

window_size = 0.9 #seconds
_lambda = 0.1 #seconds

def calc_avg_bps(window, time):
    cwnd_total = 0
    size = len(window)
    for row in window:
        cwnd_total += (row[1] * 8 / ((1 << 20) * time))
    return cwnd_total

with open("./data/" + prob + "/" + prefix + "-inflight.data", "r") as cwnd_data, \
     open("./data/" + prob + "/" + prefix + "-mov-avg.data", "w") as mov_avg_data:
    reader = csv.reader(cwnd_data, delimiter=' ')
    window_pos = 0.0
    last_window_pos = 0.0
    delta = 0.0
    total_window = []
    total_seconds = 0.0
    window = deque()
    for row in reader:
        delta = float(row[0])
        cwnd = float(row[1])

        window.append((delta, cwnd))

        if (delta - window_pos >= window_size and window_pos == 0.0) or window_pos - last_window_pos >= _lambda:
            mov_avg_data.write(str(delta) + " " + str(calc_avg_bps(window, delta - window_pos)) + "\n")
            last_window_pos = window.popleft()[0]
            window_pos = last_window_pos
        elif delta - window_pos >= window_size:
            while delta - window_pos >= window_size:
                window_pos = window.popleft()[0]