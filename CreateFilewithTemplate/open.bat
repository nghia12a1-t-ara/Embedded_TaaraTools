REM @file			open.bat
REM @brief			Script to open file or create file if it doesn't exist
REM @createdDate	22-Nov-2024
REM @author			Nghia Taarabt

@echo off
REM Check if input parameters are passed
if "%~1"=="" (
    echo Usage: %0 <filename> [npp|vscode]
    exit /b 1
)

SET DEFAULT_FILENAME=""
SET DEFAULT_EDITOR="npp"

set argv1=%~1
set argv2=%~2
set argv3=%~3
set argv4=%~4

set "filestatus="
set "filename="

call :get_filename %argv1% filestatus filename

if /i "%filestatus%"=="1" (
    REM - file exists -> Open file with editor
    call :open_file %filename% %argv2%
) else if /i "%filestatus%"=="0" (
    REM - file does not exist -> Create file and open it
    call create.bat %argv1% %argv2% %argv3% %argv4%
    call :open_file %argv1% %argv2%
) else if /i "%filestatus%"=="2" (
    REM - open last created file
    call :open_file %filename% %argv2%
) else (
    REM - file does not exist and no last file found
    echo Error: Please create file! or provide the correct filename!
)
exit /b

REM ----------------------------------------------------------##
REM ----------------------------------------------------------##
REM Function to open the file with the specified editor
:open_file
    setlocal
    set "filename=%~1"
    set "editor=%~2"
 
    if exist "%CD%\%filename%" (
        set "filestatus=1"
        if /i "%editor%"=="npp" (
            start notepad++ "%CD%\%filename%"
        ) else if /i "%editor%"=="vscode" (
            start /B code "%CD%\%filename%"
        ) else (
            start "%CD%\%filename%"
        )
    )
EXIT /B 0

REM Function to check if the file exists or create it
:get_filename
    setlocal
    REM Store the input argument in "argv"
    set "argv=%~1"
    set "filestatus=0"
    set "filename="

    REM Check file extension
    for %%f in (%argv%) do set EXT=%%~xf

    REM Determine template based on file extension (.c or .h)
    if /i "%EXT%"==".c" (
        set "filestatus=1"
    ) else if /i "%EXT%"==".h" (
        set "filestatus=1"
    ) else (
        REM If the file extension is neither .c nor .h
        set "filestatus=0"
    )

    REM If filestatus is 1 (valid file type), check if it exists
    if "%filestatus%"=="1" (
        set filename=%argv%
  if /i "%argv%"=="" (
   set "filestatus=0"
  ) else if not exist "%CD%\%argv%" (
   set "filestatus=0"
  )
    ) else (
        REM If the file extension is invalid, check for the last created file
        if exist "%~dp0last_file.txt" (
            set /p CREATED_FILE=<"%~dp0last_file.txt"
            REM Check if the previously created file exists
            if exist "%CREATED_FILE%" (
                set "filename=%CREATED_FILE%"
                set "filestatus=2"
            ) else (
                set "filestatus=3"
            )
        ) else (
            set "filestatus=3"
        )
    )
 
 REM Return filestatus and filename values to the caller
    endlocal & set "%2=%filestatus%" & set "%3=%filename%"
EXIT /B 0
