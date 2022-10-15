# WaveShapePlay
# Find a detailed youtube tutorial for the Arduino Com Connection Code at: https://youtu.be/DJD28uK5qIk

import serial.tools.list_ports
from time import sleep
import re, os, csv
from datetime import datetime

###################### DEFINE FUNCTIONS ######################
'''------------------- DEFINE FUNCTIONS -------------------'''

'''--------- List all Device connect with COM Port --------'''
def listDevice():
    list_ports = serial.tools.list_ports.comports()
    print(list_ports)
    return list_ports

'''-------------- Get data from Arduino CH340 -------------'''
def ArduinoPort(lst):
    commPort = 'None'
    numConnection = len(lst)
    #print(numConnection)
    for i in range(0,numConnection):
        port = lst[i]
        strPort = str(port)

        if 'CH340' in strPort:
            splitPort = strPort.split(' ')
            commPort = (splitPort[0])
    return commPort

'''-------------- Get data from STM32-UART ---------------'''
def USBtoCOMPort(lst):
    commPort = 'None'
    numConnection = len(lst)
    #print(numConnection)
    for i in range(0,numConnection):
        port = lst[i]
        strPort = str(port)

        if 'CP210x' in strPort:
            splitPort = strPort.split(' ')
            commPort = (splitPort[0])
    return commPort

'''-------------------- Request sensor's data ------------------'''
def getValues(ser):
    ser.write(b'g')
    myData = ser.readline().decode().split('\r\n')
    return myData[0]

def printToFile(file, data, index):
    file.write(data)
    if index!= (numPoints - 1):
        file.write(',')
    else:
        file.write('\n')

def getAverage(dataSet, row):
    dataAvg = sum(dataSet) / len(dataSet)
    print('Temperature of ten values ' + str(row) + ' times is: ' + str(dataAvg))
    return dataAvg


######################### MY MAIN FUNCTION #########################
'''---------------------- MY MAIN FUNCTION ----------------------'''

lst = listDevice()
print(lst)
connectPort = ArduinoPort(lst)
print(connectPort)
while connectPort == 'None':
    ()

#---------------- Enter Baudrate and Connect -----------------#
mybaud = input("Enter Baudrate: ")
ser1 = serial.Serial(connectPort, baudrate = mybaud, timeout = 1)
print('Connected to ' + connectPort)
print('DONE - Start Read:')

<<<<<<< HEAD
sleep(1.2)
=======
sleep(1.3)
>>>>>>> py2

#------------------- Text File PARAMETERS --------------------#
numPoints = 10
numRowsCollect = 50
dataList = [0]*numPoints

#----------------- FILE HANDLING INITIALIZE ------------------#
mydir = input('Direction to save data file: ')
os.chdir(str(mydir))
csvFile = input('Name of csv file to save data: ')
myFile = input('Name of text file to save data: ')
dataFile = open((str(myFile) +  '.txt'), 'w')

with open((str(csvFile) + '.csv'), 'w') as outFile:
    writer = csv.writer(outFile, delimiter='\t', quotechar='"', quoting=csv.QUOTE_MINIMAL)
    writer.writerow(['Time'.center(17), 'Temperature'])

########################## SUPER LOOP ##########################
'''----------------------- SUPER LOOP -----------------------'''

while 1:
    #--------------- Request data from sensor --------------#
    userInput = input('Get data point?')
    if userInput == 'y':
        # Collect many rows of datas
        for row in range(0, numRowsCollect):
            for i in range(0, numPoints):
                data = getValues(ser1)
                #dataFile.write(data + ' ')
                printToFile(dataFile, data, i)
                dataInt = int(data)
                dataList[i] = dataInt

            # Take average of each row
            dataAvg = getAverage(dataList, row)
            '''------------------- Append to output csv file 50 values -------------------'''
            now = datetime.now()
            datetime_str = now.strftime("%d/%m/%y %H:%M:%S")
            with open((str(csvFile) + '.csv'), 'a', newline = '') as outFile:
                writer = csv.writer(outFile, delimiter='\t', quotechar='"', quoting=csv.QUOTE_MINIMAL)
                writer.writerow([datetime_str, str(dataAvg) + ' oC'])

    elif userInput == 'n':      # Out to collecting
        dataFile.close()
        break

    #---------------- Read Data Continuously ---------------#
    '''
    arduinoData = ser1.readline()
    #    byte to string
    #    b'465\r\n' --> '465'   
    

    ziyech = re.compile(r"b'.{1,4}\\r")
    data = ziyech.search(str(arduinoData))
    dataStr = data.group()
    dataStr = dataStr[0:len(dataStr) - 2]
    dataStr = dataStr[2:]
    print('Sensor Value = ' + dataStr)

    sleep(0.1)
    '''
