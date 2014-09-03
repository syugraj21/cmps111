Project 2 Lottery Scheduler

Date 5-2-14

Group Members
Justin Kwok
Ian Hamilton
Yugraj Singh
Greg Arnheiter
Benjamin Lieu


File Included:
schedule.c
schedproc.h
README.txt
designdoc.pdf
schedule.c (this schedule.c file is from the /usr/src/servsers/pm directory and is located in the pm directory)


Instructions to Run:

1. Overrides files in the following directories: 
  A. path /usr/src/servers/sched
      schedule.c
      schedproc.h

  B. path /usr/src/servers/pm
      schedule.c   

Static/Dynamic: 
- Currently the scheduler is set to Dynamic. To change the scheduler to a static sched, just change the MACRO  defined below in schedule.c (the user scheduler not pm). 

  #define DYNAMIC 1 
  1 = dynamic lottery scheduler implementation
  0 = static lottery scheduler implementation

Compile:
  A. go to path /usr/src/tools
  B. type: make install

Run:
  A. Shutdown minix and logging with the custom minix




