import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
#vectors=np.array([[0,0,0,1,1,1]])
vectors = np.array([[0,0,0,0.999511,0.0312449,0.0012],[0,0,0,-0.0312006,0.998093,0.0532576],[0,0,0,-0.00119941,-3.75E-05,0.999999]])

fig = plt.figure()
ax = fig.add_subplot(111,projection='3d')
for vector in vectors:
    v = np.array([vector[3],vector[4],vector[5]])
    vlength = np.linalg.norm(v)
    ax.quiver(vector[0],vector[1],vector[2],vector[3],vector[4],vector[5],pivot='tail',length=vlength,arrow_length_ratio=0.3/vlength)
ax.set_xlim([-1,1])
ax.set_ylim([-1,1])
ax.set_zlim([-1,1])
ax.set_xlabel('x')
ax.set_ylabel('y')
ax.set_zlabel('z')
plt.show()