@echo off
REM @file			GenFile.bat
REM @brief			Create file with template
REM @createdDate	22-Nov-2024
REM @author			Nghia Taarabt

REM echo Start GenFile.bat

REM Get the path of the batch file being executed
set BAT_DIR=%~dp0

REM Check if input parameters are provided
if "%~1"=="" (
    echo Usage: %0 ^<filename^> [-m description]
    exit /b 1
)

set FILENAME=%~1
set DESCRIPTION=Detail Descriptions for the Source Code

:parse_options
if "%~1"=="" goto done
if "%~1"=="-m" (
    set DESCRIPTION=%2
    shift
)
shift
goto parse_options
:done

REM Extract the file extension
for %%f in (%FILENAME%) do set EXT=%%~xf

REM Determine the template based on the file extension
if /i "%EXT%"==".c" (
    set TEMPLATE_FILE=%BAT_DIR%template.c
) else if /i "%EXT%"==".h" (
    set TEMPLATE_FILE=%BAT_DIR%template.h
) else (
    echo Error: Unsupported file extension "%EXT%". Only .c and .h are supported.
    exit /b 1
)

REM Check if the template file exists
if not exist "%TEMPLATE_FILE%" (
    echo Error: Template file "%TEMPLATE_FILE%" not found.
    exit /b 1
)

REM Set the output file path to the current directory
set OUTPUT_FILE=%CD%\%FILENAME%

REM Check if the file already exists
if exist "%OUTPUT_FILE%" (
    echo Error: File "%OUTPUT_FILE%" already exists.
    exit /b 1
)

REM Get the current date and time
for /f "tokens=2 delims==" %%i in ('wmic OS Get localdatetime /value') do set DT=%%i
REM Format the date as YYYY-MM-DD
set CURRENT_DATE=%DT:~0,4%-%DT:~4,2%-%DT:~6,2%

REM Convert the filename to uppercase (handling the first character)
REM Remove the file extension
REM Convert to uppercase
setlocal enabledelayedexpansion
set UPPERNAME=%FILENAME%
set UPPERNAME=!UPPERNAME:~0,-2!
REM set UPPERNAME=!UPPERNAME!
for /f "usebackq delims=" %%I in (`powershell "\"%UPPERNAME%\".toUpper()"`) do set "UPPERNAME=%%~I"

REM Copy the template file and perform replacements
copy "%TEMPLATE_FILE%" "%OUTPUT_FILE%" >nul

REM Perform replacements for {{filename}}, {{uppername}}, {{date}}, {{time}}, {{description}}
powershell -Command "(Get-Content '%OUTPUT_FILE%') -replace '{{filename}}', '%FILENAME:~0,-2%' -replace '{{uppername}}', '%UPPERNAME%' -replace '{{date}}', '%CURRENT_DATE%' -replace '{{time}}', '%CURRENT_TIME%' -replace '{{description}}', '%DESCRIPTION%' | Set-Content '%OUTPUT_FILE%'"

REM Create the 'last_file.txt' in the same directory as the batch file and store the full path of the created file
echo %OUTPUT_FILE% > "%BAT_DIR%last_file.txt"

echo File generated successfully: %OUTPUT_FILE%

REM echo End GenFile.bat

exit /b
