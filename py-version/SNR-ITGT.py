#!/usr/bin/python3
#  SNR-PKT.py
# Program to analyze and summarize rtl_433 JSON logs of ISM 433MHz device data
# This version analyzes both signal-to-noise ratios and
#   inter-transmission gap times.
# A "transmission" is a group of one or more packets broadcast to provide information.
# A "packet" is a group of bits sent to communicate that information.
# More than one packet may be sent per transmission in order to increase reliability.
#
import sys
import json
import time
import class_stats as stats
import datetime
from json.decoder import JSONDecodeError

# Structure of summary node is
#   {"device",(transmission-count, running-std^2, running-sum, running-min, running-max)}
#   field labels are (DDTC,snr-sum,snr-min,snr-max)

global devices

def CnvTime(ts):
    if ts.find("-") > 0:
        try:
            eTime = datetime.datetime.fromisoformat(y['time']).timestamp()
        except ValueError as e:
            err={}
            print("datetime error in input line", lc, "converting time string: ", ts)
            print("datetime  msg:", err.get("error", str(e)))
            quit()
    else:
        try:
            eTime = float(ts)
        except ValueError as e:
            err = {}
            print("Datetime conversion failed on line", lc, "with datetime string", ts)
            print("float() error msg:", err.get("error", str(e)))
            quit()
    return(eTime)

# main program

print("\nSNR-ITGT.py: Analyze rtl_433 json log files")
print("             For signal-to-noise (SNR) statistics and")
print("             Inter-Transmission Gap Time (ITGT) statistics:")
print("             mean, standard deviation, min and max values\n")

if len(sys.argv)>=2:
    fn = sys.argv[1]
else:
    fn = input("Path/Name to file to be processed: ")

firstTime = float('inf')
lastTime  = 0
DDTC = 0
lastDevice = {"time":0.0, "dev":""}

devices = {}
itgts   = {}
ipTimes = {}
pktcount= {}

print("Processing ISM 433MHz messages from file", fn)

with open(fn,"rt") as log:
    lc = 0                # line count, for error reporting
    for line in log:
        lc += 1
        # unpack the rtl_433 JSON log record
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

#        eTime = time.mktime(time.strptime(y["time"], "%Y-%m-%d %H:%M:%S"))
        eTime = CnvTime(y["time"])

        #  Statement below makes 'model'+'id' the key for cataloging and summarizing
        #  Change the following statement to experiment with other keys
        dev = y["model"]
        if "id" in y:
            dev += " "+str(y["id"])  #use 'model'+'id' as unique key
        if dev in pktcount:
            pktcount[dev] += 1
        else:
            pktcount[dev] = 1
        if eTime>lastDevice["time"]+2.0 or dev != lastDevice["dev"]:
        
            # record the basic stats about this entry: increment transmission count, record time
            lastDevice["dev"]= dev
            lastDevice["time"] = eTime
            DDTC += 1
            firstTime = eTime if (eTime<firstTime) else firstTime
            lastTime  = eTime if (eTime>lastTime)  else lastTime

            # Now process this record
            snr  = float(y['snr'])
            # EDIT HERE TO Ignore the first packet for a device ... autogain may distort SNR
            # If device "dev" was already seen, just add this data to the stats
            #   If it's a new device, initiate stats with this data 
            if dev in devices:
                devices[dev].append(snr)
                if dev in itgts:
                    itgts[dev].append(eTime - ipTimes[dev])
                else:
                    itgts[dev] = stats.stats(eTime - ipTimes[dev])
                    ipTimes[dev] = eTime
            else:
                devices[dev] = stats.stats(snr)
            ipTimes[dev] = eTime
            
print("\nProcessed", lc, "Packets as", DDTC, "De-Duplicated Transmissions\nDated from",
      time.strftime("%a %Y-%m-%d %H:%M:%S",time.localtime(firstTime)), "to",
      time.strftime("%a %Y-%m-%d %H:%M:%S",time.localtime(lastTime)) )
print()
print("{:<30}{:^33}         {:^37}".format(" ", "Signal-to-Noise", "Inter-Tranmssion Gap Time"))
print("{:<27} {:>33}    {:>38}".format(" "*25, "_"*40, "_"*38))
print("{:<25} {:>7} {:>7}  {:>8}  {:>5}  {:>5}".format("Device", "#Pkts", "#Xmits"," Mean ±   𝜎", "Min", "Max"), end="")
print("  {:>7}    {:>13}   {:>5}   {:>5}".format("#Xmits", "Mean  ±     𝜎", "Min", "Max"))

for d in sorted(devices):
    (n,avg,std,min,max) = devices[d].get()
    print("{:<25} {:>7} {:>7} {:>6.1f} ±{:>5.1f} {:>5.1f}  {:>5.1f}".format(d,pktcount[d],n,avg,std,min,max), end="")
    if d in itgts:
        (n,avg,std,min,max) = itgts[d].get()
        print("  {:>7} {:>7.1f}s ± {:>6.1f}  {:>5.1f} {:>7.1f}".format(n,avg,std,min,max))
    else:
        print()
