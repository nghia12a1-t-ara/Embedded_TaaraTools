# Project Title

## Description
This project is a Python application that utilizes PyInstaller to create a standalone executable from the `objdump.py` script. The application includes an icon and an image for a better user interface.

## Prerequisites
- Python 3.x (for developer)
- PyInstaller (for developer)
- ARM Toolchain GCC (for building applications for ARM architecture)
- Any other dependencies required by `objdump.py`

### Installation
To install PyInstaller, you can use pip:
```bash
pip install pyinstaller
```

## How to Run
### For Developers
1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-directory>
   ```

2. Build the executable:
   Run the following command to create the executable:
   ```bash
   python build.py
   ```

3. After the build process is complete, you will find the executable in the current directory.

4. Run the application:
   ```bash
   ./objdump  # or objdump.exe on Windows
   ```

### For Users
If you have the executable file, you can run the application directly without needing to install Python or any dependencies. Simply follow these steps:

1. Download the executable file from the provided link.
2. Navigate to the directory where the executable is located.
3. Run the application:
   ```bash
   ./objdump  # or objdump.exe on Windows
   ```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details. 