#!/bin/bash

set -e
set -u

rm -f test_solver_log.txt

while read LINE; do
    echo "> '$LINE'" >> test_solver_log.txt
    case "$LINE" in
        "(check-sat)")
            echo "< 'unknown'" >> test_solver_log.txt
            echo "unknown"
            ;;
        "(exit)")
            exit 0
            ;;
        *)
            echo "< 'success'" >> test_solver_log.txt
            echo "success"
            ;;
    esac
done
