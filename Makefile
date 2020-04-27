.PHONY : clean all test

CFLAGS=-g -O3
LDFLAGS=-static -lutil

all: smtlib2_trace_executor SMT-COMP-2020-trace-executor.tar.xz

smtlib2_trace_executor: smtlib2_trace_executor.o
	$(CC) $< -o $@ $(LDFLAGS)

clean:
	rm -f smtlib2_trace_executor.o smtlib2_trace_executor SMT-COMP-2020-trace-executor.tar.xz

SMT-COMP-2020-trace-executor.tar.xz: starexec_run_default smtlib2_trace_executor
	tar -cJf $@ $^

test: smtlib2_trace_executor
	test/run_tests.sh ./smtlib2_trace_executor
