import datetime
import time
import os

startTime = datetime.datetime(2023,7,11,9,44,0)
delay =  datetime.timedelta(minutes = 0.5)
freq = 10
txtout = ""
listTimeWork = []
for i in range(freq):
    time = i*delay + startTime
    work = "Sheet1.cml"
    txtout = txtout + str(time)[:19]+","+ work + "\n"
    #listTimeWork.append([time,work])


f= open("TestArrange.csv","w")
f.write(txtout)
f.close()

import shutil
shutil.copy("TestArrange.csv","./cmake-build-debug/TestArrange.csv")
shutil.copy("TestArrange.csv","./cmake-build-release/TestArrange.csv")