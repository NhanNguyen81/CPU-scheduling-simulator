#ifndef INIT_SIMULATOR_H
#define INIT_SIMULATOR_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include "event.h"
using namespace std;

#define MAX_PROCESSES 10000

enum Scheduler {_FCFS = 1, _SRTF = 2, _RR = 3};

struct process
{
    int pid;
    float arrivalTime;
    float startTime;
    float reStartTime;
    float finishTime;
    float burst;
    float remainingTime;
    struct process *pl_next;
};

struct CPU
{
    float clock;
    bool busy;
    struct process *p_link;
};

struct Ready
{
    struct process *p_link;
    struct Ready *rq_next;
};

class ReadyQueue
{ public:
    ReadyQueue() { rq_head = rq_tail = nullptr; }
    Ready* top();
    void pop();
    void push(Ready *);
    bool check_empty();
    process *get_srt();
    Ready* rq_head, *rq_tail;
};

Ready* ReadyQueue::top() { return rq_head; }

void ReadyQueue::pop()
{
    Ready *tempPtr = rq_head;
    rq_head = rq_head->rq_next;
    delete tempPtr;
}

bool ReadyQueue::check_empty() { return rq_head == nullptr; }

void ReadyQueue::push(Ready *newReady)
{
    if (rq_head == nullptr)  //empty queue
        rq_head = newReady;
    else
    {
        Ready *rq_cursor = rq_head;
        while (rq_cursor->rq_next != nullptr)
            rq_cursor = rq_cursor->rq_next;
        rq_cursor->rq_next = newReady;
    }
}

process* ReadyQueue::get_srt()
{
    Ready *rq_cursor = rq_head;
    process *srtProc = rq_cursor->p_link;
    float srt = rq_cursor->p_link->remainingTime;
    while (rq_cursor != nullptr)
    {
        if (rq_cursor->p_link->remainingTime < srt)
        {
            srt = rq_cursor->p_link->remainingTime;
            srtProc = rq_cursor->p_link;
        }
        rq_cursor = rq_cursor->rq_next;
    }
    return srtProc;
}

Scheduler scheduler;

float totalTurnaroundTime;
float completionTime;
float cpuBusyTime;
float totalWaitingTime;
float avgServiceTime = 0.0;
float quantum;
float quantumClock;
int lambda;

EventQueue eventQ;
ReadyQueue readyQ;
event *eq_head;
process *pl_head;
process *pl_tail;
Ready *rq_head;
CPU *cpu;

void FCFS();
void SRTF();
void RR();

void parseArgs(int, char *[]);
static void show_usage();
void init();
int run_sim();
void generate_report();
float urand();
float genexp(float);
float estimate_fin_time();


void sched_arrival();
void arrival();
void sched_dispatch();
void dispatch();
void sched_depart();

bool does_preempt();
void sched_preempt();
void preempt();

void sched_q_dispatch();
void q_dispatch();
void sched_q_depart();
void q_depart();
void sched_q_preempt();
void q_preempt();
float get_next_q_dispatch();
float get_next_q_clock();

void parseArgs(int argc, char *argv[])
{
    scheduler = static_cast<Scheduler>(stoi(argv[1]));
    lambda = atoi(argv[2]);
    avgServiceTime = (float)atof(argv[3]);
    if (argc == 5) quantum = (float)atof(argv[4]);
}

void init()
{
    cout << "lambda: " << lambda << endl;

    quantumClock = 0.0;
    avgServiceTime = (float)1.0/avgServiceTime;

    cpu = new CPU;
    cpu->clock = 0.0;
    cpu->busy = false;
    cpu->p_link = nullptr;

    pl_head = new process;
    pl_head->pid = 0;
    pl_head->arrivalTime = genexp((float)lambda);
    pl_head->startTime = 0.0;
    pl_head->reStartTime = 0.0;
    pl_head->finishTime = 0.0;
    pl_head->burst = genexp(avgServiceTime);
    pl_head->remainingTime = pl_head->burst;
    pl_head->pl_next = nullptr;
    pl_tail = pl_head;

    eq_head = new event;
    eq_head->time = pl_head->arrivalTime;
    eq_head->type = ARRIVE;
    eq_head->eq_next = nullptr;
    eq_head->p_link = pl_head;

    readyQ = ReadyQueue();
    eventQ = EventQueue();
    eventQ.push(eq_head);
}

void print_report()
{
    float avgTurnaroundTime = totalTurnaroundTime / (float)MAX_PROCESSES;
    float totalThroughput = (float)MAX_PROCESSES / completionTime;
    float cpuUtil = cpuBusyTime / completionTime;
    float avgWaitingTime = totalWaitingTime / (float)MAX_PROCESSES;
    float avgQlength = 1.0/(float)lambda * avgWaitingTime;

    string _scheduler;
    switch (scheduler) {
        case _FCFS: _scheduler = "FCFS"; break;
        case _SRTF: _scheduler = "SRTF"; break;
        case _RR: _scheduler = "RR"; break;
    }
    cout << "Scheduler\t"       << "lambda\t"   << "AvgTurnaround\t"        << "Throughput\t"   << "CPU Utilization\t"  << "\tAvgReadyQ\n"
         << _scheduler<<"\t\t"    << lambda<<"\t" << avgTurnaroundTime<<"\t\t"  <<totalThroughput   <<"\t\t"<<cpuUtil<<"\t\t"   <<avgQlength << endl;
}

float urand() { return (float) rand() / RAND_MAX; }


float genexp(float lambda)
{
    float u,x;
    x = 0;
    while (x == 0)
    {
        u = urand();
        x = (-1/lambda)*log(u);
    }
    return(x);
}

void sched_arrival()
{
    process *pl_cursor = pl_tail;
    pl_cursor->pl_next = new process;
    pl_cursor->pl_next->pid = pl_cursor->pid + 1;
    pl_cursor->pl_next->arrivalTime = pl_cursor->arrivalTime + genexp((float)lambda);
    pl_cursor->pl_next->startTime = 0.0;
    pl_cursor->pl_next->reStartTime = 0.0;
    pl_cursor->pl_next->finishTime = 0.0;
    pl_cursor->pl_next->burst = genexp(avgServiceTime);
    pl_cursor->pl_next->remainingTime = pl_cursor->pl_next->burst;
    pl_cursor->pl_next->pl_next = nullptr;
    pl_tail = pl_tail->pl_next;

    event *arrival = new event;
    arrival->type = ARRIVE;
    arrival->time = pl_cursor->pl_next->arrivalTime;
    arrival->p_link = pl_cursor->pl_next;
    arrival->eq_next = nullptr;

    eventQ.push(arrival);
}

void arrival()
{
    Ready *ready = new Ready;
    ready->p_link = eventQ.top()->p_link;   //link process at head of event queue to ready queue
    ready->rq_next = nullptr;
    readyQ.push(ready);
    eventQ.pop();
}

void sched_dispatch()
{
    event *dispatch = new event;
    process *nextProc;
    if (scheduler == _FCFS)
        nextProc = readyQ.top()->p_link;
    else if (scheduler == _SRTF)
    {
        if (cpu->clock > readyQ.top()->p_link->arrivalTime)
            nextProc = readyQ.get_srt();
        else
            nextProc = readyQ.top()->p_link;
    }

    dispatch->time = cpu->clock < nextProc->arrivalTime ?
            nextProc->arrivalTime : cpu->clock;

    dispatch->type = DISPATCH;
    dispatch->eq_next = nullptr;
    dispatch->p_link = nextProc;

    eventQ.push(dispatch);
}

void dispatch()
{

    cpu->p_link = eventQ.top()->p_link;

    if (scheduler == _SRTF || scheduler == _RR)
    {
        Ready *rq_cursor = readyQ.top()->rq_next;
        Ready *rq_precursor = readyQ.top();
        if (rq_precursor->p_link->arrivalTime != eventQ.top()->p_link->arrivalTime)
        {
            while (rq_cursor != nullptr)
            {
                if (rq_cursor->p_link->arrivalTime == eventQ.top()->p_link->arrivalTime)
                {
                    rq_precursor->rq_next = rq_cursor->rq_next;
                    rq_cursor->rq_next = readyQ.top();
                    readyQ.rq_head = rq_cursor;
                    break;
                }
                rq_cursor = rq_cursor->rq_next;
                rq_precursor = rq_precursor->rq_next;
            }
        }
    }

    readyQ.pop();
    eventQ.pop();

    cpu->busy = true;

    if (cpu->clock < cpu->p_link->arrivalTime)
        cpu->clock = cpu->p_link->arrivalTime;

    if (cpu->p_link->startTime == 0)
        cpu->p_link->startTime = cpu->clock;
    else
        cpu->p_link->reStartTime = cpu->clock;
}


void sched_depart()
{
    event *departure = new event;
    departure->type = DEPARTURE;
    departure->eq_next = nullptr;
    departure->p_link = cpu->p_link;

    if (scheduler == _FCFS || scheduler == _RR)
        departure->time = cpu->p_link->startTime + cpu->p_link->remainingTime;

    else if (scheduler == _SRTF)
    {
        if (cpu->p_link->reStartTime == 0)
            departure->time = cpu->p_link->startTime + cpu->p_link->remainingTime;
        else
            departure->time = cpu->p_link->reStartTime + cpu->p_link->remainingTime;
    }

    eventQ.push(departure);
}

void departure()
{
    cpu->clock = eventQ.top()->time;
    cpu->p_link->finishTime = cpu->clock;

    cpu->p_link->remainingTime = 0.0;

    cpuBusyTime += cpu->p_link->burst;
    totalTurnaroundTime += (cpu->p_link->finishTime - cpu->p_link->arrivalTime);
    completionTime = cpu->p_link->finishTime;
    totalWaitingTime += (cpu->p_link->finishTime - cpu->p_link->arrivalTime - cpu->p_link->burst);

    cpu->p_link = nullptr;
    cpu->busy = false;

    eventQ.pop();
}


bool does_preempt()
{
    float cpu_fin_time = estimate_fin_time();
    float cpu_remaining_time = cpu_fin_time - eventQ.top()->time;

    return (eventQ.top()->time < cpu_fin_time) && (eventQ.top()->p_link->remainingTime < cpu_remaining_time);
}

void sched_preempt()
{
    event *preemption = new event;
    preemption->time = eventQ.top()->time;
    preemption->type = PREEMPT;
    preemption->eq_next = nullptr;
    preemption->p_link = eventQ.top()->p_link;

    eventQ.pop();
    eventQ.push(preemption);
}

void preemption()
{
    process *preempt_process = cpu->p_link;

    cpu->p_link->remainingTime =
            estimate_fin_time() - eventQ.top()->time;

    cpu->p_link = eventQ.top()->p_link;
    cpu->clock = eventQ.top()->time;
    if (cpu->p_link->reStartTime == 0.0)
        cpu->p_link->startTime = eventQ.top()->time;
    else
        cpu->p_link->reStartTime = eventQ.top()->time;

    event *preempt_arrival = new event;
    preempt_arrival->time = eventQ.top()->time;
    preempt_arrival->type = ARRIVE;
    preempt_arrival->eq_next = nullptr;
    preempt_arrival->p_link = preempt_process;

    eventQ.pop();
    eventQ.push(preempt_arrival);
}

void sched_q_dispatch()
{
    event *dispatch = new event;
    process *nextProc;
    nextProc = readyQ.top()->p_link;

    if (readyQ.top() != nullptr)
    {
        if (readyQ.top()->p_link->arrivalTime < cpu->clock)
            dispatch->time = cpu->clock;
        else
        {
            cpu->clock = readyQ.top()->p_link->arrivalTime;
            float nextQuantumTime = quantumClock;
            while (nextQuantumTime < cpu->clock)
                nextQuantumTime += quantum;
            quantumClock = nextQuantumTime;

            dispatch->time = get_next_q_dispatch();
        }
    }
    else cerr << "Error in sched_q_dispatch()\n";

    dispatch->type = DISPATCH;
    dispatch->eq_next = nullptr;
    dispatch->p_link = nextProc;

    eventQ.push(dispatch);
}

void q_dispatch()
{
    cpu->p_link = eventQ.top()->p_link;
    cpu->busy = true;

    if (cpu->p_link->startTime == 0)
        cpu->p_link->startTime = eventQ.top()->time;
    else
        cpu->p_link->reStartTime = eventQ.top()->time;

    readyQ.pop();
    eventQ.pop();
}


void sched_q_depart()
{
    event *departure = new event;
    departure->type = DEPARTURE;
    departure->eq_next = nullptr;
    departure->p_link = cpu->p_link;

    if (cpu->p_link->reStartTime == 0)
        departure->time = cpu->p_link->startTime + cpu->p_link->remainingTime;
    else
        departure->time = cpu->p_link->reStartTime + cpu->p_link->remainingTime;

    eventQ.push(departure);
}

void q_depart()
{
    cpu->p_link->finishTime = eventQ.top()->time;
    cpu->p_link->remainingTime = 0.0;
    cpu->clock = eventQ.top()->time;
    cpu->busy = false;
    cpuBusyTime += cpu->p_link->burst;
    totalTurnaroundTime += (cpu->p_link->finishTime - cpu->p_link->arrivalTime);
    completionTime = cpu->p_link->finishTime;
    totalWaitingTime += (cpu->p_link->finishTime - cpu->p_link->arrivalTime - cpu->p_link->burst);

    cpu->p_link = nullptr;
    eventQ.pop();
}


void sched_q_preempt()
{
    event *preemption = new event;
    preemption->type = PREEMPT;
    preemption->eq_next = nullptr;

    cpu->clock = readyQ.top()->p_link->arrivalTime;

    float nextQuantumTime = quantumClock;
    while (nextQuantumTime < cpu->clock)
        nextQuantumTime += quantum;

    quantumClock = nextQuantumTime;
    preemption->time = get_next_q_clock();
    preemption->p_link = readyQ.top()->p_link;
    eventQ.push(preemption);
}


void q_preempt()
{
    process *preempted_pr = cpu->p_link;

    cpu->p_link->remainingTime = estimate_fin_time() - eventQ.top()->time;

    cpu->p_link = eventQ.top()->p_link;
    cpu->clock = eventQ.top()->time;

    cpu->p_link->startTime == 0.0 ? cpu->p_link->startTime : cpu->p_link->reStartTime = eventQ.top()->time;

    float nextQuantumTime = quantumClock;
    while (nextQuantumTime < eventQ.top()->time)
        nextQuantumTime += quantum;

    quantumClock = nextQuantumTime;

    event *preempted_p_arrival = new event;
    preempted_p_arrival->time = eventQ.top()->time;
    preempted_p_arrival->type = ARRIVE;
    preempted_p_arrival->eq_next = nullptr;
    preempted_p_arrival->p_link = preempted_pr;

    eventQ.pop();
    readyQ.pop();
    eventQ.push(preempted_p_arrival);

}

float get_next_q_clock(){ return quantumClock + quantum; }

float get_next_q_dispatch()
{
    float nextQuantumTime = quantumClock;
    while (nextQuantumTime < readyQ.top()->p_link->arrivalTime)
        nextQuantumTime += quantum;

    return nextQuantumTime;
}

float estimate_fin_time()
{
    float est_fin_time;
    float start = cpu->p_link->startTime;
    float reStart = cpu->p_link->reStartTime;
    float remaining = cpu->p_link->remainingTime;

    est_fin_time = (reStart == 0 ? start : reStart) + remaining;
    return est_fin_time;
}

#endif
