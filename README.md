# rtl_snr
Catalog and analyze signals from devices recorded in rtl_433 JSON logs

## Function
rtl_snr processes rtl_433 JSON log files to catalog all devices recorded in that log and summarize the statistics about their signal-to-noise ratios.  Sample output looks like this:
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
```
