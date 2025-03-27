# build.py
import os
import sys
from PyInstaller.__main__ import run

# Define the script and data files
script_name = 'objdump.py'
icon_file = 'logo.ico'
image_file = 'logo.png'

# Create a list of data files to include
data_files = [icon_file, image_file]

# Build options
options = [
    '--onefile',  # Create a single executable
    '--windowed',  # No console window
    '--add-data', f"{icon_file};.",  # Include the icon file
    '--add-data', f"{image_file};.",  # Include the image file
    '--distpath', '.',
    script_name  # The main script to build
]

if __name__ == '__main__':
    # Run PyInstaller with the specified options
    run(options)

    # Remove the build and dist folders, and the objdump.spec file
    if os.path.exists('build'):
        os.system('rm -r build')
    if os.path.exists('dist'):
        os.system('rm -r dist')
    if os.path.exists('objdump.spec'):
        os.remove('objdump.spec')
