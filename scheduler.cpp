//
//  scheduler.cpp
//  Processor Scheduler
//
//  Created by ELMOOTAZBELLAH ELNOZAHY on 9/13/26.
//

#include <queue>
#include "scheduler.hpp"

// Big cores take queued processes only while more than this many are waiting
static const size_t BIG_CORE_BACKLOG = 50;

static const CPUId_t NUM_CORES = 8;
static const CPUId_t ORDER[NUM_CORES] = {4, 5, 6, 7, 0, 1, 2, 3}; 

typedef enum {ASLEEP, WAKING, BUSY} CoreState_t;

static CoreState_t state[NUM_CORES];
static ProcessId_t running[NUM_CORES];
static PState_t pstate[NUM_CORES];

static std::deque<ProcessId_t> readyQ;
static bool initialized = false;

static bool IsBig(CPUId_t c)        { return c < 4; }
static Time_t P4TickWork(CPUId_t c) { return IsBig(c) ? 200 : 120; } 
static bool AllowedRun(CPUId_t c)       { return !IsBig(c) || readyQ.size() > BIG_CORE_BACKLOG; }

// Put every core to sleep right away
static void Initialize() {
    
    if(initialized)
        return;
    initialized = true;
    for(CPUId_t c = 0; c < NUM_CORES; c++) {
        SetPState(c, P3);
        pstate[c] = P3;
        SetCState(c, C6);
        state[c] = ASLEEP;
    }
}

static void Start(CPUId_t c, ProcessId_t pid, PState_t p) {
    if(pstate[c] != p) {
        SetPState(c, p);
        pstate[c] = p;
    }
    running[c] = pid;
    state[c] = BUSY;
    LoadContext(pid, c);
    RunCore(c);
}

static void WakeCores() {
    // Wake enough cores, most efficient first, to cover the processes that are waiting.
    size_t waking = 0;
    for(CPUId_t c : ORDER)
        if(state[c] == WAKING)
            waking++;
    for(CPUId_t c : ORDER) {
        if(readyQ.size() <= waking)
            break;
        if(state[c] == ASLEEP && AllowedRun(c)) {
            SetCState(c, C1);
            state[c] = WAKING;
            waking++;
        }
    }
}

static void RunOrSleep(CPUId_t c) {
    // The core is awake in C1: give it the next process, or put it back to sleep.
    if(readyQ.empty() || !AllowedRun(c)) {
        SetCState(c, C6);
        state[c] = ASLEEP;
        return;
    }
    ProcessId_t pid = readyQ.front();
    readyQ.pop_front();
    Start(c, pid, P3);
}

void CreateProcess(ProcessId_t pid) {
    // A new process has been created. Update the scheduler's data structures and decisions accordingly.
    SimOutput("CreateProcess(" + std::to_string(pid) + ")", 4);
    Initialize();
    readyQ.push_back(pid);
    WakeCores();
}

void ExitProcess(ProcessId_t pid) {
    // Process finished running. Update the scheduler's data structures and decisions accordingly.
    for(CPUId_t c = 0; c < NUM_CORES; c++) {
        if(state[c] == BUSY && running[c] == pid) {
            RunOrSleep(c);
            return;
        }
    }
}

void TimerInterrupt(Time_t now) {
    // You received a timer interrupt. This is where you want to execute scheduling decisions
    Initialize();
    // Switch a process to P4 for its final tick. SetPState on a running core here would count this tick's
    // progress twice, so the process is switched out and back in around the change.
    for(CPUId_t c = 0; c < NUM_CORES; c++) {
        if(state[c] == BUSY && pstate[c] == P3 && GetRemaining(running[c]) <= P4TickWork(c)) {
            SaveContext(running[c], c);
            Start(c, running[c], P4);
        }
    }
}

void CStateTransitionComplete(CPUId_t core_id){
    RunOrSleep(core_id);
}

void SimulationComplete(Time_t now) {
    // Add any bookkeeping or statistics that you would want to collect. Program terminates after this function returns.
    std::cout << "Run stopped at " << FormatTime(now) << " after consuming " << GetTotalEnergyConsumed()/3600000000.0 << " kWh" << std::endl;
}
