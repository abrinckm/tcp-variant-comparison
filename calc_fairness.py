import sys
import csv
import numpy as np

delays = ["100ms"]
drop_p = ["0.00001"]  #, "0.001"]
prefixes = ["Bic", "Hybla", "Htcp", "HighSpeed"]

#  fA(x) = f(x) = [(sum xi)**2]/{n*sum[(xi)**2)]} = 0.1
with open("./data/f_c/index.data", "w") as index_data:
    index_data.write("variants " + " ".join(prefixes) + "\n")
    for delay in delays:
        for prob in drop_p:
            index_data.write(delay + "-" + prob + " ")
            for i in range(len(prefixes)):
                with open("./data/f_c/" + delay + "/" + prob + "/Tcp" + prefixes[i] + "-mov-avg.data", "r") as avg_data, \
                    open("./data/f_c/" + delay + "/" + prob + "/TcpNewReno-" + prefixes[i] + "-mov-avg.data", "r") as reno_avg_data:
                    variant_avg = list(map(lambda tup: float(tup[1]), list(csv.reader(avg_data, delimiter=" "))))
                    reno_avg = list(map(lambda tup: float(tup[1]), list(csv.reader(reno_avg_data, delimiter=" "))))
                    variant_avg.sort()
                    variant_avg = np.array(variant_avg)
                    variant_long = (np.quantile(variant_avg, 0.75) +np.quantile(variant_avg, 0.25)) / 2
                    # variant_long = np.average(variant_avg)
                    reno_avg.sort()
                    reno_avg = np.array(reno_avg)
                    reno_long = (np.quantile(reno_avg, 0.75) +np.quantile(reno_avg, 0.25)) /2
                    # reno_long = np.average(reno_avg)
                    numerator = (reno_long + variant_long) * (reno_long + variant_long)
                    denom = 2 * (reno_long * reno_long + variant_long * variant_long)
                    index_data.write(str(numerator / float(denom)) + " ")
            index_data.write("\n")