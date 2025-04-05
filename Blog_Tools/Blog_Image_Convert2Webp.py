import tkinter as tk
from tkinter import filedialog, messagebox
import ttkbootstrap as ttk
from PIL import Image, ImageEnhance
import os
from moviepy import VideoFileClip

# Default logo path
default_logo_path = r"D:\STM32\My_BLOG\logoIcon.png"

def resource_path(relative_path):
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath("."), relative_path)

def select_file():
    file_path = filedialog.askopenfilename(filetypes=[("Image Files", "*.png;*.jpg;*.jpeg;*.gif")])
    if file_path:
        entry_file.delete(0, tk.END)
        entry_file.insert(0, file_path)
        # Suggest the output file name (same name as input but with .webp or .webm extension)
        suggested_save_path = os.path.splitext(file_path)[0] + ".webp" if not file_path.lower().endswith(".gif") else os.path.splitext(file_path)[0] + ".webm"
        entry_save.delete(0, tk.END)
        entry_save.insert(0, suggested_save_path)

def select_logo():
    logo_path = filedialog.askopenfilename(filetypes=[("PNG Files", "*.png")], initialdir=default_logo_path)
    if logo_path:
        entry_logo.delete(0, tk.END)
        entry_logo.insert(0, logo_path)

def select_save_location():
    save_path = filedialog.asksaveasfilename(defaultextension=".webp", filetypes=[("WebP Image", "*.webp"), ("WebM Video", "*.webm")])
    if save_path:
        entry_save.delete(0, tk.END)
        entry_save.insert(0, save_path)

def convert_image():
    input_path = entry_file.get()
    logo_path = entry_logo.get()
    output_path = entry_save.get()
    quality = 100 if quality_var.get() == 1 else quality_slider.get()

    # Ensure all paths are selected
    if not input_path or not output_path:
        messagebox.showerror("Error", "Please select image or video, logo, and save location.")
        return

    try:
        if input_path.lower().endswith(".gif"):
            # Convert GIF to WebM
            clip = VideoFileClip(input_path)
            clip.write_videofile(output_path, codec="libvpx", audio=False, threads=4, preset="ultrafast")
            messagebox.showinfo("Success", "GIF converted to WebM successfully!")
        else:
            # Process as image (PNG, JPG)
            img = Image.open(input_path).convert("RGBA")
            logo = Image.open(logo_path).convert("RGBA")

            # Keep the original size of the logo (don't resize)
            img_width, img_height = img.size
            logo_width, logo_height = logo.size

            # Calculate the position to center the logo
            position = ((img_width - logo_width) // 2, (img_height - logo_height) // 2)

            # Adjust logo opacity to 5%
            alpha = logo.split()[3]  # Get the alpha channel
            alpha = ImageEnhance.Brightness(alpha).enhance(0.05)  # Reduce opacity to 5%
            logo.putalpha(alpha)

            # Composite the logo onto the image
            img.paste(logo, position, logo)  # Use the logo as a mask for proper transparency

            # Save the output image as WebP with specified quality
            img.save(output_path, "WEBP", quality=int(quality), lossless=(quality == 100))
            messagebox.showinfo("Success", "Image converted successfully!")
    
    except Exception as e:
        messagebox.showerror("Error", f"Failed to convert file: {e}")

# Main UI setup
root = ttk.Window(themename="cosmo")
root.title("PNG/JPG/GIF to WebP/WebM Converter")
root.geometry("500x550")

# Image file selection
ttk.Label(root, text="Select Image or GIF:").pack(pady=5)
entry_file = ttk.Entry(root, width=40)
entry_file.pack(pady=5)
ttk.Button(root, text="Browse", command=select_file).pack()

# Logo selection with default path
ttk.Label(root, text="Select Logo:").pack(pady=5)
entry_logo = ttk.Entry(root, width=40)
entry_logo.pack(pady=5)
entry_logo.insert(0, default_logo_path)  # Set default logo path
ttk.Button(root, text="Browse", command=select_logo).pack()

# Save location
ttk.Label(root, text="Save As:").pack(pady=5)
entry_save = ttk.Entry(root, width=40)
entry_save.pack(pady=5)
ttk.Button(root, text="Browse", command=select_save_location).pack()

# Quality options
quality_var = tk.IntVar(value=1)
ttk.Radiobutton(root, text="Lossless (Best Quality)", variable=quality_var, value=1).pack()
ttk.Radiobutton(root, text="Lossy (Adjustable)", variable=quality_var, value=0).pack()

quality_slider = ttk.Scale(root, from_=1, to=100, orient="horizontal")
quality_slider.pack()
quality_slider.set(90)  # Default lossy quality

# Convert button
ttk.Button(root, text="Convert", command=convert_image, bootstyle="success").pack(pady=10)

root.mainloop()
