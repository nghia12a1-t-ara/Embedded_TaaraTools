import os
import sys
import subprocess
import shutil

# Main Python file to build
MAIN_FILE = "Taa_RemoteCopyTool.py"
# Output executable name
OUTPUT_NAME = "Taa_RemoteCopyTool"
# List of data files to include (source path, destination relative path)
DATA_FILES = [
    ("./bmc.svg", "."),  # Include bmc.svg in the root directory of the executable
    ("./icon.ico", "."),  # Include icon.ico in the root directory of the executable
]

# Files and folders to clean up after the build
files = [
    "Taa_RemoteCopyTool.spec",  # Only delete the relevant .spec file
]
folders = [
    "build/",
    "dist/",
    "converted_files/",
    "__pycache__",
]

def run_pyinstaller():
    """Function to execute PyInstaller and build the application"""
    # Check if the main file exists
    if not os.path.exists(MAIN_FILE):
        print(f"Error: File {MAIN_FILE} not found!")
        sys.exit(1)

    # Check if all data files exist
    for src, _ in DATA_FILES:
        if not os.path.exists(src):
            print(f"Error: Data file {src} not found!")
            sys.exit(1)

    # Build the PyInstaller command
    command = [
        "pyinstaller",
        "--onefile",  # Package everything into a single executable file
        "--windowed",  # Run without showing a console (for GUI applications)
        f"--icon={os.path.abspath('./icon.ico')}",  # Set icon.ico as the application icon
        "--name", OUTPUT_NAME,  # Specify the output executable name
        "--distpath", ".",      # Output to the current directory
    ]

    # Add data files to the command
    for src, dst in DATA_FILES:
        command.append("--add-data")
        # Use the appropriate path separator based on the operating system
        if sys.platform == "win32":
            command.append(f"{os.path.abspath(src)};{dst}")  # Windows uses semicolon
        else:
            command.append(f"{os.path.abspath(src)}:{dst}")  # Linux/macOS uses colon

    # Append the main file to the command
    command.append(MAIN_FILE)

    # Print the full command for debugging purposes
    print("Running command:", " ".join(command))

    try:
        # Execute the PyInstaller command and capture output
        result = subprocess.run(command, check=True, text=True, capture_output=True)

        # Clean up temporary files and folders
        for file in files:
            if os.path.exists(file) and os.path.isfile(file):
                os.remove(file)
                print(f"Deleted file: {file}")

        for folder_to_remove in folders:
            if os.path.exists(folder_to_remove) and os.path.isdir(folder_to_remove):
                shutil.rmtree(folder_to_remove)
                print(f"Deleted folder: {folder_to_remove}")

        print("Build successful!")
        print("Output directory:", os.path.abspath("./dist"))
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        # Handle build failure and display error details
        print("Build failed!")
        print("Error output:", e.stderr)
        sys.exit(1)

if __name__ == "__main__":
    # Run the build process when the script is executed
    run_pyinstaller()
