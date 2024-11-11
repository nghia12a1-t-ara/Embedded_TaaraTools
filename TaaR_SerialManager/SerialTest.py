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
    void setup() {
      Serial.begin(9600);
    }
    void loop() {
      // send data only when you receive data
      if (Serial.available() > 0) {
        incomingByte = Serial.read();
        Serial.write(incomingByte+1);
      }
    }
'''

from TaaR_SerialManager import SerialManager

def main():
    pass
    
###############################################
if __name__ == "__main__":
    main()
