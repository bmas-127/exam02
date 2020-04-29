import matplotlib.pyplot as plt
import numpy as np
import serial
import time

t = np.arange(0, 100)
gravity = np.zeros((100, 3))
tilt = np.zeros((100))

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)


for i in range(0, 100):
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    gravity[i][0] = float(line)
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    gravity[i][1] = float(line)
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    gravity[i][2] = float(line)

    line=s.readline() # Read an echo string from K66F terminated with '\n'
    tilt[i] = int(line)





fig, ax = plt.subplots(2, 1)
ax[0].plot(t,gravity)
ax[1].stem(t, tilt) 
plt.show()


s.close()