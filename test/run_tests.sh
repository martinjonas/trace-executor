#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
TESTS="test_all_unknown.smt2 test_mixed.smt2"

for bench in $(find "${SCRIPT_DIR}" -name "*.smt2"); do
  echo ${bench}
  SOLVER="$(dirname "${bench}")/test_solver.sh"
  EXPECT="${bench}.expect"

  if [ ! -f "${EXPECT}" ]; then
    echo "ERROR: file ${EXPECT} with expected results missing"
    exit 1
  elif [ ! -x "${SOLVER}" ]; then
    echo "ERROR: solver ${SOLVER} missing or not executable"
    exit 1
  fi

  result=$(diff <($1 "${SOLVER}" "${bench}") "${EXPECT}")
  if [ -z "$result" ]; then
    echo "SUCCESS"
  else
    echo "ERROR: Difference between expected and actual result:"
    echo "${result}"
    exit 1
  fi
done
