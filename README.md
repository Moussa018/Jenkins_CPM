# Jenkins CPM Optimizer

A tool that analyzes a CI/CD pipeline's dependency graph and identifies its
**critical path** using the Critical Path Method (CPM) — a classic scheduling
algorithm. Stages that aren't on the critical path can be safely parallelized,
reducing total pipeline runtime. 
A Python script estimates realistic stage durations from Jenkins
build history for future builds.

## Why

Most Jenkins pipelines run stages sequentially even when some could run in
parallel. This tool builds a dependency graph of the pipeline, computes each
stage's earliest/latest start time and slack, and flags which stages are
critical (zero slack) versus which have room to be parallelized.

## Project structure

```
jenkins-cpm-optimizer/
├── CMakeLists.txt          # build configuration
├── Node.h                  # a single pipeline stage (name, duration, dependencies)
├── Graph.h                 # dependency graph + topological sort
├── CPMSolver.h              # CPM algorithm (forward/backward pass, slack, critical path)
├── main.cpp                 # entry point: builds a graph, runs the solver, prints results
├── scripts/
    └── estimate_durations.py    # queries Jenkins API for real stage durations
```

## How it works

1. **`Node`** stores a stage's name, duration, and direct dependencies.
2. **`Graph`** holds all nodes and can compute a topological order and reverse
   dependencies.
3. **`CPMSolver`** runs the two-pass CPM algorithm:
   - Forward pass → earliest start time for each stage
   - Backward pass → latest start time for each stage
   - Slack = LST − EST. A stage with slack = 0 is on the critical path.
4. **`estimate_durations.py`** pulls the last N builds from Jenkins' Pipeline
   API and averages stage durations for more accurate time builds.

## Tech stack

C++17, CMake, Python 3, Jenkins REST API (Pipeline plugin)
