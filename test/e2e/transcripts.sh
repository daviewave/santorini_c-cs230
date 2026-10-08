#!/usr/bin/env bash
# Feeds every test/e2e/cases/<name>.in to the game and diffs stdout against
# <name>.expected. The program must exit 0 on every case.
set -euo pipefail
cd "$(dirname "$0")/../.."

binary=build/Santorini
mkdir -p build/e2e
status=0

for input in test/e2e/cases/*.in; do
    [ -e "$input" ] || continue
    name=$(basename "$input" .in)
    expected="test/e2e/cases/$name.expected"
    actual="build/e2e/$name.out"
    if ! timeout 5 "$binary" <"$input" >"$actual"; then
        echo "transcripts: $name exited non-zero"
        status=1
        continue
    fi
    if ! diff -u "$expected" "$actual"; then
        echo "transcripts: $name differs from $expected"
        status=1
    fi
done

exit $status
