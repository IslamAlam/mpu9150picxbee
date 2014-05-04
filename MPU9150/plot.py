# plot  data, pass in argument, like this:
# %python plot.py X
# or this
# %python plot.py x
# or this
# %python plot.py Y
import sys
import serial
import numpy as np
from matplotlib.lines import Line2D
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from MPU9150 import MPU9150 
from Scope import Scope
#from MPU9150Fig import MPU9150Fig
print('Gathering Info For Scale of Plot, so shake it.')

# sample to get scale
a = MPU9150()	# get the MPU9150 object
samp = [];
for i in range(0,100):
   samp.append(a.get(sys.argv[1]))

# sort the samples from low to hig
samp.sort()
n = len(samp)
minimum = samp[1]
maximum = samp[n-1]
#a.close()
#
#print(samp)
#
fig, ax = plt.subplots()
#fig     = MPU9150Fig(fig)
ax.set_ylim(minimum,maximum)
scope = Scope(ax)
#
# pass a generator in "emitter" to produce data for the update func
# passing a function with an argument does not work
if sys.argv[1]=='X':
   ani = animation.FuncAnimation(fig, scope.update, a.emitX, interval=10, blit=True)
elif sys.argv[1]=='x':
   ani = animation.FuncAnimation(fig, scope.update, a.emitx, interval=10, blit=True)
elif sys.argv[1]=='Y':
   ani = animation.FuncAnimation(fig, scope.update, a.emitY, interval=10, blit=True)
elif sys.argv[1]=='y':
   ani = animation.FuncAnimation(fig, scope.update, a.emity, interval=10, blit=True)
elif sys.argv[1]=='Z':
   ani = animation.FuncAnimation(fig, scope.update, a.emitZ, interval=10, blit=True)
elif sys.argv[1]=='z':
   ani = animation.FuncAnimation(fig, scope.update, a.emitz, interval=10, blit=True)
elif sys.argv[1]=='T':
   ani = animation.FuncAnimation(fig, scope.update, a.emitT, interval=10, blit=True)

plt.show()
