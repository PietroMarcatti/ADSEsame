import tkinter as tk
from tkinter import filedialog
from PIL import Image, ImageTk

# Function to handle mouse clicks
def on_click(event):
    x, y = event.x, event.y
    coordinates.append((x, y))  # Store the coordinate
    canvas.create_oval(x-3, y-3, x+3, y+3, fill="red")  # Mark the point

# Save the coordinates to a file
def save_coordinates():
    with open("coordinates.txt", "w") as file:
        for coord in coordinates:
            file.write(f"{coord[0]}, {coord[1]}\n")
    print("Coordinates saved to coordinates.txt")

# Exit the GUI
def exit_app():
    save_coordinates()  # Save before exiting
    root.destroy()  # Close the window

# Load image and set up Tkinter canvas
root = tk.Tk()
root.title("Board Game Coordinate Marker")
coordinates = []

# Load image
file_path = filedialog.askopenfilename(title="Select an Image", filetypes=[("Image Files", "*.png;*.jpg;*.jpeg;*.bmp")])
if not file_path:
    print("No file selected. Exiting.")
    exit()

# Use Pillow to open and resize the image
image = Image.open(file_path)
original_width, original_height = image.size
max_width, max_height = 1742, 980  # Limit dimensions for the canvas
scale = min(max_width / original_width, max_height / original_height, 1)  # Scale factor
new_width, new_height = int(original_width * scale), int(original_height * scale)
image = image.resize((new_width, new_height))

photo = ImageTk.PhotoImage(image)

canvas = tk.Canvas(root, width=new_width, height=new_height)
canvas.pack()
canvas.create_image(0, 0, anchor=tk.NW, image=photo)

# Bind mouse click and add buttons
canvas.bind("<Button-1>", on_click)

button_frame = tk.Frame(root)
button_frame.pack()

save_button = tk.Button(button_frame, text="Save Coordinates", command=save_coordinates)
save_button.pack(side=tk.LEFT, padx=10)

exit_button = tk.Button(button_frame, text="Exit", command=exit_app)
exit_button.pack(side=tk.LEFT, padx=10)

root.mainloop()


