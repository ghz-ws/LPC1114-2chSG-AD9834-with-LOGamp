import serial
import time
import numpy
import csv
import matplotlib.pyplot as plt

inst=serial.Serial("COM10",115200)
Start_Freq=1000     ##kHz unit
Stop_Freq=1001     ##kHz unit
Step_Freq=0.1       ##kHz unit
Wait=0.01             ##sec
Ampl=[1000,1000]  ##mV unit. 50ohm loaded.

step=int((Stop_Freq-Start_Freq)/Step_Freq)
data=numpy.zeros((step,3))
for i in range(step):
    freq=Start_Freq+i*Step_Freq
    buf=f'{int(freq*1000):08}'+'000'+f'{Ampl[0]:04}'+f'{int(freq*1000):08}'+'000'+f'{Ampl[1]:04}'
    inst.write(buf.encode())
    time.sleep(Wait)
    buf=inst.read(4)
    data1=int(buf)
    buf=inst.read(4)
    data2=int(buf)
    print('Freq=',int(freq*1000),'Hz, Ch1=',data1,'mV, Ch2=',data2,'mV')
    data[i][0]=int(freq*1000)
    data[i][1]=data1
    data[i][2]=data2
    
with open('data.csv','w',newline="") as f:
    writer=csv.writer(f)
    writer.writerows(data)
    
plt.subplot(1,2,1)
plt.plot(data[:,0],data[:,1])
plt.subplot(1,2,2)
plt.plot(data[:,0],data[:,2])
plt.show()