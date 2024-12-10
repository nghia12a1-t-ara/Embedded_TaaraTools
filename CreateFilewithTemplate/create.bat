@echo off

REM @file			create.bat
REM @brief			Create file with template
REM @createdDate	22-Nov-2024
REM @author			Nghia Taarabt

echo Creating File .........

REM This file will call GenFile.bat with the passed arguments
call GenFile.bat %*
