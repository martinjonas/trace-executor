This is the official "trace executor" for the Application track of SMT-COMP.

The trace executor emulates an on-line interaction between an SMT solver and a
client application, providing incremental queries to the SMT solver.

  Usage: ./smtlib2_trace_executor SOLVER BENCHMARK_WITH_SOLUTIONS

The format of the BENCHMARK_WITH_SOLUTIONS file is

  ((sat|unsat|unknown)\n)*
  --- BENCHMARK BEGINS HERE ---
  SMT-LIBv2 script

The prefix of solutions is used for checking the correctness of the results
given by the SMT solver. If the prefix contains n "sat"/"unsat" solutions
before the first "unknown", at most n (check-sat) queries will be executed.

Author: Tjark Weber <tjark.weber@it.uu.se>

Based on the official "trace executor" for the Application track of
SMT-COMP 2011.

Author: Alberto Griggio <griggio@fbk.eu>
License: MIT

Copyright (C) 2011 Alberto Griggio

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
