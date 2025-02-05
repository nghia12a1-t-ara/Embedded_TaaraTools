### @Project: TaaR_SerialManager Test File ###################################################
### @File	: SerialTest.py
### 
### @Desp	: This file to Use the TaaR_SerialManager Class and Test communicate with PC (Arduino)
### Author 	: Nghia Taarabt
### Created Time : 06/06/2023
### Link 	: https://github.com/nghia12a1-t-ara/Embedded_MyTools/tree/master/TaaR_SerialManager
#################################################################################################

'''
    Connect Arduino <-> PC (Open this tool)
    Arduino Code:

    void setup()
    {
      Serial.begin(9600);
    }
    void loop()
    {
      // send data only when you receive data
      if (Serial.available() > 0)
      {
        Serial.write(Serial.read());
      }
    }
'''

from TaaR_SerialManager import SerialManager
import time

def UART_RecvCallback(buffer):
    # Add your code
    print("The receive character from Arduino = ", buffer)

def main():
    UART = SerialManager(port="COM6", baudrate=115200)
    if UART.connect():
        print("Connect is done")
    else:
        print("Connect is fail")
        return 0
    
    # UART.SetRecvEndByte(0xBB) # default endByte = '\n'
    
    ######### Start Read Async ##########
    UART.read_async(callback=UART_RecvCallback)

    # Send data to Arduino 'A' - Expected Recving 'B' from Arduino
    UART.send_byte('AA')

    while True:
        # Add your code
        character = input("Input the character to send to Arduino: ")
        UART.send_byte(str(character))
        time.sleep(2)
        character = input("Input the character to send to Arduino: ")
        UART.send_byte(str(character))
        time.sleep(2)
    
###############################################
if __name__ == "__main__":
    main()
