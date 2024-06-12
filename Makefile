.PHONY : clean all test

YEAR=2024
CFLAGS=-g -O3
LDFLAGS=
DISTFILES=SMT-COMP-$(YEAR)-trace-executor.tar.gz

all: $(DISTFILES)

smtlib2_trace_executor: smtlib2_trace_executor.o
	$(CC) $< -o $@ $(LDFLAGS)

clean:
	rm -f smtlib2_trace_executor.o smtlib2_trace_executor $(DISTFILES) test_solver_log.txt

SMT-COMP-$(YEAR)-trace-executor.tar.gz: starexec_run_incremental smtlib2_trace_executor
	tar -czf $@ $^

test: smtlib2_trace_executor
	test/run_tests.sh ./smtlib2_trace_executor

dist: $(DISTFILES)
	cp $(DISTFILES) /dist

