# rtl_snr
Catalog and analyze transmissions from devices recorded in rtl_433 JSON logs

## Function
rtl_snr ("snr") is a program for cataloging and characterizing ISM-band devices in your neighborhood.  The snr program processes rtl_433 JSON log files to:

* read the recorded packet information,
* count the packets and consolidate redundant packets into an individual transmission, ignoring redundant packets sent to increase reliability,
* catalog all devices recorded in that log, and
* summarize the statistics about their signal-to-noise ratios (SNR).  Sample output looks like this:
```
Processed 20000 Packets as 7045 De-Duplicated Transmissions
Dated from Thu 2022-06-09 07:08:27 to Thu 2022-06-09 19:46:16

Device                      #Pkts    #Xmits  Mean SNR ± 𝜎    Min    Max
Acurite-01185M 0                4         4    9.6 ±  4.9    6.4   16.9
Acurite-606TX 134             858       858    8.4 ±  2.1    5.5   20.0
Acurite-609TXC 194           8006      1446   19.2 ±  0.5   12.4   21.2
Acurite-Tower 11524          8203      2753   19.2 ±  0.5   13.2   20.8
Hyundai-VDO 60b87768            1         1   11.0 ±  0.0   11.0   11.0
Hyundai-VDO aeba4a98            1         1    7.2 ±  0.0    7.2    7.2
LaCrosse-TX141Bv3 253         597       348    8.2 ±  1.1    5.7   11.5
LaCrosse-TX141THBv2 168      1536       840    9.6 ±  1.1    6.0   19.2
Markisol 0                     75        75   19.2 ±  0.9   12.3   20.2
Markisol 256                   20        20   19.3 ±  0.4   18.5   20.2
Prologue-TH 203               699       699   11.6 ±  1.3    7.2   19.5
```

## Details

`snr` is distributed as two versions: a C version (*snr*) and a Python version (*SNR.py*).  SNR.py requires Python 3.10.3+ version since it uses *classes*.
They generate the same output.  The only differences are that *snr* has the option of selecting the date-time ranges of records to be processed while *SNR.py* does not, and processing times.

The program(s) read the JSON log file created by rtl_433 (advise to stop rtl_433 so that the JSON log file is closed for processing). The observed devices, as recorded in the JSON file in temporal order, are cataloged in alphabetical order in a summary table along with a count of the number of de-duplicated transmissions seen for that device and with the basic signal-to-noise ratio statistics for that device (mean snr, std deviation, min value seen, max value seen). 

A burst of data transmitted by a device is called a "transmission".  A transmission may be made up of multiple separate data blocks: "packets". If the packets contain the same data (for reliability) they're called "repeats".  These SNR programs assume that packets from the same device within 2 seconds of each other were repeated for reliability, and only the data from the first packet in each transmission is used for analysis.

The key string for cataloging a device is the concatenation of the JSON 'model' field and 'id' field from the received data record.

"Duplicated" packets are ignored: a packet is considered a duplicate of its predecessor if the concatenated device 'model'+'id' string is repeated within 2 seconds of that predecessor.  Other recorded data (snr, etc.) are *not* compared, and for some devices that may not be desirable.  The code that performs the de-duplication is marked in each program (*snr.c* and *SNR.py*) for easy removal if needed.  The algorithm only considers the immediate predecessor record in the JSON file, not the immediate predecessor record *for that device*, so interleaved data packets from differing devices would result in imperfect de-duplication in high-traffic regions. 

*snr* runs about 5 times faster than the Python version.  A 199MB JSON file with 786K raw records and 236K de-duplicated records required 64 seconds *real* time for processing by *SNR.py* and 14 seconds *real* time for processing by *snr* on a Raspberry Pi-4B running from a USB3-attached SSD.

## Installation
1. Download the distribution from Github.  
2. Connect to the download directory.
3. For the C version, connect to the `c-version` directory and `make` and then `make install`.  Note that this installs the *snr* executable, the *SNR.py* main program, and the *stats_class.py* class definitions into `~/bin`; edit `Makefile`'s definition of `BIN` if you want the code installed elsewhere, or simply execute the programs from the download directory rather than install.
5. Assuming that `~/bin/` is in your path or that you execute from the download directory, you may then process JSON log files.  For example, to process the `xaa.json` file that is distributed with the package, 
   - `snr -f xaa.json`
   or
   - `SNR.py xaa.json` (again, noting that Python 3.10.3+ is required).
6.  Note that issuing the simple command `snr` to the C-based program gets you the (brief) help message with command-line options.

After verifying that snr or SNR.py is functioning correctly with test data, you may want to configure your rtl_433 config file to record data from your own RTL_SDR dongle.  For example, add something like the following to your config file:
```
output json:/var/rtl_433/rtl_433.json
```
(and create and assign ownership to /var/rtl_433/ if necessary) and then restart rtl_433.

## Dependencies
This code uses Eric Raymond's mjson.c library to parse the rtl_433 JSON file and would not have been possible without it: that code is included in this distribution.  One slight modification to Raymond's distributed code was needed to accommodate model values that were sometimes numeric and sometimes quoted strings; that modification is noted in the mjson.c file included in this distribution.

## Known Issues
JSON log times are expected to be in the format "HH:MM:SS", to the nearest second with no fractional part.  Will be corrected to allow fractional seconds.  Fixed in the next release.

C version does not yet provide per-device packet counts. Fixed in the next release.

The first packet from a device during a transmission interval (individually or as the first in a transmission packet) may have a distorted SNR because of a high auto-gain on the RTL_SDR.  Some devics issue just one packet per transmission and others issue 3-6.  No fix anticipated.

If the JSON input file has blocks of null characters (as might happen if rtl_433 is interrupted), a JSON data load error occurs, message issued, and processing is terminated.  Guidance is provided on how to remove null characters from the JSON log file.

## Additional Statistics
The Python code `SNR-ITGT` processes the JSON log files to provide SNR statisics _and_ statistics about the Inter-Transmission Gap Times (ITGT) -- the time-gap between succesive transmissions from an individual device.  That information might be helpful to characterize devices in your neighborhood.  However, when used to process lengthy log files with many spurious devices -- trucks passing through the neighborhood, occasional security alarms, occasional window-shade operations, etc -- the resulting report will be cluttered with many devices that have only a few transmission for which the mean and standard deviation of the ITGT are meaningless.  The program is included because in some cases it may provide useful information for nearby sensor devices that broadcast routinely and reliably.

Invoke it with `python3 SNR-ITGT.py <JSON file name>`.  The report has the same format as `snr`.

## Author
David Todd, hdtodd@gmail.com, 2022.05.  Updated 2023.03.





