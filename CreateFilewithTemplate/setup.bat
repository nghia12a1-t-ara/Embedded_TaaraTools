@echo off
REM Get the current directory where the script is located
set "CURRENT_DIR=%~dp0"
REM Remove the trailing backslash from the directory path
set "CURRENT_DIR=%CURRENT_DIR:~0,-1%"

REM Get the current PATH environment variable value
for /F "tokens=* USEBACKQ" %%G IN (`reg query "HKCU\Environment" /v Path`) do (
    set "REG_PATH=%%G"
)

REM Check if the current directory is already in the PATH environment variable
echo %REG_PATH% | find /i "%CURRENT_DIR%" >nul
if %errorlevel%==0 (
    echo The directory is already in PATH.
) else (
    REM Add the current directory to the PATH environment variable
    setx PATH "%PATH%;%CURRENT_DIR%"
    echo The directory %CURRENT_DIR% has been added to PATH.
)

REM Pause to keep the command window open and display the result
pause
