# CPU-scheduling-simulator

Author: Nhan Nguyen

Submission code file in in .zip
After copy file into sercver zeus.cs.txstate.edu
Use the following command to unzip and run file

***************************************************
unzip Simulation.zip

g++ -o run_sim main.cpp -std=c++11

./run_sim 1 30 0.04 (0.02)
**************************************************


The last command follow format


./run_sim [type schedule][lambda value][average service time][quantum time]

- Type of schedule will be chosen by number:
	1 : First Come First Serve
	2 : Shortest Time Remain First
	3 : Round Robin

- The average service time should keep constant at 0.04

- Change the value of lambda from 10 to 30 and record value

- With quantum time only need when type of schedule is Round Robin. Otherwise
program needs 3 agrument to run

The output will have format:

Scheduler       lambda  AvgTurnaround   Throughput      CPU Utilization         AvgReadyQ
FCFS            10      0.0659161       9.8566          0.39657         	0.00256822
