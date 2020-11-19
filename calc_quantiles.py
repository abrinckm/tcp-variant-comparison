import sys
import csv
import numpy as np

prefixes = ["TcpBic", "TcpHighSpeed", "TcpHtcp", "TcpHybla", "TcpNewReno"]
probs = ["0.00001", "0.0001", "0.001"]

for prob in probs:
    with open("./data/" + prob + "/quantiles.data", "w") as quantiles_data:
        for i in range(len(prefixes)):
            # least 25th 50th 75th greatest
            with open("./data/" + prob + "/" + prefixes[i] + "-mov-avg.data", "r") as avg_data:
                data = list(map(lambda tup: float(tup[1]), list(csv.reader(avg_data, delimiter=" "))))
                data.sort()
                data = np.array(data)
                quantiles_data.write(str(i + 1) + " " +
                                    str(np.quantile(data, 0.10)) + " " +
                                    str(np.quantile(data, 0.25)) + " " +
                                    str(np.quantile(data, 0.5)) + " " +
                                    str(np.quantile(data, 0.75)) + " " +
                                    str(np.quantile(data, 0.95)) + "\n")