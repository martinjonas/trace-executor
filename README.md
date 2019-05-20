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
