#################################################################################
'''
    @file    : Taa_RemoteCopyTool.py
    @project : Copy File Tools
    @brief   : Tool to copy files/folders between 2 computers on the same network
    @author  : Nghia Taarabt
'''
#################################################################################
import sys
import subprocess
import os
import time
import hashlib
import threading
import queue
import concurrent.futures
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QLabel, QLineEdit, QPushButton,
                             QTreeWidget, QTreeWidgetItem, QMessageBox, QFileDialog,
                             QProgressDialog, QProgressBar, QDialog, QSpinBox,
                             QCheckBox)
from PyQt6.QtCore import Qt, QSettings, QThread, pyqtSignal, QMutex, QMetaObject, Q_ARG, QObject, QSize
from PyQt6.QtGui import QIcon
import traceback  # For detailed error logging

# Define flag to hide console on Windows
if sys.platform == "win32":
    CREATE_NO_WINDOW = 0x08000000  # Windows-specific flag to hide console
else:
    CREATE_NO_WINDOW = 0  # No effect on non-Windows systems

class SafeConnector(QObject):
    """Helper class to safely update UI from worker threads"""
    update_progress_signal = pyqtSignal(QObject, int, str, str)
    finished_signal = pyqtSignal(QObject, bool, str)
    
    def __init__(self):
        super().__init__()
        self.update_progress_signal.connect(self._update_progress_slot)
        self.finished_signal.connect(self._finished_slot)
    
    def update_progress(self, dialog, value, status, detail=""):
        """Safely update progress dialog from any thread"""
        self.update_progress_signal.emit(dialog, value, status, detail)
    
    def finished(self, dialog, success, message):
        """Safely handle completion from any thread"""
        self.finished_signal.emit(dialog, success, message)
    
    def _update_progress_slot(self, dialog, value, status, detail):
        """Slot that runs in the main thread"""
        try:
            if dialog and dialog.isVisible():
                dialog.update_progress(value, status, detail)
        except Exception as e:
            print(f"Error updating progress: {str(e)}")
    
    def _finished_slot(self, dialog, success, message):
        """Slot that runs in the main thread"""
        try:
            if dialog and dialog.isVisible():
                dialog.accept()  # Close the dialog
                
            # Show message box after dialog is closed
            if success:
                QMessageBox.information(None, "Success", message)
            else:
                QMessageBox.critical(None, "Error", message)
        except Exception as e:
            print(f"Error in finished slot: {str(e)}")


class FileTransferWorker(QThread):
    """Worker thread for file transfers to avoid UI freezing"""
    def __init__(self, remote_user, remote_host, port, remote_path, local_path, 
                 connector, dialog, use_checksum=True, skip_unchanged=True, chunk_size=5_000_000):
        super().__init__()
        self.remote_user = remote_user
        self.remote_host = remote_host
        self.port = port
        self.remote_path = remote_path
        self.local_path = local_path
        self.connector = connector
        self.dialog = dialog
        self.use_checksum = use_checksum
        self.skip_unchanged = skip_unchanged
        self.chunk_size = chunk_size
        self.is_cancelled = False
    
    def run(self):
        try:
            # Get file size
            size_cmd = [
                'ssh', '-p', self.port,
                '-o', 'StrictHostKeyChecking=no',
                '-o', 'BatchMode=yes',
                f'{self.remote_user}@{self.remote_host}',
                f'stat -c %s "{self.remote_path}"'
            ]
            
            self.connector.update_progress(self.dialog, 0, "Getting file information...", "")
            
            try:
                size_result = subprocess.run(
                    size_cmd, capture_output=True, text=True, timeout=10,
                    creationflags=CREATE_NO_WINDOW
                )
                
                if size_result.returncode != 0:
                    self.connector.finished(self.dialog, False, f"Failed to get file size: {size_result.stderr}")
                    return
                    
                total_size = int(size_result.stdout.strip())
            except Exception as e:
                self.connector.finished(self.dialog, False, f"Error getting file size: {str(e)}")
                return
            
            # Create destination directory if it doesn't exist
            try:
                os.makedirs(os.path.dirname(self.local_path), exist_ok=True)
            except Exception as e:
                self.connector.finished(self.dialog, False, f"Error creating directory: {str(e)}")
                return
            
            # Check if file exists and has same checksum (if enabled)
            if os.path.exists(self.local_path) and self.use_checksum:
                self.connector.update_progress(self.dialog, 0, "Checking if file has changed...", "")
                try:
                    if self._check_file_checksum():
                        if self.skip_unchanged:
                            self.connector.finished(self.dialog, True, "File already exists with matching checksum. Skipped copying.")
                            return
                        else:
                            self.connector.update_progress(self.dialog, 0, "File unchanged, but copying anyway...", "")
                except Exception as e:
                    print(f"Checksum check error: {str(e)}")
                    # Continue with copy even if checksum check fails
            
            # For large files, use chunked transfer
            if total_size > 10_000_000:  # 10MB threshold
                try:
                    self._copy_large_file(total_size)
                except Exception as e:
                    self.connector.finished(self.dialog, False, f"Error during chunked copy: {str(e)}")
            else:
                # For smaller files, use standard SCP
                self.connector.update_progress(self.dialog, 0, "Copying file...", "")
                try:
                    success, message = self._copy_file_with_scp()
                    self.connector.finished(self.dialog, success, message)
                except Exception as e:
                    self.connector.finished(self.dialog, False, f"Error during copy: {str(e)}")
        
        except Exception as e:
            self.connector.finished(self.dialog, False, f"Unexpected error: {str(e)}")
    
    def _check_file_checksum(self):
        """Compare MD5 checksum of remote and local files"""
        try:
            # Get remote checksum
            remote_cmd = [
                'ssh', '-p', self.port,
                '-o', 'StrictHostKeyChecking=no',
                '-o', 'BatchMode=yes',
                f'{self.remote_user}@{self.remote_host}',
                f'md5sum "{self.remote_path}" | cut -d" " -f1'
            ]
            
            remote_result = subprocess.run(
                remote_cmd, capture_output=True, text=True, timeout=30,
                creationflags=CREATE_NO_WINDOW
            )
            
            if remote_result.returncode != 0:
                return False
                
            remote_md5 = remote_result.stdout.strip()
            
            # Calculate local checksum
            local_md5 = hashlib.md5()
            with open(self.local_path, 'rb') as f:
                for chunk in iter(lambda: f.read(4096), b''):
                    if self.is_cancelled:
                        return False
                    local_md5.update(chunk)
                    
            return local_md5.hexdigest() == remote_md5
        except Exception as e:
            print(f"Checksum error: {str(e)}")
            return False
    
    def _copy_large_file(self, total_size):
        """Copy large files by splitting them into chunks"""
        chunks = (total_size + self.chunk_size - 1) // self.chunk_size
        
        # Create destination directory if it doesn't exist
        os.makedirs(os.path.dirname(self.local_path), exist_ok=True)
        
        # Create destination file
        with open(self.local_path, 'wb') as out_file:
            for i in range(chunks):
                if self.is_cancelled:
                    self.connector.finished(self.dialog, False, "Transfer cancelled")
                    return
                    
                start = i * self.chunk_size
                end = min((i + 1) * self.chunk_size - 1, total_size - 1)
                
                progress_percent = int((i / chunks) * 100)
                self.connector.update_progress(
                    self.dialog, 
                    progress_percent, 
                    f"Copying chunk {i+1}/{chunks}", 
                    f"{self._format_size(start)}-{self._format_size(end)} of {self._format_size(total_size)}"
                )
                
                # Get chunk data from remote
                dd_cmd = [
                    'ssh', '-p', self.port,
                    '-o', 'StrictHostKeyChecking=no',
                    '-o', 'BatchMode=yes',
                    f'{self.remote_user}@{self.remote_host}',
                    f'dd if="{self.remote_path}" bs=1 skip={start} count={end-start+1} status=none'
                ]
                
                try:
                    chunk_result = subprocess.run(
                        dd_cmd, capture_output=True, check=True, timeout=300,
                        creationflags=CREATE_NO_WINDOW
                    )
                    
                    out_file.write(chunk_result.stdout)
                except subprocess.SubprocessError as e:
                    self.connector.finished(self.dialog, False, f"Error during chunk transfer: {str(e)}")
                    return
        
        # Verify checksum after transfer if enabled
        if self.use_checksum:
            self.connector.update_progress(self.dialog, 95, "Verifying file integrity...", "")
            try:
                if not self._check_file_checksum():
                    self.connector.finished(self.dialog, False, "Checksum verification failed after transfer")
                    return
            except Exception as e:
                self.connector.finished(self.dialog, False, f"Checksum verification error: {str(e)}")
                return
        
        self.connector.finished(self.dialog, True, "Successfully copied file")
    
    def _copy_file_with_scp(self):
        """Copy a file using standard SCP"""
        try:
            # Create destination directory if it doesn't exist
            os.makedirs(os.path.dirname(self.local_path), exist_ok=True)
            
            scp_cmd = [
                'scp', '-P', self.port,
                '-o', 'StrictHostKeyChecking=no',
                '-o', 'BatchMode=yes',
                f'{self.remote_user}@{self.remote_host}:"{self.remote_path}"',
                self.local_path
            ]
            
            result = subprocess.run(
                scp_cmd, capture_output=True, text=True, timeout=3600,
                creationflags=CREATE_NO_WINDOW
            )
            
            if result.returncode == 0:
                # Verify checksum after transfer if enabled
                if self.use_checksum:
                    self.connector.update_progress(self.dialog, 95, "Verifying file integrity...", "")
                    if not self._check_file_checksum():
                        return False, "Checksum verification failed after transfer"
                return True, "Successfully copied file"
            else:
                return False, f"SCP failed: {result.stderr}"
        except Exception as e:
            return False, f"Error: {str(e)}"
    
    def cancel(self):
        """Cancel the transfer"""
        self.is_cancelled = True
    
    def _format_size(self, size_bytes):
        """Format bytes to human-readable size"""
        for unit in ['B', 'KB', 'MB', 'GB']:
            if size_bytes < 1024 or unit == 'GB':
                return f"{size_bytes:.2f} {unit}"
            size_bytes /= 1024


class DirectoryCopyWorker(QThread):
    """Worker thread for directory copying"""
    def __init__(self, remote_user, remote_host, port, remote_path, local_dest_folder,
                 connector, dialog, max_workers=4, use_checksum=True, skip_unchanged=True):
        super().__init__()
        self.remote_user = remote_user
        self.remote_host = remote_host
        self.port = port
        self.remote_path = remote_path
        self.local_dest_folder = local_dest_folder
        self.connector = connector
        self.dialog = dialog
        self.max_workers = max_workers
        self.use_checksum = use_checksum
        self.skip_unchanged = skip_unchanged
        self.is_cancelled = False
        self.mutex = QMutex()
        self.completed_files = 0
        self.failed_files = 0
        self.skipped_files = 0
        self.total_files = 0
    
    def run(self):
        try:
            remote_item_name = os.path.basename(self.remote_path)
            local_dest_path = os.path.join(self.local_dest_folder, remote_item_name)
            
            # Create the base destination directory
            try:
                os.makedirs(local_dest_path, exist_ok=True)
            except Exception as e:
                self.connector.finished(self.dialog, False, f"Error creating destination directory: {str(e)}")
                return
            
            # Get list of files in remote directory using a simpler, more reliable approach
            self.connector.update_progress(self.dialog, 0, f"Listing files in '{remote_item_name}'...", "")
            
            # First, create a temporary script to list files
            list_script = f"""
    cd "{self.remote_path}" || exit 1
    find . -type f -print0 | xargs -0 -I{{}} echo "{{}}"
    """
            
            list_cmd = [
                'ssh', '-p', self.port,
                '-o', 'StrictHostKeyChecking=no',
                '-o', 'BatchMode=yes',
                f'{self.remote_user}@{self.remote_host}',
                list_script
            ]
            
            try:
                result = subprocess.run(
                    list_cmd, capture_output=True, text=True, timeout=60,
                    creationflags=CREATE_NO_WINDOW
                )
                
                if result.returncode != 0:
                    error_msg = f"Failed to list remote files: {result.stderr}"
                    print(error_msg)
                    self.connector.finished(self.dialog, False, error_msg)
                    return
                    
                # Process the output to get relative file paths
                files = []
                for line in result.stdout.strip().split('\n'):
                    line = line.strip()
                    if line and line != '.':
                        # Remove leading './' if present
                        if line.startswith('./'):
                            line = line[2:]
                        if line:  # Only add non-empty lines
                            files.append(line)
                
                self.total_files = len(files)
                
                if self.total_files == 0:
                    self.connector.finished(self.dialog, True, "No files to copy")
                    return
                    
                print(f"Found {self.total_files} files to copy")
            except Exception as e:
                error_msg = f"Error listing files: {str(e)}"
                print(error_msg)
                traceback.print_exc()
                self.connector.finished(self.dialog, False, error_msg)
                return
            
            # Create directory structure
            self.connector.update_progress(self.dialog, 0, "Creating directory structure...", "")
            
            try:
                # Create a temporary script to list directories
                dir_script = f"""
    cd "{self.remote_path}" || exit 1
    find . -type d -print0 | xargs -0 -I{{}} echo "{{}}"
    """
                
                dir_cmd = [
                    'ssh', '-p', self.port,
                    '-o', 'StrictHostKeyChecking=no',
                    '-o', 'BatchMode=yes',
                    f'{self.remote_user}@{self.remote_host}',
                    dir_script
                ]
                
                dir_result = subprocess.run(
                    dir_cmd, capture_output=True, text=True, timeout=60,
                    creationflags=CREATE_NO_WINDOW
                )
                
                if dir_result.returncode == 0:
                    dirs = []
                    for line in dir_result.stdout.strip().split('\n'):
                        line = line.strip()
                        if line and line != '.':
                            # Remove leading './' if present
                            if line.startswith('./'):
                                line = line[2:]
                            if line:  # Only add non-empty lines
                                dirs.append(line)
                    
                    # Create all directories first
                    for d in dirs:
                        if self.is_cancelled:
                            self.connector.finished(self.dialog, False, "Operation cancelled")
                            return
                        dir_path = os.path.join(local_dest_path, d)
                        try:
                            os.makedirs(dir_path, exist_ok=True)
                            print(f"Created directory: {dir_path}")
                        except Exception as e:
                            print(f"Error creating directory {dir_path}: {str(e)}")
            except Exception as e:
                error_msg = f"Error creating directory structure: {str(e)}"
                print(error_msg)
                traceback.print_exc()
                self.connector.finished(self.dialog, False, error_msg)
                return
            
            # Start copying files
            self.connector.update_progress(
                self.dialog, 0, 
                f"Starting copy of {self.total_files} files with {self.max_workers} workers...",
                ""
            )
            
            # Use a direct approach for copying each file
            for i, rel_path in enumerate(files):
                if self.is_cancelled:
                    self.connector.finished(self.dialog, False, "Operation cancelled")
                    return
                    
                remote_file_path = os.path.join(self.remote_path, rel_path).replace('\\', '/')
                local_file_path = os.path.join(local_dest_path, rel_path)
                
                # Make sure the directory exists
                try:
                    os.makedirs(os.path.dirname(local_file_path), exist_ok=True)
                except Exception as e:
                    print(f"Error creating directory for {local_file_path}: {str(e)}")
                    self.failed_files += 1
                    continue
                    
                # Copy the file using scp
                try:
                    # Check if file exists and has same checksum
                    if os.path.exists(local_file_path) and self.use_checksum:
                        if self._check_file_checksum(remote_file_path, local_file_path):
                            if self.skip_unchanged:
                                self.skipped_files += 1
                                progress = (i + 1) / self.total_files
                                status = f"Processed {i + 1}/{self.total_files} files"
                                detail = f"Copied: {self.completed_files}, Skipped: {self.skipped_files}, Failed: {self.failed_files}"
                                self.connector.update_progress(self.dialog, int(progress * 100), status, detail)
                                continue
                    
                    # Normalize paths for SCP
                    remote_file_path_norm = remote_file_path.replace('\\', '/')
                    
                    print(f"Copying file {i+1}/{self.total_files}: {remote_file_path} -> {local_file_path}")
                    
                    # On Windows, we need to handle paths differently
                    if sys.platform == "win32":
                        # Create a temporary batch file to run the SCP command
                        import tempfile
                        
                        # Create a batch file with the SCP command
                        with tempfile.NamedTemporaryFile(suffix='.bat', delete=False, mode='w') as batch_file:
                            # Use the raw SCP command without subprocess's argument splitting
                            batch_file.write(f'scp -P {self.port} -o StrictHostKeyChecking=no -o BatchMode=yes "{self.remote_user}@{self.remote_host}:{remote_file_path_norm}" "{local_file_path}"\n')
                            batch_file_path = batch_file.name
                        
                        try:
                            # Execute the batch file
                            print(f"Executing batch file: {batch_file_path}")
                            result = subprocess.run(
                                [batch_file_path], 
                                shell=True,
                                capture_output=True, 
                                text=True, 
                                timeout=3600,
                                creationflags=CREATE_NO_WINDOW
                            )
                            
                            # Clean up the batch file
                            try:
                                os.unlink(batch_file_path)
                            except:
                                pass
                            
                            if result.returncode == 0:
                                # Verify checksum after transfer if enabled
                                if self.use_checksum and not self._check_file_checksum(remote_file_path, local_file_path):
                                    print(f"Checksum verification failed for {remote_file_path}")
                                    self.failed_files += 1
                                else:
                                    self.completed_files += 1
                            else:
                                print(f"SCP failed for {remote_file_path}: {result.stderr}")
                                self.failed_files += 1
                        except Exception as e:
                            print(f"Error executing batch file: {str(e)}")
                            self.failed_files += 1
                    else:
                        # For non-Windows systems, use the standard approach
                        scp_cmd = [
                            'scp', '-P', self.port,
                            '-o', 'StrictHostKeyChecking=no',
                            '-o', 'BatchMode=yes',
                            f'{self.remote_user}@{self.remote_host}:{remote_file_path_norm}',
                            local_file_path
                        ]
                        
                        print(f"SCP command: {' '.join(scp_cmd)}")
                        
                        result = subprocess.run(
                            scp_cmd, capture_output=True, text=True, timeout=3600,
                            creationflags=CREATE_NO_WINDOW
                        )
                        
                        if result.returncode == 0:
                            # Verify checksum after transfer if enabled
                            if self.use_checksum and not self._check_file_checksum(remote_file_path, local_file_path):
                                print(f"Checksum verification failed for {remote_file_path}")
                                self.failed_files += 1
                            else:
                                self.completed_files += 1
                        else:
                            print(f"SCP failed for {remote_file_path}: {result.stderr}")
                            self.failed_files += 1
                except Exception as e:
                    print(f"Error copying {remote_file_path}: {str(e)}")
                    traceback.print_exc()
                    self.failed_files += 1
                    
                # Update progress
                progress = (i + 1) / self.total_files
                status = f"Processed {i + 1}/{self.total_files} files"
                detail = f"Copied: {self.completed_files}, Skipped: {self.skipped_files}, Failed: {self.failed_files}"
                self.connector.update_progress(self.dialog, int(progress * 100), status, detail)
            
            # Final callback
            if self.is_cancelled:
                self.connector.finished(self.dialog, False, "Operation cancelled")
            elif self.failed_files == 0:
                if self.skipped_files > 0:
                    self.connector.finished(
                        self.dialog, True, 
                        f"Copy complete: {self.completed_files} copied, {self.skipped_files} unchanged (skipped)"
                    )
                else:
                    self.connector.finished(self.dialog, True, f"Successfully copied {self.completed_files} files")
            else:
                self.connector.finished(
                    self.dialog, False, 
                    f"Completed with errors: {self.completed_files} copied, {self.skipped_files} skipped, {self.failed_files} failed"
                )
        
        except Exception as e:
            error_msg = f"Unexpected error: {str(e)}"
            print(error_msg)
            traceback.print_exc()
            self.connector.finished(self.dialog, False, error_msg)

    def _worker_thread(self, file_queue, local_dest_path):
        """Worker thread function for copying files"""
        while not self.is_cancelled:
            try:
                # Get next file from queue with timeout
                try:
                    rel_path = file_queue.get(block=True, timeout=0.5)
                except queue.Empty:
                    break
                
                # Copy the file
                remote_file_path = f"{self.remote_path}/{rel_path}"
                local_file_path = os.path.join(local_dest_path, rel_path)
                
                # Make sure the directory exists
                try:
                    os.makedirs(os.path.dirname(local_file_path), exist_ok=True)
                except Exception as e:
                    print(f"Error creating directory for {local_file_path}: {str(e)}")
                    self.mutex.lock()
                    self.failed_files += 1
                    self.mutex.unlock()
                    file_queue.task_done()
                    continue
                
                result = self._copy_single_file(remote_file_path, local_file_path)
                
                # Update counters
                self.mutex.lock()
                if result == "copied":
                    self.completed_files += 1
                elif result == "skipped":
                    self.skipped_files += 1
                else:  # failed
                    self.failed_files += 1
                
                # Update progress
                progress = (self.completed_files + self.skipped_files + self.failed_files) / self.total_files
                status = f"Processed {self.completed_files + self.skipped_files + self.failed_files}/{self.total_files} files"
                detail = f"Copied: {self.completed_files}, Skipped: {self.skipped_files}, Failed: {self.failed_files}"
                self.mutex.unlock()
                
                # Update UI safely
                self.connector.update_progress(self.dialog, int(progress * 100), status, detail)
                
                # Mark task as done
                file_queue.task_done()
            
            except Exception as e:
                print(f"Worker thread error: {str(e)}")
                self.mutex.lock()
                self.failed_files += 1
                self.mutex.unlock()
                # Make sure to mark the task as done even if there's an error
                try:
                    file_queue.task_done()
                except:
                    pass
    
    def _copy_single_file(self, remote_path, local_path):
        """Copy a single file and return result status: 'copied', 'skipped', or 'failed'"""
        try:
            # Check if file exists and has same checksum
            if os.path.exists(local_path) and self.use_checksum:
                if self._check_file_checksum(remote_path, local_path):
                    if self.skip_unchanged:
                        return "skipped"
            
            # Ensure the directory exists
            os.makedirs(os.path.dirname(local_path), exist_ok=True)
            
            # Normalize paths for SCP
            remote_path_norm = remote_path.replace('\\', '/')
            local_path_norm = local_path.replace('\\', '/')
            
            # On Windows, SCP requires forward slashes
            if sys.platform == "win32":
                local_path_norm = local_path_norm.replace('\\', '/')
            
            # For Windows paths that include drive letters (e.g., D:/path), we need special handling
            if sys.platform == "win32" and ':' in local_path_norm:
                # Use the raw string format for the local path
                scp_cmd = [
                    'scp', '-P', self.port,
                    '-o', 'StrictHostKeyChecking=no',
                    '-o', 'BatchMode=yes',
                    f'{self.remote_user}@{self.remote_host}:"{remote_path_norm}"',
                    f"{local_path_norm}"  # No quotes for Windows paths with drive letters
                ]
            else:
                scp_cmd = [
                    'scp', '-P', self.port,
                    '-o', 'StrictHostKeyChecking=no',
                    '-o', 'BatchMode=yes',
                    f'{self.remote_user}@{self.remote_host}:"{remote_path_norm}"',
                    f'"{local_path_norm}"'
                ]
            
            print(f"SCP command: {' '.join(scp_cmd)}")  # Debug output
            
            result = subprocess.run(
                scp_cmd, capture_output=True, text=True, timeout=3600,
                creationflags=CREATE_NO_WINDOW
            )
            
            if result.returncode == 0:
                # Verify checksum after transfer if enabled
                if self.use_checksum and not self._check_file_checksum(remote_path, local_path):
                    print(f"Checksum verification failed for {remote_path}")
                    return "failed"
                return "copied"
            else:
                print(f"SCP failed for {remote_path}: {result.stderr}")
                return "failed"
        
        except Exception as e:
            print(f"Copy error for {remote_path}: {str(e)}")
            traceback.print_exc()  # Print full stack trace for debugging
            return "failed"
    
    def _check_file_checksum(self, remote_path, local_path):
        """Compare MD5 checksum of remote and local files"""
        try:
            # Get remote checksum
            remote_cmd = [
                'ssh', '-p', self.port,
                '-o', 'StrictHostKeyChecking=no',
                '-o', 'BatchMode=yes',
                f'{self.remote_user}@{self.remote_host}',
                f'md5sum "{remote_path}" | cut -d" " -f1'
            ]
            
            remote_result = subprocess.run(
                remote_cmd, capture_output=True, text=True, timeout=30,
                creationflags=CREATE_NO_WINDOW
            )
            
            if remote_result.returncode != 0:
                return False
                
            remote_md5 = remote_result.stdout.strip()
            
            # Calculate local checksum
            local_md5 = hashlib.md5()
            with open(local_path, 'rb') as f:
                for chunk in iter(lambda: f.read(4096), b''):
                    if self.is_cancelled:
                        return False
                    local_md5.update(chunk)
                    
            return local_md5.hexdigest() == remote_md5
        except Exception as e:
            print(f"Checksum error: {str(e)}")
            return False
    
    def cancel(self):
        """Cancel all operations"""
        self.is_cancelled = True


class ProgressDialog(QDialog):
    """Custom progress dialog with more detailed information"""
    def __init__(self, title, parent=None):
        super().__init__(parent)
        self.setWindowTitle(title)
        self.setMinimumWidth(400)
        self.setWindowModality(Qt.WindowModality.WindowModal)
        
        layout = QVBoxLayout()
        
        self.status_label = QLabel("Initializing...")
        layout.addWidget(self.status_label)
        
        self.progress_bar = QProgressBar()
        self.progress_bar.setRange(0, 100)
        layout.addWidget(self.progress_bar)
        
        self.detail_label = QLabel("")
        self.detail_label.setWordWrap(True)
        layout.addWidget(self.detail_label)
        
        button_layout = QHBoxLayout()
        self.cancel_button = QPushButton("Cancel")
        self.cancel_button.clicked.connect(self.reject)
        button_layout.addWidget(self.cancel_button)
        
        layout.addLayout(button_layout)
        self.setLayout(layout)
    
    def update_progress(self, value, status="", detail=""):
        """Update progress bar and labels"""
        try:
            self.progress_bar.setValue(value)
            if status:
                self.status_label.setText(status)
            if detail:
                self.detail_label.setText(detail)
        except Exception as e:
            print(f"Error updating progress dialog: {str(e)}")


class SettingsDialog(QDialog):
    """Dialog for configuring copy settings"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Copy Settings")
        self.setMinimumWidth(300)
        
        layout = QVBoxLayout()
        
        # Parallel workers setting
        worker_layout = QHBoxLayout()
        worker_layout.addWidget(QLabel("Parallel Workers:"))
        self.worker_spin = QSpinBox()
        self.worker_spin.setRange(1, 16)
        self.worker_spin.setValue(4)  # Default value
        worker_layout.addWidget(self.worker_spin)
        layout.addLayout(worker_layout)
        
        # Checksum verification
        self.checksum_check = QCheckBox("Enable Checksum Verification")
        self.checksum_check.setChecked(True)
        layout.addWidget(self.checksum_check)
        
        # Skip unchanged files
        self.skip_unchanged_check = QCheckBox("Skip Unchanged Files (faster)")
        self.skip_unchanged_check.setChecked(True)
        layout.addWidget(self.skip_unchanged_check)
        
        # Chunk size for large files
        chunk_layout = QHBoxLayout()
        chunk_layout.addWidget(QLabel("Chunk Size (MB):"))
        self.chunk_spin = QSpinBox()
        self.chunk_spin.setRange(1, 50)
        self.chunk_spin.setValue(5)  # Default 5MB
        chunk_layout.addWidget(self.chunk_spin)
        layout.addLayout(chunk_layout)
        
        # Buttons
        button_layout = QHBoxLayout()
        ok_button = QPushButton("OK")
        ok_button.clicked.connect(self.accept)
        cancel_button = QPushButton("Cancel")
        cancel_button.clicked.connect(self.reject)
        button_layout.addWidget(cancel_button)
        button_layout.addWidget(ok_button)
        layout.addLayout(button_layout)
        
        self.setLayout(layout)
        
        # Load settings
        settings = QSettings("LGEDV", "FileCopyTool")
        self.worker_spin.setValue(int(settings.value("parallel_workers", 4)))
        self.checksum_check.setChecked(settings.value("use_checksum", "true") == "true")
        self.skip_unchanged_check.setChecked(settings.value("skip_unchanged", "true") == "true")
        self.chunk_spin.setValue(int(settings.value("chunk_size_mb", 5)))
    
    def get_settings(self):
        """Return the current settings"""
        return {
            "parallel_workers": self.worker_spin.value(),
            "use_checksum": self.checksum_check.isChecked(),
            "skip_unchanged": self.skip_unchanged_check.isChecked(),
            "chunk_size_mb": self.chunk_spin.value()
        }


class FileCopyTool(QMainWindow):
    def __init__(self):
        super().__init__()
        # Set window title and size
        self.setWindowTitle("Enhanced SCP File Copy Tool")
        self.setGeometry(100, 100, 800, 600)
        # Optional: Uncomment to set a window icon
        # self.setWindowIcon(QIcon("path/to/your/icon.png"))

        # Initialize settings to save user preferences
        self.settings = QSettings("LGEDV", "FileCopyTool")
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        
        # Initialize copy settings
        self.parallel_workers = int(self.settings.value("parallel_workers", 4))
        self.use_checksum = self.settings.value("use_checksum", "true") == "true"
        self.skip_unchanged = self.settings.value("skip_unchanged", "true") == "true"
        self.chunk_size_mb = int(self.settings.value("chunk_size_mb", 5))
        self.last_path = self.settings.value("last_path", "")
        
        # Initialize active workers
        self.active_workers = []
        
        # Create safe connector for thread-safe UI updates
        self.connector = SafeConnector()
        
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
        
        # Remote path input with Home button
        remote_path_layout = QHBoxLayout()
        remote_label = QLabel("Remote Path:")
        remote_label.setFixedWidth(120)
        remote_path_layout.addWidget(remote_label)
        self.remote_path_input = QLineEdit()
        self.remote_path_input.setAlignment(Qt.AlignmentFlag.AlignLeft)
        self.remote_path_input.setMinimumWidth(300)
        if self.last_path:
            self.remote_path_input.setText(self.last_path)
        self.remote_path_input.textChanged.connect(self.save_last_path)
        remote_path_layout.addWidget(self.remote_path_input)
        home_button = QPushButton("Home")
        home_button.clicked.connect(self.reset_to_home)
        remote_path_layout.addWidget(home_button)
        layout.addLayout(remote_path_layout)

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

        copy_button = QPushButton("Copy Item")
        copy_button.clicked.connect(self.copy_file)
        button_layout.addWidget(copy_button)
        
        settings_button = QPushButton("Settings")
        settings_button.clicked.connect(self.show_settings)
        button_layout.addWidget(settings_button)

        layout.addLayout(button_layout)

        # Tree widget to display remote directory content
        self.tree = QTreeWidget()
        self.tree.setHeaderLabels(["Name", "Size", "Type", "Last Modified"])
        self.tree.setColumnWidth(0, 300)
        self.tree.itemDoubleClicked.connect(self.on_item_double_clicked)
        self.tree.itemClicked.connect(self.on_item_clicked)  # Handle single click for selection
        layout.addWidget(QLabel("Remote Directory Content:"))
        layout.addWidget(self.tree)

        # Status bar at the bottom showing copy stats
        self.status_bar = QLabel("Ready")
        layout.addWidget(self.status_bar)

        # Donate button
        donate_button = QPushButton()
        donate_button.setFixedSize(150, 60)
        donate_button.setIcon(QIcon("./bmc.svg"))
        donate_button.setIconSize(QSize(150, 60))
        donate_button.setFlat(True)
        donate_layout = QHBoxLayout()
        donate_layout.addStretch()
        donate_layout.addWidget(donate_button)
        donate_button.clicked.connect(self.open_donate_page)
        layout.addLayout(donate_layout)

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

    def save_last_path(self, path):
        if path:
            self.last_path = path
            self.settings.setValue("last_path", self.last_path)

    def reset_to_home(self):
        username = self.user_input.text()
        default_home = f"/home/{username}" if username else "/home/"
        self.remote_path_input.setText(default_home)

    def update_default_path(self):
        user = self.user_input.text()
        current_remote = self.remote_path_input.text()
        if not current_remote and user:
            default_path = f"/home/{user}"
            self.remote_path_input.setText(default_path)
        elif not current_remote and self.last_path:
            self.remote_path_input.setText(self.last_path)

    def browse_local_path(self):
        # Open a dialog to select local destination folder
        start_dir = self.local_path_input.text()
        if not os.path.isdir(start_dir):
            start_dir = os.path.expanduser("~")
        folder = QFileDialog.getExistingDirectory(self, "Select Destination Folder", start_dir)
        if folder:
            self.local_path_input.setText(folder)
            self.settings.setValue("local_path", folder)

    def show_settings(self):
        """Show settings dialog"""
        dialog = SettingsDialog(self)
        if dialog.exec():
            settings = dialog.get_settings()
            self.parallel_workers = settings["parallel_workers"]
            self.use_checksum = settings["use_checksum"]
            self.skip_unchanged = settings["skip_unchanged"]
            self.chunk_size_mb = settings["chunk_size_mb"]
            
            # Save settings
            self.settings.setValue("parallel_workers", self.parallel_workers)
            self.settings.setValue("use_checksum", "true" if self.use_checksum else "false")
            self.settings.setValue("skip_unchanged", "true" if self.skip_unchanged else "false")
            self.settings.setValue("chunk_size_mb", self.chunk_size_mb)
            
            # Update status bar
            self.update_status_bar()
    
    def update_status_bar(self):
        """Update status bar with current settings"""
        status = f"Workers: {self.parallel_workers} | "
        status += "Checksum: ON | " if self.use_checksum else "Checksum: OFF | "
        status += "Skip Unchanged: ON | " if self.skip_unchanged else "Skip Unchanged: OFF | "
        status += f"Chunk Size: {self.chunk_size_mb}MB"
        self.status_bar.setText(status)

    def check_ssh_connection(self):
        # Check SSH connection to the remote server
        remote_user = self.user_input.text()
        remote_host = self.host_input.text()
        port = self.port_input.text() or "22"

        if not remote_user or not remote_host:
            QMessageBox.warning(self, "Input Error", "Please enter Username and IP/Host.")
            return

        progress = QProgressDialog("Checking SSH connection...", "Cancel", 0, 0, self)
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
            
            # Save successful connection details
            self.settings.setValue("username", remote_user)
            self.settings.setValue("host", remote_host)
            self.settings.setValue("port", port)
            
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
        remote_path = self.normalize_path(self.remote_path_input.text())

        if not all([remote_user, remote_host, remote_path]):
            QMessageBox.warning(self, "Input Error", "Please fill in all required fields.")
            return

        progress = QProgressDialog("Listing remote directory...", "Cancel", 0, 0, self)
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
            parent_item.setData(0, Qt.ItemDataRole.UserRole, "Parent Directory")

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

    def normalize_path(self, path):
        """Normalize path to avoid issues"""
        # Handle backslashes
        path = path.replace('\\', '/')
        # Remove multiple consecutive slashes
        while '//' in path:
            path = path.replace('//', '/')
        # Remove trailing slash if present
        if len(path) > 1 and path.endswith('/'):
            path = path[:-1]
        return path

    def copy_file(self):
        """Start the copy process for a file or directory"""
        # Get current variables
        remote_user = self.user_input.text()
        remote_host = self.host_input.text()
        port = self.port_input.text() or "22"
        remote_path = self.normalize_path(self.remote_path_input.text())
        local_dest_folder = self.local_path_input.text()
        
        # Validate input fields
        if not all([remote_user, remote_host, remote_path, local_dest_folder]):
            QMessageBox.warning(self, "Input Error", "Please fill in all fields!")
            return

        # Basic hostname validation
        import re
        if not re.match(r'^[a-zA-Z0-9.-]+$', remote_host) and not re.match(r'^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$', remote_host):
            QMessageBox.warning(self, "Input Error", "Invalid IP/Host format. Please enter a valid IP address (e.g., 192.168.1.1) or hostname (e.g., example.com).")
            return

        # Validate port
        try:
            port = str(int(port))
            if not (1 <= int(port) <= 65535):
                raise ValueError
        except ValueError:
            QMessageBox.warning(self, "Input Error", "Port must be a number between 1 and 65535.")
            return

        # Check if local destination directory exists
        if not os.path.isdir(local_dest_folder):
            reply = QMessageBox.question(self, "Local Path Issue",
                                      f"'{local_dest_folder}' does not exist. Create it?",
                                      QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
            if reply == QMessageBox.StandardButton.Yes:
                try:
                    os.makedirs(local_dest_folder, exist_ok=True)
                except Exception as e:
                    QMessageBox.critical(self, "Error", f"Failed to create directory: {str(e)}")
                    return
            else:
                return

        remote_item_name = os.path.basename(remote_path)
        progress = QProgressDialog(f"Checking '{remote_item_name}'...", "Cancel", 0, 0, self)
        progress.setWindowTitle("Checking")
        progress.setWindowModality(Qt.WindowModality.WindowModal)
        progress.setMinimumDuration(0)
        progress.show()
        QApplication.processEvents()

        # Check if remote_path is a file or directory
        check_cmd = [
            'ssh', '-p', port,
            '-o', 'StrictHostKeyChecking=no',
            '-o', 'BatchMode=yes',
            f'{remote_user}@{remote_host}',
            f'[ -d "{remote_path}" ] && echo dir || ( [ -f "{remote_path}" ] && echo file || echo error )'
        ]

        try:
            print(f"Executing check command: {' '.join(check_cmd)}")  # Debug
            check_result = subprocess.run(
                check_cmd,
                capture_output=True,
                text=True,
                timeout=10,
                creationflags=CREATE_NO_WINDOW
            )
            result_output = check_result.stdout.strip()
            print(f"Check result stdout: {result_output}")  # Debug
            print(f"Check result stderr: {check_result.stderr}")  # Debug

            if check_result.returncode != 0:
                progress.close()
                stderr_msg = check_result.stderr.strip() if check_result.stderr else "Unknown SSH error."
                QMessageBox.critical(self, "Error", f"Failed to check remote path (Exit Code {check_result.returncode}):\n{stderr_msg}")
                return

            if result_output == "dir":
                item_type = "directory"
            elif result_output == "file":
                item_type = "file"
            else:
                progress.close()
                QMessageBox.critical(self, "Error", f"Remote path '{remote_path}' is neither a file nor a directory, or does not exist.\nDetails: {check_result.stderr}")
                return
                
            progress.close()
            
            # Handle based on item type
            if item_type == "file":
                self._copy_single_file(remote_user, remote_host, port, remote_path, local_dest_folder)
            else:  # directory
                self._copy_directory(remote_user, remote_host, port, remote_path, local_dest_folder)
                
        except subprocess.TimeoutExpired:
            progress.close()
            QMessageBox.critical(self, "Error", "Check timed out after 10 seconds.")
            return
        except Exception as e:
            progress.close()
            traceback.print_exc()
            QMessageBox.critical(self, "Error", f"Failed to check remote path: {str(e)}")
            return

    def _copy_single_file(self, remote_user, remote_host, port, remote_path, local_dest_folder):
        """Copy a single file with progress tracking"""
        remote_item_name = os.path.basename(remote_path)
        local_path = os.path.join(local_dest_folder, remote_item_name)
        
        # Create progress dialog
        progress_dialog = ProgressDialog(f"Copying {remote_item_name}", self)
        progress_dialog.show()
        
        # Create worker thread
        worker = FileTransferWorker(
            remote_user, 
            remote_host, 
            port, 
            remote_path, 
            local_path,
            self.connector,
            progress_dialog,
            use_checksum=self.use_checksum,
            skip_unchanged=self.skip_unchanged,
            chunk_size=self.chunk_size_mb * 1024 * 1024
        )
        
        # Connect cancel button
        progress_dialog.rejected.connect(worker.cancel)
        
        # Start worker
        worker.start()
        self.active_workers.append(worker)
    
    def _copy_directory(self, remote_user, remote_host, port, remote_path, local_dest_folder):
        """Copy a directory with parallel file transfers"""
        remote_item_name = os.path.basename(remote_path)
        
        # Create progress dialog
        progress_dialog = ProgressDialog(f"Copying Directory: {remote_item_name}", self)
        progress_dialog.show()
        
        # Create directory copy worker
        worker = DirectoryCopyWorker(
            remote_user,
            remote_host,
            port,
            remote_path,
            local_dest_folder,
            self.connector,
            progress_dialog,
            max_workers=self.parallel_workers,
            use_checksum=self.use_checksum,
            skip_unchanged=self.skip_unchanged
        )
        
        # Connect cancel button
        progress_dialog.rejected.connect(worker.cancel)
        
        # Start worker
        worker.start()
        self.active_workers.append(worker)
    
    def closeEvent(self, event):
        """Handle application close event"""
        # Cancel any active workers
        for worker in self.active_workers:
            if worker.isRunning():
                worker.cancel()
                worker.wait(100)  # Give a little time to clean up
        
        # Save settings
        self.settings.setValue("username", self.user_input.text())
        self.settings.setValue("host", self.host_input.text())
        self.settings.setValue("port", self.port_input.text())
        self.settings.setValue("local_path", self.local_path_input.text())
        self.settings.setValue("last_path", self.last_path)
        
        super().closeEvent(event)


def main():
    app = QApplication(sys.argv)
    window = FileCopyTool()
    window.update_status_bar()  # Initialize status bar with current settings
    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
