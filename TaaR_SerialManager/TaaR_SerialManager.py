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
import serial.tools.list_ports
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

class SerialManager:
    """
    Class to manage serial communication, including connecting, disconnecting,
    sending, and receiving data asynchronously or synchronously.
    """

    def __init__(self, port="COM1", baudrate=9600):
        self.port = port
        self.baudrate = baudrate
        self.protocol = None
        self.connection = None
        self.thread = None

    def connect(self):
        """Establish a serial connection."""
        try:
            self.connection = serial.Serial(self.port, self.baudrate, timeout=1)
            logging.info(f"Connected to {self.port} at {self.baudrate} baud.")
            return True
        except serial.SerialException as e:
            logging.error(f"Failed to connect to {self.port}: {e}")
            return False

    def disconnect(self):
        """Close the serial connection and stop the reader thread if active."""
        if self.thread:
            self.thread.close()
            logging.info("Reader thread stopped.")

        if self.connection and self.connection.is_open:
            self.connection.close()
            logging.info(f"Disconnected from {self.port}.")

    def get_available_ports(self):
        """Return a list of available serial ports."""
        return [port.device for port in serial.tools.list_ports.comports()]

    def read_async(self, byte_num=1, end_byte=None, callback=None):
        """
        Start a ReaderThread to read data asynchronously from the serial port.

        Args:
            byte_num (int): Number of bytes to wait for.
            end_byte (int): End byte indicating the end of a message.
            callback (function): Callback function to handle received data.
        """
        if not self.connection or not self.connection.is_open:
            logging.error("Connection is not open. Cannot start async reading.")
            return

        self.thread = serial.threaded.ReaderThread(self.connection, SerialReader)
        self.thread.start()
        self.protocol = self.thread.protocol
        self.protocol.set_callback(callback)
        self.protocol.set_end_byte(end_byte)
        self.protocol.set_byte_num(byte_num)
        logging.info("Asynchronous reading started.")

    def read_sync(self):
        """Read data synchronously from the serial port."""
        if not self.connection or not self.connection.is_open:
            logging.error("Connection is not open. Cannot read data.")
            return None

        try:
            if self.connection.in_waiting > 0:
                data_line = self.connection.readline().decode('utf-8').rstrip()
                return data_line
        except Exception as e:
            logging.error(f"Error reading data synchronously: {e}")
        return None

    def send_byte(self, data):
        """Send data as bytes through the serial port."""
        if self.connection and self.connection.is_open:
            self.connection.write(data.encode('utf-8'))
            logging.info(f"Sent data: {data}")
        else:
            logging.error("Connection is not open. Cannot send data.")

    def send_hex_string(self, hex_string):
        """Send data as a hexadecimal string through the serial port."""
        if self.connection and self.connection.is_open:
            try:
                data = bytes.fromhex(hex_string)
                self.connection.write(data)
                logging.info(f"Sent hex string: {hex_string}")
            except ValueError as e:
                logging.error(f"Error converting hex string: {e}")
        else:
            logging.error("Connection is not open. Cannot send data.")

    def stop(self):
        """Stop the ReaderThread and close the serial connection."""
        self.disconnect()


class SerialReader(serial.threaded.Protocol):
    """
    Class to handle data received from the serial port using a callback mechanism.
    """

    def __init__(self):
        super().__init__()
        self.recv_byte_list = []
        self.end_byte = None
        self.callback = None
        self.byte_num = 1

    def connection_made(self, transport):
        """Called when the serial connection is established."""
        self.transport = transport
        logging.info("Serial connection established.")

    def data_received(self, data):
        """Called automatically when new data is received from the serial port."""
        self._process_received_data(data, self.byte_num, self.end_byte)

    def _process_received_data(self, recv_data, byte_num=1, end_byte=None):
        byte_array = bytearray(recv_data)
        self.recv_byte_list.extend(byte_array)

        if (end_byte is not None and end_byte in self.recv_byte_list) or \
           (len(self.recv_byte_list) >= byte_num):
            if self.callback:
                self.callback(self.recv_byte_list)
            self.recv_byte_list.clear()

    def set_end_byte(self, byte):
        """Set the custom end byte."""
        self.end_byte = byte

    def set_byte_num(self, byte_num):
        """Set the number of bytes to wait for before processing."""
        self.byte_num = byte_num

    def set_callback(self, callback):
        """Set the callback function for handling received data."""
        if callable(callback):
            self.callback = callback
        else:
            raise ValueError("Provided callback is not callable.")

    def connection_lost(self, exc):
        """Called when the serial connection is lost."""
        if exc:
            logging.error(f"Serial connection lost due to error: {exc}")
        else:
            logging.info("Serial connection closed.")
