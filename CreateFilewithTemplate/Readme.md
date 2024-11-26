# Description
This tool is used to create new source C file with the Template, used for the project's coding convension.
> author: **Nghia Taarabt**
> article and guide video: @TODO

# Tool folder Structure
- **GenFile.bat** : Core file to create a new file with template.c and template.h
- **create.bat**  : Script to create new file
- **open.bat**    : Script to Open created file or create new file if it doesn't exist

# Guide
### Setup
1. Ensure your computer has the bash.exe
	> You can setup it manually or setup via [Cygwin package](https://www.laptrinhdientu.com/2024/09/cygwin-gcc-gdb-setup.html)
1. Clone/download this tool to a folder, ex. D:/Tool_CreateFiles
2. Add PATH at "Edit Environment Variable" for this folder

### How to open / create a new file with Template
1. Open the **Command Prompt** in your folder where you want to create new file
	> Example. cd "D:/YourProject" \
	> New File will be created at this directory
2. If you want to create a new file with template only, you can run this Command
- Create File without Description
	> create Filename.c
	> create Filename.h
- Option -m use for add the Description (same the git commit comment)
	> create Filename.c -m "This file use to implement GPIO code"
- After create the new File, you can open it manually by run open Command (text editor option)
	> open \
	> open npp \
	> open vscode
3. If you want to open file or create a new file with openning, you can run this Command
- Open File only when it exist
	> open Filename.c \
	> open Filename.c npp \
	> open Filename.c vscode
- Create and Open File when it doesn't exist
	> open Filename.c npp -m "Your Description"
