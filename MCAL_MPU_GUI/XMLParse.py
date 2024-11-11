import os
from pathlib import Path

"""
 @ Generate XML file output
"""
def GenerateXML(Path):
    ### Path to XML File
    directory = "Output"
    path = os.path.join(Path, directory)
    
    ### Try to create new dir if it's NOT exist
    try:
        os.mkdir(path)
    except:
        pass
    
    
    
    print(repr(path))

GenerateXML(r'D:\STM32\STM32_SourceCode\STM32_Programming\Python_Tools\MPU_GUI')


