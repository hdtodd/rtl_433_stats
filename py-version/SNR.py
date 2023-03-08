#!/usr/bin/python3
#  SNR.py
# Program to analyze and summarize rtl_433 JSON logs of ISM 433MHz device data

import sys
import json
import time
import class_stats as stats
from json.decoder import JSONDecodeError

# Structure of summary node is
#   {"device",(record-count, running-std^2, running-sum, running-min, running-max)}
#   field labels are (RC,snr-sum,snr-min,snr-max)

global devices

# main program

print("\nSNR.py: Analyze rtl_433 json log files\n")

if len(sys.argv)>=2:
    fn = sys.argv[1]
else:
    fn = input("Path/Name to file to be processed: ")

firstTime = float('inf')
lastTime  = 0
RC = 0
lastDevice = {"time":0.0, "dev":""}

devices = None
devices = {}

print("Processing ISM 433MHz messages from file", fn)

with open(fn,"rt") as log:
    lc = 0                # line count, for error reporting
    for line in log:
        lc += 1
        # unpack the record in JSON
        try:
            y = json.loads(line)
        except JSONDecodeError as e:
            print("JSON decode error at file line ", lc)
            print("Line contents:\n", line)
            err = {}
            print("JSON error msg:", err.get("error", str(e)))
            print("Or are there null characters in your input file?")
            print("Try sed 's/\\x0//g' oldfile > newfile to remove them")
            quit()
        except TypeError as e:
            print("JSON type error in file line", lc)
            print("Line contents:\n", line)
            err = {}
            print("JSON error msg:", err.get("error", str(e)))
            quit()
            
        eTime = time.mktime(time.strptime(y["time"], "%Y-%m-%d %H:%M:%S"))

        #  Statement below makes 'model'+'id' the key for cataloging and summarizing
        #  Change the following statement to experiment with other keys
        dev = y["model"]
        if "id" in y:
            dev += " "+str(y["id"])  #use 'model'+'id' as unique key
        if eTime>lastDevice["time"]+2.0 or dev != lastDevice["dev"]:
        
            # record the basic stats about this entry: increment record count, record time
            lastDevice["dev"]= dev
            lastDevice["time"] = eTime
            RC += 1
            firstTime = eTime if (eTime<firstTime) else firstTime
            lastTime  = eTime if (eTime>lastTime)  else lastTime

            # Now process this record
            snr  = float(y['snr'])
            # EDIT HERE TO Ignore the first record for a device ... autogain may distort SNR
            # If device already seen, append this reading to the stream
            #   else create a new dictionary entry with this as its initial data
            if dev in devices:
                devices[dev].append(snr)
            else:
                devices[dev] = stats.stats(snr)

print("\nProcessed", RC, "de-duplicated records\nDated from",
      time.strftime("%a %Y-%m-%d %H:%M:%S",time.localtime(firstTime)), "to",
      time.strftime("%a %Y-%m-%d %H:%M:%S",time.localtime(lastTime)) )
print()
print("{:<25} {:>7}  {:>8}  {:>5}  {:>5}".format("Device", "#Recs", "Mean SNR ± 𝜎", "Min", "Max"))

for d in sorted(devices):
    (n,avg,std,min,max) = devices[d].get()
    print("{:<25} {:>7} {:>6.1f} ±{:>5.1f}  {:>5.1f}  {:>5.1f}".format(d,n,avg,std,min,max))
