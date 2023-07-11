import serial
inst=serial.Serial("COM10",115200)
Freq=[1000,1000]  ##kHz unit
Pha=[0,00]               ##deg. unit
Ampl=[1000,1000]  ##mV unit. 50ohm loaded.

buf=f'{Freq[0]*1000:08}'+f'{Pha[0]:03}'+f'{Ampl[0]:04}'+f'{Freq[1]*1000:08}'+f'{Pha[1]:03}'+f'{Ampl[1]:04}'
inst.write(buf.encode())
print('Ch1 Freq=',Freq[0],'kHz, Pha=',Pha[0],'deg., Ampl=',Ampl[0],'mV\nCh2 Freq=',Freq[1],'kHz, Pha=',Pha[1],'deg., Ampl=',Ampl[1],'mV')
buf=inst.read(4)
data1=int(buf)
buf=inst.read(4)
data2=int(buf)
print('Ch1=',data1,'mV\nCh2=',data2,'mV\n')