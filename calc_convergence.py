import sys
import csv
import numpy as np

delays = ["100ms"]
drop_p = ["0.00001"]
prefixes = ["Bic", "Hybla", "Htcp", "HighSpeed"]

def calc_long_term(avg_data):
    c = []
    start_span = 0.0
    end_span = 0.0
    previous = 0.0
    dist = 0.0
    for row in avg_data:
        _delta = row[0]
        avg = row[1]
        if previous == 0:
            previous == avg
        if avg > previous:
            dist = previous / avg
        elif previous != 0.0:
            dist = avg / previous
        else:
            dist = 0.0
        if dist >= 0.80:
            end_span = _delta
        else:
            c.append((end_span-start_span, previous, start_span))
            start_span = _delta
            end_span = start_span
            previous = avg

    dtype = [('span', float), ('avg', float), ('delta', float)]
    a = np.array(c, dtype=dtype)

    return np.sort(a, order='span')

def calc_converg(long_term_data):
    closest = []
    long_term = long_term_data[-1][1]
    for row in long_term_data:
        avg = row[1]
        if avg > long_term:
            tmp = long_term
            long_term = avg
            avg = tmp
        if avg / long_term >= 0.92:
            closest.append(row[2])

    closest.sort()
    return closest[0]

with open("./data/f_c/converg.data", "w") as index_data:
    index_data.write("converg variant reno\n")
    for delay in delays:
        for prob in drop_p:
            for prefix in prefixes:
                index_data.write(prefix + "-new-reno ")
                with open("./data/f_c/" + delay + "/" + prob + "/Tcp" + prefix + "-mov-avg.data", "r") as avg_data, \
                    open("./data/f_c/" + delay + "/" + prob + "/TcpNewReno-" + prefix + "-mov-avg.data", "r") as reno_avg_data:
                    variant_data = list(map(lambda tup: (float(tup[0]), float(tup[1])), list(csv.reader(avg_data, delimiter=" "))))
                    reno_data = list(map(lambda tup: (float(tup[0]), float(tup[1])), list(csv.reader(reno_avg_data, delimiter=" "))))
                    variant_long_term = calc_long_term(variant_data)
                    # index_data.write(str(calc_converg(variant_long_term)) + " ")
                    index_data.write(str(variant_long_term[-1][2]) + " ")
                    reno_long_term = calc_long_term(reno_data)
                    # print (reno_long_term)
                    index_data.write(str(reno_long_term[-1][2]) + " \n")
