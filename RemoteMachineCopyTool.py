import sys
import subprocess
import os
import time
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QLabel, QLineEdit, QPushButton,
                             QTreeWidget, QTreeWidgetItem, QMessageBox, QFileDialog,
                             QProgressDialog)
from PyQt6.QtCore import Qt, QSettings
from PyQt6.QtGui import QIcon
import traceback  # For detailed error logging

# Define flag to hide console on Windows
if sys.platform == "win32":
    CREATE_NO_WINDOW = 0x08000000  # Windows-specific flag to hide console
else:
    CREATE_NO_WINDOW = 0  # No effect on non-Windows systems

class FileCopyTool(QMainWindow):
    def __init__(self):
        super().__init__()
        # Set window title and size
        self.setWindowTitle("SCP File Copy Tool")
        self.setGeometry(100, 100, 800, 600)
        # Optional: Uncomment to set a window icon
        # self.setWindowIcon(QIcon("path/to/your/icon.png"))

        # Initialize settings to save user preferences
        self.settings = QSettings("xAI", "FileCopyTool")
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.setup_ui()

    def setup_ui(self):
        # Main vertical layout for the UI
        layout = QVBoxLayout()

        # Create input fields with saved values from settings
        self.user_input = self.create_input_field(layout, "Username:")
        self.user_input.setText(self.settings.value("username", "worker"))
        self.host_input = self.create_input_field(layout, "IP/Host:")
        self.host_input.setText(self.settings.value("host", ""))
        self.port_input = self.create_input_field(layout, "Port (default 22):")
        self.port_input.setText(self.settings.value("port", ""))
        self.remote_path_input = self.create_input_field(layout, "Remote Path:")

        # Local path input with browse button
        local_layout = QHBoxLayout()
        local_label = QLabel("Local Path:")
        local_label.setFixedWidth(120)
        local_layout.addWidget(local_label)
        self.local_path_input = QLineEdit()
        self.local_path_input.setText(self.settings.value("local_path", os.path.expanduser("~")))  # Default to home directory
        self.local_path_input.setAlignment(Qt.AlignmentFlag.AlignLeft)
        self.local_path_input.setMinimumWidth(300)
        local_layout.addWidget(self.local_path_input)
        browse_button = QPushButton("Browse")
        browse_button.clicked.connect(self.browse_local_path)
        local_layout.addWidget(browse_button)
        layout.addLayout(local_layout)

        # Buttons for actions
        button_layout = QHBoxLayout()
        ssh_button = QPushButton("Check SSH Connection")
        ssh_button.clicked.connect(self.check_ssh_connection)
        button_layout.addWidget(ssh_button)

        list_button = QPushButton("List Remote Directory")
        list_button.clicked.connect(self.list_remote_dir)
        button_layout.addWidget(list_button)

        copy_button = QPushButton("Copy Item")  # Renamed for clarity
        copy_button.clicked.connect(self.copy_file)
        button_layout.addWidget(copy_button)

        layout.addLayout(button_layout)

        # Tree widget to display remote directory content
        self.tree = QTreeWidget()
        self.tree.setHeaderLabels(["Name", "Size", "Type", "Last Modified"])
        self.tree.setColumnWidth(0, 300)
        self.tree.itemDoubleClicked.connect(self.on_item_double_clicked)
        self.tree.itemClicked.connect(self.on_item_clicked)  # Handle single click for selection
        layout.addWidget(QLabel("Remote Directory Content:"))
        layout.addWidget(self.tree)

        # Donate button
        donate_button = QPushButton("Invite me a coffee")
        donate_button.clicked.connect(self.open_donate_page)
        layout.addWidget(donate_button)

        self.central_widget.setLayout(layout)
        self.update_default_path()
        self.user_input.textChanged.connect(self.update_default_path)

    def open_donate_page(self):
        # Open donation page in default web browser
        import webbrowser
        webbrowser.open("https://www.laptrinhdientu.com/2021/10/Donate.html")

    def create_input_field(self, layout, label_text):
        # Helper method to create a labeled input field
        h_layout = QHBoxLayout()
        label = QLabel(label_text)
        label.setFixedWidth(120)
        input_field = QLineEdit()
        input_field.setAlignment(Qt.AlignmentFlag.AlignLeft)
        input_field.setMinimumWidth(300)
        h_layout.addWidget(label)
        h_layout.addWidget(input_field)
        layout.addLayout(h_layout)
        return input_field

    def update_default_path(self):
        # Update remote path to user's home directory if empty or outdated
        user = self.user_input.text()
        current_remote = self.remote_path_input.text()
        if user and (not current_remote or current_remote.startswith("/home/")):
            parts = current_remote.split('/')
            if len(parts) >= 3 and parts[1] == 'home':
                default_path = f"/home/{user}"
                self.remote_path_input.setText(default_path)
            elif not current_remote:
                default_path = f"/home/{user}"
                self.remote_path_input.setText(default_path)

    def browse_local_path(self):
        # Open a dialog to select local destination folder
        start_dir = self.local_path_input.text()
        if not os.path.isdir(start_dir):
            start_dir = os.path.expanduser("~")
        folder = QFileDialog.getExistingDirectory(self, "Select Destination Folder", start_dir)
        if folder:
            self.local_path_input.setText(folder)
            self.settings.setValue("local_path", folder)

    def check_ssh_connection(self):
        # Check SSH connection to the remote server
        remote_user = self.user_input.text()
        remote_host = self.host_input.text()
        port = self.port_input.text() or "22"

        if not remote_user or not remote_host:
            QMessageBox.warning(self, "Input Error", "Please enter Username and IP/Host.")
            return

        progress = QProgressDialog("Checking SSH connection...", None, 0, 0, self)
        progress.setWindowTitle("Connecting")
        progress.setWindowModality(Qt.WindowModality.WindowModal)
        progress.setMinimumDuration(500)
        progress.show()
        QApplication.processEvents()

        cmd_list = [
            'ssh', '-p', port,
            '-o', 'StrictHostKeyChecking=no',
            '-o', 'BatchMode=yes',
            f'{remote_user}@{remote_host}',
            'exit 0'
        ]

        try:
            print(f"Executing: {' '.join(cmd_list)}")  # Debug output
            result = subprocess.run(cmd_list, check=True, timeout=10, capture_output=True, text=True,
                                    creationflags=CREATE_NO_WINDOW)  # Hide console on Windows
            progress.close()
            QMessageBox.information(self, "Success", "SSH connection successful!")
            self.list_remote_dir()  # Auto-list directory on success
        except subprocess.TimeoutExpired:
            progress.close()
            QMessageBox.critical(self, "Error", "Connection timed out after 10 seconds.")
        except FileNotFoundError:
            progress.close()
            QMessageBox.critical(self, "Error", "Error: 'ssh' command not found.")
        except subprocess.CalledProcessError as e:
            progress.close()
            stderr_msg = e.stderr.strip() if e.stderr else "Unknown SSH error."
            if "Permission denied" in stderr_msg:
                error_detail = "Permission denied. Check credentials or server config."
            elif "Connection refused" in stderr_msg:
                error_detail = "Connection refused. Check host and port."
            else:
                error_detail = f"SSH Error (Exit Code {e.returncode}):\n{stderr_msg}"
            QMessageBox.critical(self, "Error", error_detail)
        except Exception as e:
            progress.close()
            traceback.print_exc()
            QMessageBox.critical(self, "Error", f"Unexpected error: {str(e)}")

    def list_remote_dir(self, show_message=True):
        # List contents of the remote directory using SSH
        remote_user = self.user_input.text()
        remote_host = self.host_input.text()
        port = self.port_input.text() or "22"
        remote_path = self.remote_path_input.text()

        if not all([remote_user, remote_host, remote_path]):
            QMessageBox.warning(self, "Input Error", "Please fill in all required fields.")
            return

        progress = QProgressDialog("Listing remote directory...", None, 0, 0, self)
        progress.setWindowTitle("Listing Files")
        progress.setWindowModality(Qt.WindowModality.WindowModal)
        progress.setMinimumDuration(500)
        progress.show()
        QApplication.processEvents()

        cmd_list = [
            'ssh', '-p', port,
            '-o', 'StrictHostKeyChecking=no',
            '-o', 'BatchMode=yes',
            f'{remote_user}@{remote_host}',
            'ls', '-lA', remote_path
        ]

        try:
            print(f"Executing: {' '.join(cmd_list)}")  # Debug output
            result = subprocess.run(cmd_list, check=True, capture_output=True, text=True, timeout=15,
                                    creationflags=CREATE_NO_WINDOW)  # Hide console on Windows
            progress.close()
            self.parse_ls_output(result.stdout, remote_path)
            # Update the tree's current listed path
            self.tree.setProperty("current_listed_path", remote_path)
        except subprocess.TimeoutExpired:
            progress.close()
            self.tree.clear()
            QMessageBox.critical(self, "Error", "Listing timed out after 15 seconds.")
        except FileNotFoundError:
            progress.close()
            self.tree.clear()
            QMessageBox.critical(self, "Error", "Error: 'ssh' command not found.")
        except subprocess.CalledProcessError as e:
            progress.close()
            self.tree.clear()
            stderr_msg = e.stderr.strip() if e.stderr else "Unknown error."
            if "No such file or directory" in stderr_msg:
                error_detail = f"Remote path '{remote_path}' not found."
            else:
                error_detail = f"Listing Error (Exit Code {e.returncode}):\n{stderr_msg}"
            QMessageBox.critical(self, "Error", error_detail)
        except Exception as e:
            progress.close()
            self.tree.clear()
            traceback.print_exc()
            QMessageBox.critical(self, "Error", f"Unexpected error: {str(e)}")

    def parse_ls_output(self, output, current_path):
        # Parse the output of 'ls -lA' and populate the tree widget
        self.tree.clear()
        lines = output.strip().split("\n")
        if lines and lines[0].startswith("total"):
            lines = lines[1:]  # Skip 'total' line

        user_home = f"/home/{self.user_input.text()}"
        if current_path != "/" and current_path != user_home:
            parent_item = QTreeWidgetItem(self.tree)
            parent_item.setText(0, "..")
            parent_item.setText(2, "Parent Directory")
            parent_item.setData(0, Qt.ItemDataRole.UserRole, "..")

        for line in lines:
            parts = line.split(maxsplit=8)
            if len(parts) < 9:
                continue
            permissions, size, date = parts[0], parts[4], f"{parts[5]} {parts[6]}"
            name = parts[8]
            item_type = "Symbolic Link" if permissions.startswith("l") else \
                        "Directory" if permissions.startswith("d") else "File"
            if name in [".", ".."]:
                continue

            item = QTreeWidgetItem(self.tree)
            item.setText(0, name)
            item.setText(1, size)
            item.setText(2, item_type)
            item.setText(3, date)
            item.setData(0, Qt.ItemDataRole.UserRole, item_type)

    def on_item_double_clicked(self, item, column):
        # Handle double-click to navigate directories
        item_name = item.text(0)
        item_type_data = item.data(0, Qt.ItemDataRole.UserRole)
        # Use the currently listed path as the base (fix for duplicate path issue)
        current_path = self.tree.property("current_listed_path") or self.remote_path_input.text().rstrip('/')

        if item_type_data == "Parent Directory" or item_name == "..":
            if current_path == "/":
                return
            new_path = os.path.dirname(current_path) or "/"
        elif item_type_data == "Directory":
            new_path = f"/{item_name}" if current_path == "/" else f"{current_path}/{item_name}"
        else:
            return

        self.remote_path_input.setText(new_path)
        self.list_remote_dir(show_message=False)

    def on_item_clicked(self, item, column):
        # Update remote_path_input with the clicked item's full path
        item_name = item.text(0)
        item_type_data = item.data(0, Qt.ItemDataRole.UserRole)
        current_listed_path = self.tree.property("current_listed_path") or self.remote_path_input.text().rstrip('/')

        if item_type_data == "Parent Directory" or item_name == "..":
            if current_listed_path != "/":
                new_path = os.path.dirname(current_listed_path) or "/"
            else:
                new_path = "/"
        elif item_type_data in ["Directory", "File", "Symbolic Link"]:
            new_path = f"/{item_name}" if current_listed_path == "/" else f"{current_listed_path}/{item_name}"
        else:
            return

        self.remote_path_input.setText(new_path)

    def copy_file(self):
        # Copy or synchronize item using rsync
        remote_user = self.user_input.text()
        remote_host = self.host_input.text()
        port = self.port_input.text() or "22"
        remote_path = self.remote_path_input.text()
        local_dest_folder = self.local_path_input.text()

        if not all([remote_user, remote_host, remote_path, local_dest_folder]):
            QMessageBox.warning(self, "Input Error", "Please fill in all fields!")
            return

        # Check and create local destination folder if it doesn't exist
        if not os.path.isdir(local_dest_folder):
            reply = QMessageBox.question(self, "Local Path Issue",
                                         f"'{local_dest_folder}' does not exist. Create it?",
                                         QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
            if reply == QMessageBox.StandardButton.Yes:
                os.makedirs(local_dest_folder, exist_ok=True)
            else:
                return

        remote_item_name = os.path.basename(remote_path.rstrip('/'))
        progress = QProgressDialog(f"Synchronizing '{remote_item_name}'...", "Cancel", 0, 0, self)
        progress.setWindowModality(Qt.WindowModality.WindowModal)
        progress.setMinimumDuration(0)
        progress.show()
        QApplication.processEvents()

        # Build rsync command as a single string with quotes around remote path
        ssh_command = f"ssh -p {port}"
        remote_arg = f'"{remote_user}@{remote_host}:{remote_path}"'  # Add quotes around remote path
        cmd = f'rsync -az -e "{ssh_command}" {remote_arg} "{local_dest_folder}"'

        try:
            # Debug: Print the exact command being executed
            print(f"Executing rsync command: {cmd}")
            result = subprocess.run(
                cmd,
                shell=True,  # Use shell to interpret the command string
                capture_output=True,
                text=True,
                timeout=3600,
                creationflags=CREATE_NO_WINDOW  # Hide console on Windows
            )
            progress.close()

            # Check rsync result
            if result.returncode == 0:
                QMessageBox.information(self, "Success", f"Synchronization complete for '{remote_item_name}'.")
            else:
                stderr_msg = result.stderr.strip() if result.stderr else "Unknown error."
                error_detail = f"rsync failed with exit code {result.returncode}:\n{stderr_msg}"
                QMessageBox.critical(self, "rsync Error", error_detail)

        except subprocess.TimeoutExpired:
            progress.close()
            QMessageBox.critical(self, "Error", "rsync timed out after 60 minutes.")
        except FileNotFoundError:
            progress.close()
            QMessageBox.critical(self, "Error", "Error: 'rsync' command not found.")
        except Exception as e:
            progress.close()
            traceback.print_exc()
            QMessageBox.critical(self, "Error", f"Unexpected error: {str(e)}")

    def closeEvent(self, event):
        # Save settings when closing the window
        self.settings.setValue("username", self.user_input.text())
        self.settings.setValue("host", self.host_input.text())
        self.settings.setValue("port", self.port_input.text())
        self.settings.setValue("local_path", self.local_path_input.text())
        event.accept()

def main():
    app = QApplication(sys.argv)
    window = FileCopyTool()
    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
