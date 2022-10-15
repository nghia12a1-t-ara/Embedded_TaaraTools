import csv, os
from datetime import datetime
import matplotlib
import time
matplotlib.use("tkAgg")
import matplotlib.pyplot as plt
import numpy as np

#-------------------- FILE HANDLING -------------------#
myDir = input('Direction to the SVC file: ')
myFile = input('Name of SVC file: ')
os.chdir(str(myDir))

'''----------- Open and Read each row of csv file ----------'''
title_row = []  # Read the input csv file
with open(str(myFile) + '.csv', 'r') as myfile:
    csv_reader = csv.reader(myfile, delimiter = '\t')
    for row in csv_reader:
        title_row.append(row)

#------------------ PLOT DATA GRAPH -------------------#
plot_window = len(title_row) - 2
y_var = np.array(np.zeros([plot_window]))

plt.ion()
fig, ax = plt.subplots()    # Create a screen to display graph
line, = ax.plot(y_var)
plt.title("DATA LOGGER")
plt.xlabel("Time(s)")
plt.ylabel("Temperature(oC)")

for row in range(2, len(title_row)):
    # Get temperature from CSV file
    temperature = float(title_row[row][1].split(' ')[0])

    y_var = np.append(y_var, temperature)
    y_var = y_var[1:plot_window+1]
    line.set_ydata(y_var)
    ax.relim()
    ax.autoscale_view()
    fig.canvas.draw()
    fig.canvas.flush_events()
    time.sleep(0.05)

myPlot = input('Name of graph to save: ')
plt.savefig(str(myPlot) + '.png')

#------------------------------------------------------#
