# SNR v2.0.0 Release Notes

## What SNR Does

rtl\_433 is a program to decode radio transmissions from devices on the Industrial-Scientific-Medical bands used for remote sensing (mainly for the 433.92 MHz, 868 MHz (SRD), 315 MHz, 345 MHz, and 915 MHz ISM bands).  rtl\_433 identifies the sending device and the data values being communicated and reports them in a variety of possible formats.

SNR is a Python3 program that analyzes the JSON log file from rtl\_433 to catalog the devices seen by rtl\_433 and characterize their radio transmissions.  It can be used to identify remote-sensing devices in your neighborhood and determine the reliability of individual devices as data sources by observing SNR, frequency variations, and reception reliability.

A device "transmission" represents one observation but may contain 1 to 6 or more "packets", and transmissions are frequently initiated by the remote device at approximately 15-second, 30-second, or 60-second intervals.  These are simplex communication devices -- the remote device sends data and receives no acknowledgement from the receiver that it has received the data.  In high-traffic neighborhoods, the signals from the various devices may interfere with each other.  Sending redundant packets increases the probability that a receiving device will successfully receive at least one packet in the transmission.

While SNR v1 focused on just the Signal-to-Noise Ratio of the signals seen at the rtl\_433 RTL-SDR receiver, v2 of SNR summarizes additional characteristics of the received signals from each of the devices observed:

*  Signal-to-Noise Ratio over all packets
*  The gap (in seconds) between successive transmissions by each device over individual transmissions
*  The frequency of the device's signal over all packets
*  The number of packets in each transmission over individual transmissions.

For each of these characteristics, SNR analyzes the individual packets and the grouped transmissions for each of the devices and reports the:

*  Number of packets or transmissions in the sample set
*  Mean value of the characteristic
*  Standard Deviation of the values relative to the mean for the characteristic
*  The minimum value observed
*  The maximum value observed.

## Use

SNR requires Python3 and uses only standard Python libraries.

`usage: SNR [-h] -f FN [-o {SNR,ITGT,Freq,PPT} [{SNR,ITGT,Freq,PPT} ...]] [-x NOISE] [-T] [-v] [-d]`

where

*  `-h` provides expanded help
*  `-f FN` is required (FN=file name)
*  `-o [options]` lists reports you want *omitted* (SNR, ITGT, Freq, and/or PPT)
*  `-x n`, n an integer, excludes from the device report any devices with less than "n" packets observed
*  `-d` is unimplemented (debug).

## Changes from v1

### No update to C version

v2 is available in Python3 only at this time.  

### Additional reporting

v2 adds the following capabilities to the basic signal-to-noise (SNR) reporting of v1:

*  ITGT: The Inter-Transmission Gap Time is the mean time between transmissions for each device
*  Freq:  The mean frequency of the packets observed
*  PPT: The mean number of packets per transmission.

Any or all of these statistics reporting categories can be disabled by command-line switches to simplify the report page.  If all are disabled, SNR simply reports the catalog of all devices seen.

### Modified identifier

The individual devices are now identified by a keyword string composed of 'model'/'channel'/'id' in rtl\_433's nomenclature, consistent with the format used in other programs.

### Ignored transmissions and data

SNR ignores status-report packets.

By default, SNR ignores transmissions from Tire Pressure Monitoring Systems ("TPMS") as these tend to be transient records of passing vehicular traffic.  An optional command-line switch enables inclusion of TPMS devices.

SNR ignores the frequency entries for devices with multiple frequencies.

### Corrections

To group a set of packets into one transmission, SNR uses a "de-duplication" algorithm.  By default, packets observed from a device within a 2-second period are considered to be one transmission.  In v1, packets from another device could break one device's multi-packet transmission into what appeared to be multiple transmissions.  In v2, the packet times are monitored on a per-device basis so that the transmission count is now correct.

### Command-line processing

V2 uses `argparse` to parse the command line and provide the appropriate help for using the progam.

### Integrated code base

The `stats` class, used to compile statistics from the data stream, is now embedded in the SNR code file rather than imported from an external file.

### Performance

SNR v2 processed a 60-day, 412MB file containing 1,547,234 Packets representing 618,091 De-Duplicated Transmissions into the 4-category report in 79.95 sec on a Raspberry Pi-4B whereas SNR v1 processed the same file into just the SNR report in 55.3 sec on the same system.  SNR v2 took 68.1 sec to generate just the SNR report on the same system and file.

### Known issues

Service disruptions may cause rtl\_433 to insert blocks of null characters (0x00) into the log file, and SNR is not programmed to ignore those.  The JSON loader fails when null characters are encountered in the input file, and an error message is issued by SNR to inform and suggest a solution for removing the null characters.  (`sed -e 's/\000//g' oldfile.json > newfile.json`).

## Author
David Todd, hdtodd@gmail.com, 2023.03
