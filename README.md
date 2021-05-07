[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://travis-ci.com/SMT-COMP/trace-executor.svg?branch=master)](https://travis-ci.com/SMT-COMP/trace-executor)

SMT-COMP Trace-Executor
===============================================================================

This is the official "trace executor" for the Incremental Track (previously:
Application Track) of SMT-COMP.  It is based on the official "trace executor"
used in the Application track of SMT-COMP 2011.

The trace executor emulates an on-line interaction between an SMT solver and a
client application, providing incremental queries to the SMT solver.

```
  Usage: ./smtlib2_trace_executor [--continue-after-unknown] SOLVER BENCHMARK_WITH_SOLUTIONS
```

The format of the BENCHMARK_WITH_SOLUTIONS file is

```
  ((sat|unsat|unknown)\n)*
  --- BENCHMARK BEGINS HERE ---
  SMT-LIBv2 script
```

The prefix of solutions is used for checking the correctness of the results
given by the SMT solver. If the prefix contains n "sat"/"unsat" solutions
before the first "unknown", at most n (check-sat) queries will be executed
unless option `--continue-after-unknown` is enabled (off by default).

## Testing

`make test` runs the tests for the trace executor. This command runs the trace
executor on all the `*.smt2` files in the subdirectories of `test/` and
compares the output against the expected output in the corresponding
`*.smt2.expect` file. The `test_solver.sh` scripts in the subdirectories of
`test/` simulate solvers and for each test case, `make test` uses the
`test_solver.sh` script in the same directory as the input.

## Docker Build

To build an executable suitable for use on StarExec, you need to use a
centos:7 image.  We provide docker scripts to help.

```sh
cd docker
./create-docker.sh
./build-docker.sh
```

This creates a tar.gz file containing the binary trace executor and
the starexec run script.

## Wrapping your Solver

The pre- and post-processors of starexec are not suitable for the
incremental track where the trace-executor needs to interact with your
solver.  Instead the solution for SMT-COMP is to add the
trace executor to the solver binaries and upload a wrapped solver.
Note that in the competition the SMT-COMP organizers will wrap your
solver.  These instructions are meant to help solver authors to test
their solvers on starexec before the competition.

To wrap your solver, first follow the above instructions for the
Docker build or download the released tar archive.  Then rename your
solver's start script and extract the tar archive.

```sh
cd mysolver/bin
mv starexec_run_* original_starexec_run_default
tar -xzf .../SMT-COMP-20*-trace-executor.tar.gz
```

Upload the wrapped solver to starexec.  Note that the wrapped solver
can only be used in the incremental track with the incremental
scrambler and post-processor.
