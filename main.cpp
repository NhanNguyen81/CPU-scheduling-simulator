#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cmath>

#include "init_simulator.h"

using namespace std;


int run_sim()
{
    cout << "Starting simulation using scheduler: ";
    switch (scheduler)
    {
        case _FCFS: cout << "FCFS\n\n"; FCFS(); break;
        case _SRTF: cout << "SRTF\n\n"; SRTF(); break;
        case _RR: cout << "RR\n\n"; RR(); break;
        default: cerr << "invalid scheduler\n"; return 1;
    }
    return 0;
}



void FCFS()
{
    int arrivalCount = 0;
    int departureCount = 0;
    int dispatchCount = 0;

    while (departureCount < MAX_PROCESSES)
    {
        if (!cpu->busy)
        {
            sched_arrival();
            if (readyQ.top() != nullptr) sched_dispatch();
        }
        else
            sched_depart();

        switch (eventQ.top()->type)
        {
            case ARRIVE:
                arrival();
                arrivalCount++;
                break;
            case DISPATCH:
                dispatch();
                dispatchCount++;
                break;
            case DEPARTURE:
                departure();
                departureCount++;
                break;
            default: cerr << "invalid event type\n";
        }
    }
}


void SRTF()
{
    int arrivalCount = 0;
    int departureCount = 0;
    int allocationCount = 0;

    while (departureCount < MAX_PROCESSES)
    {
        if (arrivalCount < (MAX_PROCESSES * 1.20))
        {
            sched_arrival();
            arrivalCount++;
        }
        if (!cpu->busy)
        {
            if (readyQ.top() != nullptr) sched_dispatch();
        }
        else
        {
            if (eventQ.top()->type == ARRIVE)
            {

                if (eventQ.top()->time > estimate_fin_time())
                    sched_depart();
                else if (does_preempt())
                    sched_preempt();
            }
        }

        switch (eventQ.top()->type)
        {
            case ARRIVE:
                arrival();
                break;
            case DISPATCH:
                dispatch();
                allocationCount++;
                break;
            case DEPARTURE:
                departure();
                departureCount++;
                break;
            case PREEMPT:
                preemption();
                break;
            default: cerr << "invalid event type\n";
        }

    }
}

void RR()
{
    int arrivalCount = 0;
    int departureCount = 0;
    while (departureCount < MAX_PROCESSES)
    {
        if (arrivalCount < (MAX_PROCESSES * 1.20))
        {
            sched_arrival();
            arrivalCount++;
        }
        if (!cpu->busy)
        {
            sched_arrival();
            if (readyQ.top() != nullptr) sched_q_dispatch();
        }
        else
        {
            float estCompletionTime = 0;
            estCompletionTime = cpu->p_link->reStartTime == 0 ?
                    cpu->p_link->remainingTime : cpu->p_link->startTime;

            if (estCompletionTime < get_next_q_clock())
                sched_q_depart();
            else
            {
                if (readyQ.top() != nullptr)
                {
                    if (readyQ.top()->p_link->arrivalTime > estCompletionTime)
                        sched_q_depart();
                    else
                        sched_q_preempt();
                }
            }
        }
        switch (eventQ.top()->type)
        {
            case ARRIVE:
                arrival();
                break;
            case DISPATCH:
                q_dispatch();
                break;
            case DEPARTURE:
                q_depart();
                departureCount++;
                if (readyQ.top() != nullptr && (readyQ.top()->p_link->arrivalTime < cpu->clock))
                    sched_q_dispatch();
                break;
            case PREEMPT:
                q_preempt();
                break;
            default: cerr << "invalid event type\n";
        }
    }
    }



int main(int argc, char *argv[] )
{
    parseArgs(argc, argv);
    init();
    run_sim();
    print_report();
    return 0;
}


