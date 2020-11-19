import sys
import csv
import decimal
import numpy as np
from collections import deque

drop_p=["0.000001", "0.00001", "0.0001", "0.001", "0.01"]
variants=["TcpBic", "TcpHybla", "TcpHtcp", "TcpHighSpeed", "TcpNewReno"]

for prefix in variants:
    with open("./data/" + prefix + "-resp.data", "w") as resp_data:
        for prob in drop_p:
            e = abs(decimal.Decimal(prob).as_tuple().exponent) - 1
            with open("./data/" + prob + "/" + prefix + "-mov-avg.data", "r") as avg_data:
                data = list(map(lambda tup: float(tup[1]), list(csv.reader(avg_data, delimiter=" "))))
                avg_bps = np.average(data)
                resp_data.write(str(e) + " " + str(avg_bps) + "\n")