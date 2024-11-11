import sys
import time, os

from PyQt5.QtWidgets import (
    QApplication, QDialog, QMainWindow, QMessageBox, QFileDialog, QCheckBox, QTableWidget
)
from PyQt5.uic import loadUi
from PyQt5 import QtWidgets
from MPU_UI import Ui_MainWindow
from AddRegion import Ui_AddConfig
from ast import literal_eval
from DataHandler import ReplaceCfg, GenerateFile


ConfigList = []     ## List to save all Configuration
tempConfig = {
        "RegionNumber" : "",
        "StartAddress" : "",
        "EndAddress" : "",
        "MemoryTypes" : "",
        "AccessRights" : ""
    }
ConfigListName = list(tempConfig.keys())
booleanConfigList = ["", "", ""]
tempList = []
SizeList = []

#################################################################
################ Get PARAMETERS of Configuration ################
#################################################################
class ConfigWindow(QMainWindow, Ui_AddConfig):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setupUi(self)
        self.connectSignalsSlots()

    def connectSignalsSlots(self):
        self.Combo_MemTypes.activated[str].connect(self.SetupMemTypes)
        self.Combo_AccessRight.activated[str].connect(self.SetupAccessRight)
        self.Accept.clicked.connect(self.ConfigAccept)
        self.Cancel.clicked.connect(self.ConfigReject)

    def ConfigAccept(self):
        RegionNumText = self.RegionNumber.toPlainText()
        StartAddrText = self.StartAddr.toPlainText()
        EndAddrText = self.EndAddr.toPlainText()
        ErrorFlag = False
        ErrorText = ""

        #------------------------ Check Empty Parameters ------------------------#
        if ( RegionNumText == '' ) or ( StartAddrText == '' ) or ( EndAddrText == '' ):
            ErrorFlag = True
            ErrorText = ErrorText + "You must fill all Parameters"

        ##########################################################################   
        #------------------------ Check VALID Parameters ------------------------#
        if ( ErrorFlag == False ):
            # Convert Text to Decimal Number (Check Hexa Syntax)
            if '0x' in RegionNumText:
                RegionNum = literal_eval(RegionNumText)
            else:
                RegionNum = int(RegionNumText)
   
            if '0x' in StartAddrText:
                StartAddr = literal_eval(StartAddrText)
            else:
                StartAddr = int(StartAddrText)

            if '0x' in EndAddrText:
                EndAddr = literal_eval(EndAddrText)
            else:
                EndAddr = int(EndAddrText)

            # Calculate Size of Region
            Size = EndAddr - StartAddr

            # Verify End Address > Start Address -> Size > 0
            if Size <= 0:
                ErrorFlag = True
                ErrorText = ErrorText + " Region End Address must be > Region Start Address\n"
            
            # Verify Region Number is from 0 to 7
            if ( RegionNum < 0 ) or ( RegionNum > 7 ):
                ErrorFlag = True
                ErrorText = ErrorText + "Region Number must be from 0 to 7\n"

            # Verify Start Address is from 0x0 to 0xFFFF.FFF0
            if ( StartAddr < 0 ) or ( StartAddr > 4294967280 ):
                ErrorFlag = True
                ErrorText = ErrorText + "Start Address must be >= 0 and <= ‭4294967280‬\n"

            # Verify End Address is from 0x0 to 0xFFFF.FFF0
            if ( EndAddr < 32 ) or ( EndAddr > 4294967295 ):
                ErrorFlag = True
                ErrorText = ErrorText + "End Address must be >= 32 and <= ‭4294967295‬\n"
            
            # Verify Size is multiple of 4096
            if ((Size % 4096) != 4095) and ((Size % 4096) != 0):
                ErrorFlag = True
                ErrorText = ErrorText + "Region Size must be multiple of 4096\n"
        
        #------------------------ Check Status ------------------------#
        if ErrorFlag == True:
            QMessageBox.about(self, "Error", ErrorText)
            
        else:
            tempConfig["RegionNumber"] = str(RegionNum)
            tempConfig["StartAddress"] = str(StartAddr)
            tempConfig["EndAddress"] = str(EndAddr)
            tempConfig["MemoryTypes"] = ConfigWindow.SetupMemTypes(self)
            tempConfig["AccessRights"] = ConfigWindow.SetupAccessRight(self)

            # Calculate Region Size
            tempSize = str(Size)
            SizeList.append(tempSize)
            
            ListTemp = list(tempConfig.values())
            
            ConfigList.append(ListTemp)

            # Show Region Config to Main Window
            ### Add Item to list View
            win.RegionList.addItem(ListTemp[0])
            win.StartAddrList.addItem(ListTemp[1])
            win.EndAddrList.addItem(ListTemp[2])
            win.MemTypeList.addItem(ListTemp[3])
            win.AccessList.addItem(ListTemp[4])
            win.RegionSizeList.addItem(tempSize)

            ### ShowRegionConfig()
            win.RegionList.show()
            win.StartAddrList.show()
            win.EndAddrList.show()
            win.MemTypeList.show()
            win.AccessList.show()
            win.RegionSizeList.show()

            ### Close Config Window
            ConfigWin.close()

    def ConfigReject(self):
        ConfigWin.close() 

    def SetupMemTypes(self):
        MemTypeText = self.Combo_MemTypes.currentText()
        return MemTypeText

    def SetupAccessRight(self):
        AccessText = self.Combo_AccessRight.currentText()
        return AccessText
    
#################################################################
################## Main WINDOW for Show Config ##################
#################################################################
class Window(QMainWindow, Ui_MainWindow):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setupUi(self)
        self.tableRegion = QTableWidget(8, 6)
        self.connectSignalsSlots()
	
    def connectSignalsSlots(self):
        self.addRegion.clicked.connect(self.AddRegion)
        self.DeleteRegion.clicked.connect(self.DelRegion)
        self.pushButton.clicked.connect(self.GenarateCode)
        self.toolButton.clicked.connect(self.FindFiles)
        self.checkBox_1.stateChanged.connect(self.checkDefaultMem)
        self.checkBox_2.stateChanged.connect(self.checkMPUinFault)
        self.checkBox_3.stateChanged.connect(self.checkEnMemFault)

    def checkDefaultMem(self):
        if self.checkBox_1.isChecked():
            DefaultMem = True
        else:
            DefaultMem = False
        return str(DefaultMem).upper()

    def checkMPUinFault(self):
        if self.checkBox_2.isChecked():
            MPUinFault = True
        else:
            MPUinFault = False
        return str(MPUinFault).upper()
        
    def checkEnMemFault(self):
        if self.checkBox_3.isChecked():
            EnMemFault = True
        else:
            EnMemFault = False
        return str(EnMemFault).upper()

    def DelRegion(self):
        # Clear Last Row of All List Config
        countConfig = win.RegionList.count()
        LastIndex = int(countConfig - 1)
        
        if countConfig > 0:
            # Remove Last Item of ConfigList
            ConfigList.pop()
            SizeList.pop()
            
            # Remove Last Item in QListView
            self.RegionList.takeItem(LastIndex)
            self.StartAddrList.takeItem(LastIndex)
            self.EndAddrList.takeItem(LastIndex)
            self.MemTypeList.takeItem(LastIndex)
            self.AccessList.takeItem(LastIndex)
            self.RegionSizeList.takeItem(LastIndex)
        
    #------------------------ Add Data Config -------------------------#
    def CollectData(self):
        booleanConfigList[0] = Window.checkDefaultMem(self)
        booleanConfigList[1] = Window.checkMPUinFault(self)
        booleanConfigList[2] = Window.checkEnMemFault(self)

        # Add Boolean Checking Config to last of List
        ConfigList.append(booleanConfigList)

    def AddRegion(self):
        # Open Configuration Window
        ConfigWin.show()
        currentRegion = int(len(ConfigList))
        ConfigWin.RegionNumber.setText(str(currentRegion))

    def FindFiles(self):
        ConfigPath = QFileDialog.getExistingDirectory()
        self.textEdit.setText(ConfigPath)
        tempList.append(ConfigPath)

    def GenarateCode(self):
        Window.CollectData(self)
        boolCheckConfig = ConfigList[(len(ConfigList) - 1)]
        
        GenerateFile(tempList[0], ConfigList, ConfigListName)
        TIME_LIMIT = 100
        count = 0

        ### Reset all lists
        ConfigList = []
        booleanConfigList = ["", "", ""]
        tempList = []
        SizeList = []

        ### Generate XML file output
        GenerateXML()

        ### Progress and Notification
        while count < TIME_LIMIT:
            count += 1
            time.sleep(0.001)
            self.GenProcess.setValue(count)
            
        QMessageBox.about(self, "Infomation", "Generate Code Done!!!")
        
#################################################################
########################## MAIN FUNCTION ########################
#################################################################
if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = Window()
    ConfigWin = ConfigWindow()
    win.show()
    sys.exit(app.exec())

#################################################################
#################################################################
   
