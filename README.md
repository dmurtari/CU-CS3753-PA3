CSCI 3753: Programming Assignment 3
===================================

By Andy Sayler - 2012
http://www.andysayler.com

With help from:
Junho Ahn - 2012

Updated by Mike Gartrell - 2014
Updated by Domenic Murtari - 2014

Executables
-----------
`./testscript` - A simple bash script for running a few sample test cases  
`./pi` - A simple program for statistically calculating pi  
`./pi-sched` - A simple program for statistically calculating pi using
             a specific scheduling policy  
`./rw0sched` - A simple i/o bound example program using a specific scheduling
             policy.  
`./rr_quantum` - A simple program for determining the RR quantum.  

Examples
--------
Build:
    make

Clean:
    make clean

pi:
    ./pi
    ./pi <Number of Iterations>

pi-sched:
    ./pi-sched
    ./pi-sched <Number of Iterations>
    ./pi-sched <Number of Iterations> <Scheduling Policy>
    ./pi-sched <Number of Iterations> <Scheduling Policy> <Processes to Fork>

rw:
    ./rw
    ./rw <#Bytes to Write to Output File>
    ./rw <#Bytes to Write to Output File> <Block Size>
    ./rw <#Bytes to Write to Output File> <Block Size> <Scheduling Policy>
    ./rw <#Bytes to Write to Output File> <Block Size> <Scheduling Policy> 
        <Processes to Fork>
    ./rw <#Bytes to Write to Output File> <Block Size> <Scheduling Policy> 
        <Processes to Fork> <Input Filename>
    ./rw <#Bytes to Write to Output File> <Block Size> <Scheduling Policy> 
        <Processes to Fork> <Input Filename> <Output Filename>

testscript:
    ./testscript

rr_quantum:
    sudo ./rr_quantum
