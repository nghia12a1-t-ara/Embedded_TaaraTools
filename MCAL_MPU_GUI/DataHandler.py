import os
from pathlib import Path

### Symbol of Multiple Code Generation in Source Codes
CODE_GEN_START_TEXT = '/* Generate Code */'
CODE_GEN_END_TEXT   = '/* !Generate Code */'

"""
 @ List all Configuration Files to Gen Sources
"""
def getConfigFile(Folder):
    ListFiles = []
    dir_list = os.listdir(Folder)
    for subdir in dir_list:
        ListConfigFile = os.listdir(Folder + '\\' + subdir)
        for Files in ListConfigFile:
            ListFiles.append(Folder + '\\' + subdir + '\\' + Files)
    return ListFiles

"""
 @ Generate Source File from File Config
"""
def getSourceFile(ConfigFile):
    parentProj = getParent(ConfigFile, 2)
    Filename = getFileName(ConfigFile, 1)
    
    GenerateDir = parentProj + "\\Generate"
    GenerateFile = GenerateDir + "\\" + Filename
    return GenerateFile

"""
 @ Generate Source File from File Config
"""
def GenerateFile(Folder, ConfigList, ConfigListName):
    # Last Elements of ConfigList = boolCheckingList
    configLen = len(ConfigList) - 1
    boolCheckList = ConfigList[configLen]
    listBoolName = ["b_DefaultMemMap", "b_MPUinFault", "b_EnMemFault"]
    
    # Get all configuration files
    ConfigListFile = getConfigFile(Folder)
    
    ### Generate each files in Config Folder
    for i in range(len(ConfigListFile)):
        GenString = ''
        cfgFile = ConfigListFile[i]
        mysrcfile = Path(cfgFile)
        mysrcfile.touch(exist_ok=True)
            
        GenFile = getSourceFile(cfgFile)
        mydestfile = Path(GenFile)
        mydestfile.touch(exist_ok=True)
        
        fsrc = open(mysrcfile, 'r')
        srcText = fsrc.read()

        Index_1 = srcText.find(CODE_GEN_START_TEXT) + len(CODE_GEN_START_TEXT)
        Index_2 = srcText.find(CODE_GEN_END_TEXT)
        
        ### Generate multiple config Code
        if Index_1 != -1 and Index_2 != -1:
            # Handler Config Code
            GenString = ReplaceCfg(srcText, ConfigList, ConfigListName)
            srcText = srcText.replace(srcText[Index_1:Index_2], GenString)
        
        # Handler boolean Cheking
        for xx in range(len(listBoolName)):
            srcstr = '$(' + str(listBoolName[xx]) + ')$'
            if srcstr in srcText:
                srcText = srcText.replace(srcstr, str(boolCheckList[xx]))
        
        if '$(NumberOfRegions)$' in srcText:
            srcText = srcText.replace('$(NumberOfRegions)$', str(configLen))
        
        ### Write Data to Generate Files ###
        fdest = open(mydestfile, 'w')
        destText = fdest.write(srcText)

"""
 @ Replace Configuration - Data Handler
"""    
def ReplaceCfg(srctext, ConfigList, ConfigListName):

    # Last Elements of ConfigList = boolCheckingList
    configLen = len(ConfigList) - 1
    boolCheckList = ConfigList[configLen]
    
    GenString = ""
    Index_1 = srctext.find('/* Generate Code */') + 19
    Index_2 = srctext.find('/* !Generate Code */')
    cfgStr = srctext[Index_1:Index_2]
        
    for i in range(configLen):
        tempStr = cfgStr
        for j in range(len(ConfigListName)):
            srcstr = '$(' + ConfigListName[j] + ')$'
            if srcstr in tempStr:
                tempStr = tempStr.replace(srcstr, ConfigList[i][j])
        GenString = GenString + tempStr + '\n'
    if ',' in GenString:
        lastCommaIndex = GenString.rfind(',')
        GenString = GenString[0:lastCommaIndex] + '\n'

    return GenString
    
"""
 @ function to get parent directory
"""
def getParent(path, levels = 1):
    common = path
 
    # Using for loop for getting
    # starting point required for
    # os.path.relpath()
    for i in range(levels + 1):
 
        # Starting point
        common = os.path.dirname(common)
    
    # Parent directory upto specified
    # level
    return common

"""
 @ function to get Folder + File Name
"""
def getFileName(path, levels = 1):
    common = path
 
    # Using for loop for getting
    # starting point required for
    # os.path.relpath()
    for i in range(levels + 1):
 
        # Starting point
        common = os.path.dirname(common)
 
    # Parent directory upto specified
    # level
    return os.path.relpath(path, common)
