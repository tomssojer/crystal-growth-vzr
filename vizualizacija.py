import matplotlib.pyplot as plt
from matplotlib.patches import RegularPolygon
import numpy as np

#def draw_board(board, occupied):
coord = [[0,0,0],[0,1,0],[0,2,0],[0,3,0],[0,4,0],[0,5,0],[0.89,0.5,0],[0.89,1.5,0],[0.89,2.5,0],[0.89,3.5,0],[0.89,4.5,0],[0.89,5.5,0],[1.78,0,1],[1.78,1,1],[1.78,2,1],[1.78,3,1],[1.78,4,1],[1.78,5,1],[2.67,0.5,1],[2.67,1.5,1],[2.67,2.5,1],[2.67,3.5,1],[2.67,4.5,1],[2.67,5.5,1]]
colors = ["blue" if c[2] >= 1 else "green" for c in coord]

hcoord = [c[0] for c in coord]
vcoord = [c[1] for c in coord]


fig, ax = plt.subplots(1, figsize=(5, 5))
ax.set_aspect('equal')

    # Add some coloured hexagons
for x, y, c in zip(hcoord, vcoord, colors):
    color = c[0].lower()
    hex = RegularPolygon((x, y), numVertices=6, radius=1.75 / 3., 
                            orientation=np.radians(30), facecolor=color, edgecolor='k')
    ax.add_patch(hex)
    # Also add scatter points in hexagon centres
ax.scatter(hcoord, vcoord, c=[c[0].lower() for c in colors])

counter = 1 
plt.show(f'img_{counter}.png')
plt.savefig(f'img_{counter}.png')
counter += 1