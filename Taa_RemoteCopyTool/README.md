# Taa Remote Copy Tool

**Taa Remote Copy Tool** is a GUI-based application built with PyQt6 that allows users to copy files and directories from a remote server to a local machine using SCP (Secure Copy Protocol) over SSH. It provides features like checksum verification, parallel file transfers, and a user-friendly interface for managing remote file transfers.
 - @author : Nghia Taarabt
 - @website: https://www.laptrinhdientu.com/

## Features
- **Remote File Listing**: View the contents of a remote directory via SSH.
- **File and Directory Copy**: Copy single files or entire directories from a remote server to your local machine.
- **Checksum Verification**: Optional MD5 checksum verification to ensure file integrity.
- **Parallel Transfers**: Copy multiple files concurrently with configurable worker threads.
- **Progress Tracking**: Detailed progress dialogs for file transfers.
- **Settings Customization**: Adjust parallel workers, checksum usage, and chunk size for large files.
- **Path Persistence**: Remembers the last used remote path.
- **Home Reset**: Quickly reset the remote path to the user's home directory.

## Prerequisites
- **Python 3.8+**: Required to run the script or build the executable.
- **PyQt6**: For the GUI interface.
- **PyInstaller** (optional): For building the standalone executable.
- **SSH Client**: An SSH client (e.g., OpenSSH) must be installed and accessible via the command line.

## Installation
1. **Clone or Download the Repository**:
   ```bash
   git clone <repository-url>
   cd Taa_RemoteCopyTool
   ```
	Or download the ZIP file and extract it.

2. **Install Dependencies: Install the required Python packages**:
	```bash
   pip install PyQt6
   ```
	Optionally, install PyInstaller for building:
   ```bash
   pip install pyinstaller
   ```
3. **Ensure SSH is Installed**:
 - On Windows: Install Git Bash or an SSH client like OpenSSH.
 - On Linux/macOS: SSH is typically pre-installed.

## Usage
### Running the Script
1. Ensure all dependencies are installed.
2. Run the script directly:
	```bash
	python Taa_RemoteCopyTool.py
	```
### Interface Overview
 - Username: Enter the remote server username (e.g., worker).
 - IP/Host: Enter the remote server IP or hostname (e.g., 192.168.1.100 or example.com).
 - Port: Specify the SSH port (default is 22).
 - Remote Path: Input the remote file or directory path (e.g., /home/worker/documents).
 - Local Path: Choose the local destination folder using the "Browse" button.
 - Buttons:
	- Check SSH Connection: Test the SSH connection to the remote server.
	- List Remote Directory: Display the contents of the specified remote path.
	- Copy Item: Start copying the selected file or directory.
	- Settings: Configure copy options (workers, checksum, etc.).
	- Home: Reset the remote path to /home/<username>.
### Copying Files
 - Fill in the connection details and remote/local paths.
 - Click "Check SSH Connection" to verify connectivity.
 - Click "List Remote Directory" to explore the remote server.
 - Double-click directories to navigate, or select a file/directory and click "Copy Item".
 - Monitor the progress in the dialog that appears.
 
## Building the Executable
	To create a standalone executable, use the provided build.py script:

1. Prepare Files:
	Ensure Taa_RemoteCopyTool.py, build.py, bmc.svg, and icon.ico are in the same directory.
2. Run the Build Script:
	```bash
	python build.py
	```
3. Output:
 - The executable (Taa_RemoteCopyTool.exe on Windows or Taa_RemoteCopyTool on Linux/macOS) will be in the ./dist folder.
 - The bmc.svg (donate button icon) and icon.ico (application icon) are bundled with the executable.
### Notes for Built Executable
 - The built application includes the icon (icon.ico) and donate button image (bmc.svg).
 - Run the executable directly without needing Python installed.
## Troubleshooting
 - SSH Errors:
	- "Permission denied": Check username and SSH key/password.
	- "Connection refused": Verify the host and port.
 - File Not Found: Ensure ssh and scp commands are available in your system's PATH.
 - Build Fails: Confirm PyInstaller is installed and data files (bmc.svg, icon.ico) exist.
### Contributing
Feel free to submit issues or pull requests to improve the tool. Suggestions for new features or bug fixes are welcome!

## Donate
Support the project by donating via the "Donate" button in the application (powered by bmc.svg).
