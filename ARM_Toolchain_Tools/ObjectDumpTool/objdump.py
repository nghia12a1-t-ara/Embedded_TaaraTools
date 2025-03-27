# Project: ARM Objdump Tool - use the Objdump to analysis elf file
# Author: Nghia Taarabt - laptrinhdientu.com
# Date: 2023-02-20

import sys
import subprocess
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QPushButton, QLineEdit, QTextEdit, 
                             QLabel, QFileDialog, QGroupBox, QCheckBox, QComboBox)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QIcon, QPixmap

class ObjdumpTool(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("ARM Objdump Tool - from nghia - laptrinhdientu.com")
        self.setGeometry(100, 100, 800, 600)

        # Set window icon
        self.setWindowIcon(QIcon("logo.ico"))

        # Main widget
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.layout = QHBoxLayout(self.central_widget)  # Use horizontal layout for left panel and main area

        # Settings panel on the left
        self.settings_panel = QGroupBox("Settings")
        self.settings_panel.setFixedWidth(250)  # Set a fixed width for the settings panel
        self.settings_layout = QVBoxLayout(self.settings_panel)

        # File Information
        self.file_info_group = QGroupBox("File Information")
        self.file_info_layout = QVBoxLayout(self.file_info_group)
        self.show_file_info = QCheckBox("Show File Information (-f)")
        self.show_section_headers = QCheckBox("Show Section Headers (-h)")
        self.show_all_headers = QCheckBox("Show All Headers (-x)")
        self.file_info_layout.addWidget(self.show_file_info)
        self.file_info_layout.addWidget(self.show_section_headers)
        self.file_info_layout.addWidget(self.show_all_headers)
        self.settings_layout.addWidget(self.file_info_group)

        # Disassembly
        self.disassembly_group = QGroupBox("Disassembly")
        self.disassembly_layout = QVBoxLayout(self.disassembly_group)
        self.disassemble_code = QCheckBox("Disassemble Code Section (-d)")
        self.disassemble_entire_file = QCheckBox("Disassemble Entire File (-D)")
        self.show_binary_content = QCheckBox("Show Binary Content (-s)")
        self.section_combo = QComboBox()
        self.section_combo.addItems(["None - All Sections"])  # Example sections
        self.disassemble_symbol_text = QTextEdit()
        self.disassemble_symbol_text.setPlaceholderText("Enter symbol to disassemble")
        
        self.disassembly_layout.addWidget(self.disassemble_code)
        self.disassembly_layout.addWidget(self.disassemble_entire_file)
        self.disassembly_layout.addWidget(self.show_binary_content)
        self.disassembly_layout.addWidget(QLabel("Section:"))
        self.disassembly_layout.addWidget(self.section_combo)
        self.disassembly_layout.addWidget(QLabel("Disassemble Symbol:"))
        self.disassembly_layout.addWidget(self.disassemble_symbol_text)
        self.settings_layout.addWidget(self.disassembly_group)

        # Debug Information
        self.debug_info_group = QGroupBox("Debug Information")
        self.debug_info_layout = QVBoxLayout(self.debug_info_group)
        self.show_debug_info = QCheckBox("Show Debug Information (-g)")
        self.show_tag_debugging = QCheckBox("Show Tag Debugging (-e)")
        self.debug_info_layout.addWidget(self.show_debug_info)
        self.debug_info_layout.addWidget(self.show_tag_debugging)
        self.settings_layout.addWidget(self.debug_info_group)

        # Additional Options
        self.additional_group = QGroupBox("Additional Options")
        self.additional_layout = QVBoxLayout(self.additional_group)
        self.show_relocation_table = QCheckBox("Show Relocation Table (-r)")
        self.show_symbol_table = QCheckBox("Show Symbol Table (-t)")
        self.show_dynamic_symbols = QCheckBox("Show Dynamic Symbols (-T)")
        self.show_dynamic_segment = QCheckBox("Show Dynamic Segment (-p)")
        self.show_dynamic_relocation = QCheckBox("Show Dynamic Relocation (-R)")
        self.show_dwarf_info = QCheckBox("Show DWARF Information (-W)")
        
        self.additional_layout.addWidget(self.show_relocation_table)
        self.additional_layout.addWidget(self.show_symbol_table)
        self.additional_layout.addWidget(self.show_dynamic_symbols)
        self.additional_layout.addWidget(self.show_dynamic_segment)
        self.additional_layout.addWidget(self.show_dynamic_relocation)
        self.additional_layout.addWidget(self.show_dwarf_info)
        self.settings_layout.addWidget(self.additional_group)

        # Demangle and DWARF Info ComboBoxes
        self.demangle_combo = QComboBox()
        self.demangle_combo.addItems(["None"])  # Example options
        self.dwarf_info_combo = QComboBox()
        self.dwarf_info_combo.addItems(["None"])  # Example options
        
        self.settings_layout.addWidget(QLabel("Demangle:"))
        self.settings_layout.addWidget(self.demangle_combo)
        self.settings_layout.addWidget(QLabel("DWARF Info:"))
        self.settings_layout.addWidget(self.dwarf_info_combo)

        # Add the settings panel to the main layout
        self.layout.addWidget(self.settings_panel)

        # Main content area
        self.main_content = QVBoxLayout()

        # ELF file input field and Browse button
        self.file_input_layout = QHBoxLayout()
        self.file_label = QLabel("ELF File Path:")
        self.file_input = QLineEdit()
        self.file_input.setPlaceholderText("Enter or browse to ELF file")
        self.browse_button = QPushButton("Browse")
        self.browse_button.clicked.connect(self.browse_file)
        self.file_input_layout.addWidget(self.file_label)
        self.file_input_layout.addWidget(self.file_input)
        self.file_input_layout.addWidget(self.browse_button)
        self.main_content.addLayout(self.file_input_layout)
        self.file_input.editingFinished.connect(self.on_file_input_changed)

        # Run command button
        self.run_button = QPushButton("Run Objdump")
        self.run_button.clicked.connect(self.run_objdump)
        self.main_content.addWidget(self.run_button)

        # Output area with search and watermark
        self.output_text = QTextEdit()
        self.output_text.setReadOnly(True)
        self.main_content.addWidget(self.output_text)

        # Search area
        self.search_layout = QHBoxLayout()
        self.search_label = QLabel("Search:")
        self.search_input = QLineEdit()
        self.search_input.setPlaceholderText("Enter text to search")
        self.search_input.returnPressed.connect(self.find_text)
        self.find_button = QPushButton("Find")
        self.find_button.clicked.connect(self.find_text)
        self.copy_button = QPushButton("Copy")
        self.copy_button.clicked.connect(self.copy_output)
        self.search_layout.addWidget(self.search_label)
        self.search_layout.addWidget(self.search_input)
        self.search_layout.addWidget(self.find_button)
        self.search_layout.addWidget(self.copy_button)
        
        self.main_content.addLayout(self.search_layout)

        # Add the main content to the layout
        self.layout.addLayout(self.main_content)

        # Initialize watermark label
        self.watermark_label = QLabel(self.output_text)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self.update_watermark()
        
    def update_watermark(self):
        # Load the logo image
        self.original_pixmap = QPixmap("logo.png")
        
        if not self.original_pixmap.isNull():
            # Get the output text dimensions (dimensions of self.output_text)
            text_width = self.output_text.viewport().width()  # Use viewport to get the actual display area dimensions
            text_height = self.output_text.viewport().height()
            
            # Calculate the desired size (30% of the output text's smaller dimension)
            target_size = min(text_width, text_height) * 0.7  # 70% of the smaller dimension
            
            # Scale the pixmap to the target size while maintaining aspect ratio
            scaled_pixmap = self.original_pixmap.scaled(
                int(target_size), int(target_size),
                Qt.AspectRatioMode.KeepAspectRatio,
                Qt.TransformationMode.SmoothTransformation
            )
            
            # Update the watermark label with the scaled pixmap
            self.watermark_label.setPixmap(scaled_pixmap)
            
            # Calculate the position to center the watermark in the output text area
            x_position = (text_width - scaled_pixmap.width()) // 2
            y_position = (text_height - scaled_pixmap.height()) // 2
            
            # Set the geometry of the watermark label to center it
            self.watermark_label.setGeometry(
                x_position,
                y_position,
                scaled_pixmap.width(),
                scaled_pixmap.height()
            )
            
            # Ensure proper opacity and transparency
            self.watermark_label.setStyleSheet("background-color: transparent; opacity: 0.7;")
            
            # Ensure the watermark label is visible and on top of the output text
            self.watermark_label.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)  # Allow interaction with output_text below
            self.watermark_label.raise_()  # Bring watermark to the front
        else:
            print("Error: Could not load logo.png. Ensure the file exists in the correct directory.")

    def on_file_input_changed(self):
        file_path = self.file_input.text().strip()
        if file_path:
            self.update_section_combo(file_path)
        else:
            # If there is no file path, clear the section_combo and only keep "None - All Sections"
            self.section_combo.clear()
            self.section_combo.addItem("None - All Sections")

    def browse_file(self):
        file_path, _ = QFileDialog.getOpenFileName(self, "Select ELF File", "", "ELF Files (*.elf);;All Files (*)")
        if file_path:
            self.file_input.setText(file_path)
            self.update_section_combo(file_path)

    def update_section_combo(self, elf_file):
        # Clear the current items in section_combo
        self.section_combo.clear()

        # Add a default item (no section selected)
        self.section_combo.addItem("None - All Sections")

        # Run the command arm-none-eabi-objdump -h to get the list of sections
        command = ["arm-none-eabi-objdump", "-h", elf_file]
        try:
            result = subprocess.run(command, capture_output=True, text=True, check=True)
            sections = self.parse_sections(result.stdout)
            if sections:
                self.section_combo.addItems(sections)
            else:
                self.output_text.setText("Warning: No sections found in the ELF file.")
        except subprocess.CalledProcessError as e:
            self.output_text.setText(f"Error while scanning sections: {e.stderr}")
        except FileNotFoundError:
            self.output_text.setText("Error: 'arm-none-eabi-objdump' not found. Ensure it is installed and in PATH.")

    def parse_sections(self, objdump_output):
        sections = []
        lines = objdump_output.splitlines()
        
        # Find the "Sections:" part in the output
        start_parsing = False
        for line in lines:
            if "Sections:" in line:
                start_parsing = True
                continue
            if start_parsing:
                # Each line after "Sections:" has the format: Idx Name Size VMA LMA File off Algn
                parts = line.split()
                if len(parts) >= 2 and parts[0].isdigit():  # Check if the line has a valid format
                    section_name = parts[1]  # The section name is in the second column
                    if section_name.startswith("."):  # Only take sections starting with "."
                        sections.append(section_name)
        
        return sections

    def run_objdump(self):
        # Get the ELF file path from the input
        elf_file = self.file_input.text().strip()
        if not elf_file:
            self.output_text.setText("Error: Please specify an ELF file path.")
            return

        # Initialize the command list with the basic command
        command = ["arm-none-eabi-objdump"]

        # File Information options
        if self.show_file_info.isChecked():
            command.append("-f")
        if self.show_section_headers.isChecked():
            command.append("-h")
        if self.show_all_headers.isChecked():
            command.append("-x")

        # Disassembly options
        disassemble_enabled = False  # Variable to check if disassembly is enabled
        if self.disassemble_code.isChecked():
            command.append("-d")
            disassemble_enabled = True
        if self.disassemble_entire_file.isChecked():
            command.append("-D")
            disassemble_enabled = True
        if self.show_binary_content.isChecked():
            command.append("-s")

        # Debug Information options
        if self.show_debug_info.isChecked():
            command.append("-g")
        if self.show_tag_debugging.isChecked():
            command.append("-e")

        # Additional Options
        if self.show_relocation_table.isChecked():
            command.append("-r")
        if self.show_symbol_table.isChecked():
            command.append("-t")
        if self.show_dynamic_symbols.isChecked():
            command.append("-T")
        if self.show_dynamic_segment.isChecked():
            command.append("-p")
        if self.show_dynamic_relocation.isChecked():
            command.append("-R")
        if self.show_dwarf_info.isChecked():
            command.append("-W")

        # Handle Section (only add --section if -d or -D is selected and section is not "None - All Sections")
        selected_section = self.section_combo.currentText().strip()
        if disassemble_enabled and selected_section and selected_section != "None - All Sections":
            command.append(f"--section={selected_section}")

        # Handle Disassemble Symbol (if any)
        symbol = self.disassemble_symbol_text.toPlainText().strip()
        if symbol:
            command.append(f"--disassemble={symbol}")

        # Handle Demangle (if needed)
        demangle_option = self.demangle_combo.currentText()
        if "None" != demangle_option:
            if demangle_option == "Option 1":  # Change according to the actual value
                command.append("--demangle")

        # Handle DWARF Info (if needed)
        dwarf_info = self.dwarf_info_combo.currentText()
        if "None" != dwarf_info and self.show_dwarf_info.isChecked():
            # Add custom logic if needed
            pass

        # Add the ELF file path to the end of the command
        command.append(elf_file)

        # Display the constructed command (for debugging purposes)
        self.output_text.setText(f"Running command: {' '.join(command)}\n\n")

        # Run the objdump command
        try:
            result = subprocess.run(command, capture_output=True, text=True, check=True)
            self.output_text.append(result.stdout)
        except subprocess.CalledProcessError as e:
            self.output_text.append(f"Error: {e.stderr}")
        except FileNotFoundError:
            self.output_text.append("Error: 'arm-none-eabi-objdump' not found. Ensure it is installed and in PATH.")

    def copy_output(self):
        clipboard = QApplication.clipboard()
        clipboard.setText(self.output_text.toPlainText())
        self.statusBar().showMessage("Output copied to clipboard!", 2000)

    def find_text(self):
        search_term = self.search_input.text().strip()
        if not search_term:
            self.statusBar().showMessage("Please enter a search term.", 2000)
            return

        if self.output_text.find(search_term):
            self.statusBar().showMessage(f"Found '{search_term}'.", 2000)
        else:
            cursor = self.output_text.textCursor()
            cursor.movePosition(cursor.MoveOperation.Start)
            self.output_text.setTextCursor(cursor)
            if self.output_text.find(search_term):
                self.statusBar().showMessage(f"Found '{search_term}' (from start).", 2000)
            else:
                self.statusBar().showMessage(f"'{search_term}' not found.", 2000)

def main():
    app = QApplication(sys.argv)
    window = ObjdumpTool()
    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
