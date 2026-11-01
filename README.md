# Energy-aware CPU scheduling policy

My C++ scheduling-policy implementation for CS 350M Energy-Efficient Computing (Fall 2026). The course supplied an eight-core big/small CPU simulator and its scheduler interface; this repository contains only my policy file. The original starter-file attribution remains in `scheduler.cpp`.

The policy favors small cores and activates big cores when the ready queue exceeds a backlog threshold. It manages core sleep/wake and performance states while dispatching queued processes. I compared 15 policies in the course simulator. In my submitted report, the selected policy used 0.348 kWh of **simulated** energy versus 2.080 kWh for the provided round-robin baseline, about 83% less. These are simulator results, not physical hardware measurements.

## Build and reproduction

The file requires the course-provided `scheduler.hpp`, simulator library, and workload. Those are not distributed here, so this repository cannot be built or rerun on its own. The results above are reported from the submitted course project and were not independently reproduced for this public copy.

## Authorship

I implemented and evaluated the scheduling policy. I did not author the simulator, supplied interface, or workload. The course report retained its original two-author byline; the individual policy contribution follows my later clarification.
