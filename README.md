# rtl_snr
Catalog and analyze signals from devices recorded in rtl_433 JSON logs

## Function
rtl_snr ("snr") is a program for cataloging and characterizing ISM-band devices in your local area.  The snr program processes rtl_433 JSON log files to read the recorded packet information, catalog all devices recorded in that log, and summarize the statistics about their signal-to-noise ratios.  Sample output looks like this:
```
Processed 7045 de-duplicated records
Dated from Thu 2022-06-09 07:08:27 to Thu 2022-06-09 19:46:16

Device                      #Recs  Mean SNR ± 𝜎    Min    Max
Acurite-606TX 134             858    8.4 ±  2.1    5.5   20.0
Acurite-609TXC 194           1446   19.2 ±  0.5   12.4   21.2
Acurite-Tower 11524          2753   19.2 ±  0.5   13.2   20.8
Hyundai-VDO 60b87768            1   11.0 ±  0.0   11.0   11.0
LaCrosse-TX141THBv2 168       840    9.6 ±  1.1    6.0   19.2
Markisol 0                     75   19.2 ±  0.9   12.3   20.2
Prologue-TH 203               699   11.6 ±  1.3    7.2   19.5
...
```

## Details

`snr` is distributed as two versions: a C version (*snr*) and a Python version (*SNR.py*).  SNR.py requires Python 3.10.3+ version since it uses *classes*.
They generate the same output.  The only differences are that *snr* has the option of selecting the date-time ranges of records to be processed while *SNR.py* does not, and processing times.

The program(s) read the JSON log file created by rtl_433 (advise to stop rtl_433 so that the JSON log file is closed for processing). The observed devices, as recorded in the JSON file in temporal order, are cataloged in alphabetical order in a summary table along with a count of the number of de-duplicated records seen for that device and with the basic signal-to-noise ratio statistics for that device (mean snr, std deviation, min value seen, max value seen). 

"Duplicated" records are ignored: a record is considered a duplicate of its predecessor if the device name + model id are repeated within 2 seconds of that predecessor.  Other recorded data (snr, etc.) are *not* compared, and for some devices that may not be desirable.  The code that performs the de-duplication is marked in each program (*snr.c* and *SNR.py*) for easy removal if needed.  The algorithm only considers the immediate predecessor record in the JSON file, not the immediate predecessor record *for that device*, so interleaved data packets from differing devices would result in imperfect de-duplication in high-traffic regions. 

*snr* runs about 5 times faster than the Python version.  A 199MB JSON file with 786K raw records and 236K de-duplicated records required 64 seconds *real* time for processing by *SNR.py* and 14 seconds *real* time for processing by *snr* on a Raspberry Pi-4B running from a USB3-attached SSD.

## Installation
1. Download the distribution from Github.  
2. Connect to the download directory.
3. `make`
4. `make install`.  Note that this installs the *snr* executable, the *SNR.py* main program, and the *stats_class.py* class definitions into `~/bin`; edit `Makefile`'s definition of `BIN` if you want the code installed elsewhere, or simply execute the programs from the download directory.
5. Assuming that `~/bin/` is in your path, you may then process JSON log files.  For example, to process the `xaa.json` file that is distributed with the package, 
   - `snr -f xaa.json`
   or
   - `SNR.py xaa.json`
6.  Note that issuing the simple command `snr` to the C-based program gets you the (brief) help message with command-line options.

## Dependencies
This code uses Eric Raymond's mjson.c library to parse the rtl_433 JSON file and would not have been possible without it: that code is included in this distribution.  One slight modification to Raymond's distributed code was needed to accommodate model values that were sometimes numeric and sometimes quoted strings; that modification is noted in the mjson.c file included in this distribution.

## Author
David Todd, hdtodd@gmail.com, 2022.05




