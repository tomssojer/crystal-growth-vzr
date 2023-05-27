import matplotlib.pyplot as plt
from matplotlib.patches import RegularPolygon
import matplotlib.animation as animation
import numpy as np
import math

# Definicije
radij = 2/3
left_shift = 0.88
right_shift = 0.44

count = 1

file_path = "serial_array.txt"
with open(file_path) as file:
    for line in file.readlines():
        array = []

        i = 0
        while i < len(line):
            # Indeks naslednjega elementa
            j = i + 1
            # Vrednost trenutnega elementa
            k = line[i]

            if (line[i] != " ") and (line[i] != "\n"):

                # Dokler naslednji element ni presledek, ga dodajaj k naslednjemu
                while (line[j] != " "):
                    k += line[j]
                    j += 1
                
                array.append(int(k))

            i = j

        array = [array[i:i+3] for i in range(0, len(array), 3)]

        array_length = len(array)

        index_x = 0
        index_y = 0
        for i in range(array_length):
            if i % int(math.sqrt(array_length)) == 0:
                index_x = 0
                index_y += 1
        
            array[i][1] = array[i][1] - left_shift * (index_x + 1) + right_shift * index_y
            index_x += 1

        coord = array

        colors = []
        for c in coord:
            if c[2] == 0:
                colors.append(((1, 1, 1)))
            elif c[2] == 1:
                colors.append((0.5, 0.5, 0.5))
            else:
                colors.append((0.2, 0.2, 0.2))

        hcoord = [c[1] for c in coord]
        vcoord = [c[0] for c in coord]

        fig, ax = plt.subplots(1, figsize=(10, 10))
        ax.set_aspect('equal')
        ax.set_facecolor((0.2, 0.2, 0.2))

        # Add some coloured hexagons
        for x, y, c in zip(hcoord, vcoord, colors):
            color = c
            hex = RegularPolygon((x, y), numVertices=6, radius=radij, 
                                    orientation=np.radians(0), facecolor=color)
            ax.add_patch(hex)
        # Also add scatter points in hexagon centres
        ax.scatter(hcoord, vcoord, c=[c for c in colors])

        plt.savefig(f'img_{count}.png')
        count += 1
