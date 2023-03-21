# rtl\_433\_stats
Catalog and analyze transmissions from devices recorded in rtl_433 JSON logs

**`rtl_433_stats` is a rename of `rtl_snr`:** renamed since the analyses in the Python version have been extended to other device statistics in addition to signal-to-noise ratios.

## Function
`rtl_433_stats` is a program for cataloging and characterizing ISM-band devices in your neighborhood using the JSON log file from `rtl_433`.  The `rtl_433_stats` program processes rtl\_433 JSON log files to:

* read the recorded packet information,
* count the packets and consolidate redundant packets into an individual transmission, 
* catalog all devices recorded in that log, and
* summarize the statistics about their signal-to-noise ratios (SNR), gap times between transmissions (ITGT), radio frequencies (Freq), and packets per transmission (PPT). 
 
 
Sample output looks like this:

```
Processed 20000 Packets as 6952 De-Duplicated Transmissions in  0.24sec 
Packets dated from Thu 2022-06-09 07:08:27 to Thu 2022-06-09 19:46:16

                                         Signal-to-Noise                 Inter-Transmission Gap Time                     Frequency (MHz)                         Packets per Transmit          
Device model/channel/id        _________________________________   _______________________________________   ____________________________________________   ____________________________________
                                #Pkts   Mean ±   𝜎    Min    Max    #Gaps    Mean  ±     𝜎     Min     Max    #Pkts     Mean  ±   𝜎          Min      Max    #Xmits    Mean ±   𝜎     Min    Max
Acurite-01185M/0/0                  4    9.6 ±  4.9   6.4   16.9        3  3678.7s ± 2812.7  434.0  5425.0        4   433.911 ±  0.017   433.902  433.936         3     1.0 ±  0.0      1      1
Acurite-606TX//134                858    8.4 ±  2.1   5.5   20.0      857    53.0s ±  139.2   30.0  2573.0      858   433.901 ±  0.009   433.863  433.962       857     1.0 ±  0.0      1      1
Acurite-609TXC//194              8006   19.3 ±  0.5  12.3   21.2     1356    33.5s ±    0.7   33.0    49.0     8006   433.931 ±  0.002   433.922  433.950      1356     5.9 ±  0.4      2      6
Acurite-Tower/A/11524            8203   19.2 ±  0.5  13.2   20.8     2752    16.5s ±    2.5   15.0    33.0     8203   433.950 ±  0.002   433.926  433.955      2752     3.0 ±  0.2      1      3
LaCrosse-TX141Bv3/1/253           597    8.4 ±  1.3   5.7   19.2      347   109.9s ±  338.9   31.0  4216.0      597   433.904 ±  0.003   433.863  433.945       347     1.7 ±  0.5      1      2
LaCrosse-TX141THBv2/0/168        1536    9.6 ±  1.1   6.0   19.2      837    54.2s ±   14.7   49.0   150.0     1536   433.961 ±  0.004   433.862  433.966       837     1.8 ±  0.4      1      2
Markisol/0/0                       39   19.1 ±  1.2  12.3   20.2       38  1053.5s ± 1532.1   33.0  6633.0       39   433.932 ±  0.002   433.928  433.936        38     1.0 ±  0.0      1      1
Markisol/0/256                     20   19.3 ±  0.4  18.5   20.2       19  2108.7s ± 3625.7   33.0 14070.0       20   433.931 ±  0.002   433.927  433.936        19     1.0 ±  0.0      1      1
Markisol/1/0                       36   19.2 ±  0.5  17.6   20.0       35  1009.8s ± 1550.0   67.0  6801.0       36   433.931 ±  0.002   433.927  433.934        35     1.0 ±  0.0      1      1
Prologue-TH/2/203                 699   11.6 ±  1.3   7.2   19.5      698    64.9s ±   32.7   52.0   477.0      699   433.864 ±  0.008   433.859  433.943       698     1.0 ±  0.0      1      1
```

## Use

Once installed, issue the command `rtl_433_stats -f <JSON filename>` to generate the report.  Issue the command `rtl_433_stats -h` to see command-line options.

## Details

`rtl_433_stats` reads the JSON log file created by rtl\_433 (recommend to stop rtl_433 so that the JSON log file is closed for processing). The observed devices, as recorded in the JSON file in temporal order, are cataloged in alphabetical order in a summary table along with a count of the number of packets and de-duplicated transmissions seen for that device and with basic statistics for that device:

* count of samples,
* mean, 
* std deviation, 
* min value seen, and 
* max value seen

for these device characteristics:

* signal-to-noise ratio
* time between successive transmissions
* radio frequency of transmissions
* packets per transmission.

A burst of data transmitted by a device is called a "transmission".  A transmission may be made up of multiple separate data blocks for redundancy: individual "packets". If the packets contain the same data (for reliability) they're called "repeats".  `rtl_433_stats` assumes that packets from the same device within a default setting of 2 seconds of each other were repeated for reliability and represent one transmission.  SNR and frequency data are averaged over all packets; transmission gap times and packets per transmission are averaged over transmissions (grouped packets).

The key string for cataloging a device is the concatenation of the JSON 'model' field, 'channel' field, and 'id' field from the received data record.

`rtl_433_stats` tracks packet times _per device_ so that transmissions from different devices don't interfere with each other and tranmission statistics are more reliable in high-traffic areas.

## Installation
1. Use git to clone the distribution from Github, [https://github.com/hdtodd/rtl\_433\_stats](https://github.com/hdtodd/rtl_433_stats)  
2. Connect to the download directory, then to its `py-version` subdirectory.
3. Type `python3 rtl_433_stats -f ../xaa.json` and compare its output with the file `xaa-v2.rpt` to ensure that it is functioning correctly.

After verifying that `rtl\_433\_stats`is functioning correctly with test data, you may want to configure your rtl\_433 config file to record data from your own RTL_SDR dongle.  For example, add something like the following to your `rtl_433` config file:
```
output json:/var/rtl_433/rtl_433.json
```
(and create and assign ownership to /var/rtl\_433/ if necessary) and then restart rtl\_433.

## Dependencies
This code uses Eric Raymond's mjson.c library to parse the rtl_433 JSON file and would not have been possible without it: that code is included in this distribution.  One slight modification to Raymond's distributed code was needed to accommodate model values that were sometimes numeric and sometimes quoted strings; that modification is noted in the mjson.c file included in this distribution.

## Known Issues

The first packet from a device during a transmission interval (individually or as the first in a transmission packet) may have a distorted SNR because of a high auto-gain on the receiving RTL\_SDR dongle.  Some devices issue just one packet per transmission and others issue 3-6.  No fix anticipated.

If the JSON input file has blocks of null characters (as might happen if the computer hosting rtl\_433 is interrupted), a JSON data load error occurs, message issued, and processing is terminated.  Guidance is provided on how to remove null characters from the JSON log file.

## Prior work

`rtl_433_stats` was previously distributed as v1.0 of `rtl_snr` in two languages: the original C version (*snr*) and the original Python version (*SNR.py*).  Those programs summarized only signal-to-noise ratio. This distribution includes the C version in case it could be useful, but it has not been updated to the full functionality of the Python version.  However, the C version (*snr*) runs about 5 times faster than the Python version and so may be more desirable for some applications.  

In the C version (*snr.c*), JSON log times are expected to be in the format "HH:MM:SS", to the nearest second with no fractional part.  `rtl_433_stats` accepts either ISO-format or Epoch time in the rtl_433 JSON log files, with integer or fractional seconds in either case.

The C version summarizes information only for the first packet in each transmission and ignores "duplicated" packets.  A packet is considered a duplicate of its predecessor if the concatenated device identifier string is repeated within 2 seconds of that predecessor.  Other recorded data (snr, etc.) are *not* compared, and for some devices that may not be desirable.  The algorithm only considers the immediate predecessor record in the JSON file, not the immediate predecessor record *for that device*, so interleaved data packets from differing devices would result in imperfect de-duplication in high-traffic regions. 

The C version does not provide per-device packet counts. 

To install and use the C version:

1. Connect to the `c-version` directory and `make` and then `make install`.  Note that this installs the *snr* executable into `~/bin`; edit `Makefile`'s definition of `BIN` if you want the code installed elsewhere, or simply execute the programs from the download directory rather than install.
2. Assuming that `~/bin/` is in your path or that you execute from the download directory, you may then process JSON log files.  For example, to process the `xaa.json` file that is distributed with the package, `snr -f ../xaa.json`
3.  Note that issuing the simple command `snr` to the C-based program gets you the (brief) help message with command-line options.

## Additional information

The file SNR-v2-README.md contains the original release notes for SNR.py v2 and provides additional details about the differences in versions and how to use `rtl_433_stats`.

## Author
David Todd, hdtodd@gmail.com, 2022.05.  Updated 2023.03.





