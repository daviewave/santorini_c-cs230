#!/usr/bin/env bash
# Runs every unit test binary and every e2e script; exits non-zero on any failure.
set -euo pipefail
cd "$(dirname "$0")/.."

if [ ! -x build/Santorini ] || ! ls build/test/test_* >/dev/null 2>&1; then
    make -s all unit-tests
fi

status=0
mkdir -p build/test

for test_binary in build/test/test_*; do
    log="build/test/$(basename "$test_binary").log"
    if "$test_binary" 2>"$log"; then
        echo "PASS $test_binary ($(tail -n 1 "$log"))"
    else
        echo "FAIL $test_binary"
        cat "$log"
        status=1
    fi
done

for script in test/e2e/*.sh; do
    if bash "$script"; then
        echo "PASS $script"
    else
        echo "FAIL $script"
        status=1
    fi
done

exit $status
