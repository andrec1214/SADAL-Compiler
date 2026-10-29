#!/bin/sh
# Runs every tests/*.ada through the interpreter and compares the output
# with the matching .out file. A .in file, if present, is fed to stdin.

cd "$(dirname "$0")/.." || exit 1
pass=0
fail=0

for src in tests/*.ada; do
    name="${src%.ada}"
    input="$name.in"
    [ -f "$input" ] || input=/dev/null

    if ./sadal "$src" < "$input" 2>&1 | diff -u "$name.out" - > /dev/null; then
        pass=$((pass + 1))
    else
        echo "FAIL: $src"
        ./sadal "$src" < "$input" 2>&1 | diff -u "$name.out" -
        fail=$((fail + 1))
    fi
done

echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
