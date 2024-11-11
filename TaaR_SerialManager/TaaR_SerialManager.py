### @Project: TaaR_SerialManager Python Class ###################################################
### @File	: TaaR_SerialManager.py
### 
### @Desp	: This file to implement PySerial Function to test COM Port Communication
### Author 	: Nghia Taarabt
### Created Time : 06/06/2023
### Link 	: https://github.com/nghia12a1-t-ara/Embedded_MyTools/tree/master/TaaR_SerialManager
#################################################################################################

import serial
import serial.threaded

''' Class for User - COM Port Manager '''
''' For User - from TaaR_SerialManager import SerialManager '''
class SerialManager:
    """ Class to manage the serial connection and use SerialReader """
    def __init__(self, port="COM1", baudrate=9600):
        self.Port = port
        self.Baudrate = baudrate
        self.Protocol = None
        self.Connection = None
        self.ComList = []
        self.thread = None

    """ Connect and DisConnect Functions to the COM Port """
    def Connect(self):
        try:
            self.Connection = serial.Serial(self.Port, self.Baudrate, timeout=1)
            return True
        except serial.SerialException as e:
            print(f"Failed to connect to {self.Port}: {e}")
            return False
    def DisConnect(self):
        if self.thread:
            self.thread.close()
        if self.Connection and self.Connection.is_open:
            self.Connection.close()

    """ Get all connected COM PORT """
    def GetAvailablePorts(self):
        Comlistinfo = serial.tools.list_ports.comports()
        for eachInfo in Comlistinfo:
            self.ComList.append(eachInfo.device)
        return self.ComList

    """ Start the ReaderThread to automatically read data from the serial port """
    def ReadAsync(self):
        self.thread = serial.threaded.ReaderThread(self.Connection, SerialReader)
        self.thread.start()
        self.Protocol = self.thread.protocol

    """ Wait to Read Byte Synchronous """
    def ReadSync(self):
        try:
            if serial.in_waiting > 0:  # Kiểm tra xem có dữ liệu sẵn sàng để đọc
                DataLine = serial.readline().decode('utf-8').rstrip()  # Đọc và giải mã dữ liệu
        except:
            DataLine = None
        return DataLine

    """ Set a callback function for handling received data """
    def SetRecvCallback(self, callback):
        if self.Protocol:
            self.Protocol.SetCallback(callback)

    """ Set a custom end byte for the SerialReader """
    def SetRecvEndByte(self, end_byte):
        if self.Protocol:
            self.Protocol.SetEndByte(end_byte)

    """ Send data through the serial port """
    def SendByte(self, data):
        if self.Connection and self.Connection.is_open:
            self.Connection.write(data.encode('utf-8'))

    """ Send Hex String data through the serial port """
    def SendHexString(self, StrData):
        if self.Connection and self.Connection.is_open:
            try:
                serdata = bytes.fromhex(StrData)
                self.Connection.write(serdata)
            except ValueError as e:
                print(f"Error converting hex string: {e}")
        else:
            print("Serial connection is not open")

    """ Stop the ReaderThread and close the serial connection """
    def Stop(self):
        if self.thread:
            self.thread.close()
            self.serial.close()

''' Class for Serial Thread Reader - Use for Receiver '''
class SerialReader(serial.threaded.Protocol):
    """ Class to read data from the serial port using the callback method """
    def __init__(self):
        super().__init__()
        self.RecvByteList = []
        self.EndByte = '\n'

    """ Called when the serial connection is established """
    def connection_made(self, transport):
        self.transport = transport

    """ Automatically called when new data is received from the serial port """
    def data_received(self, data):
        self.Serial_Recv_Int(data, self.EndByte)
    def Serial_Recv_Int(self, RecvData, endByte='\n'):
        byte_array = bytearray(RecvData)
        for byte in byte_array:
            self.RecvByteList.append(byte)
        if endByte in self.RecvByteList:      # Check if the buffer has a complete line
            ## Pause the receive --------------
            if self.callback:
                self.callback(self.RecvByteList)
            self.RecvByteList = []

    """ User Set the Parameter Functions """
    def SetEndByte(self, byte):
        self.EndByte = byte
    def SetCallback(self, callback):
        if callable(callback):          ### Callback function with param(RecvByteList)
            self.callback = callback
        else:
            raise ValueError("Provided callback is not a callable function.")

    """ Called when the serial connection is lost """
    def connection_lost(self, exc):
        if exc:
            print(f"Serial connection lost due to error: {exc}")
        else:
            print("Serial connection closed")
