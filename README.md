# OS_simulator
metadataops.c, h | SimDriver.c | configops.c, h | StringUtils.c, h | simtimer.c, h written by Michael Leverington

All other files written by Andrew Liddell

## DESCRIPTION
This is an operating system simulator that loads both a metadata and config file with appropriate format.
It is entirely written in raw C with a custom string utils header provided by my professor. On execution, 
it will run through the metadata file provided, and will simulate processes depending on the specified scheduling
format. Process cycle rate, IO cycle rate, quantum time, and accessible memory are all specified in the config.
Preemptive scheduling formats are completed with multithreading, and the sim has a functioning MMU.

## SETUP

Make sure that both a config and metadata file are in the same directory, then run:

```
$ make -f sim04_mf
$ ./sim04 -rs config2.cnf
```
To see the full program format, simply run the binary file
