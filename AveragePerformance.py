import numpy as np
import math
import csv


seedNumber = [num for num in range(100)]
nUEs = [num for num in range(10000, 110000, 10000)]
averagePerformance = [[0, 0, 0, 0, 0, 0] for num in range(10)]
i = 0
for n in range(len(nUEs)):
    for s in seedNumber:
        folder = "./Beta_SimulationResults/{}_54_{}_Results.txt".format(s, nUEs[n])
        f = open(folder, 'r')
        data = []
        for line in f.readlines():
            data.append(float(line.strip()))
        f.close()
        for i in range(len(averagePerformance[n])):
            averagePerformance[n][i] += data[i]
    
npAverage = np.array(averagePerformance)
with open('results.csv', 'w') as file:
    write = csv.writer(file)
    for lists in npAverage:
        write.writerow(np.around(lists/len(seedNumber), 3))