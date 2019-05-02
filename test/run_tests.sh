#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
TESTS="test_all_unknown.smt2 test_mixed.smt2"

for test in $TESTS; do
  echo $SCRIPT_DIR
  result=$(diff <($1 "${SCRIPT_DIR}/test_solver.sh" "${SCRIPT_DIR}/${test}") "${SCRIPT_DIR}/${test}.expect")
  if [ -z "$result" ]; then
    echo "SUCCESS"
  else
    echo "ERROR: Difference between expected and actual result:"
    echo $result
    exit 1
  fi
done
