#!/bin/bash

set -eu

cp ../build/linux/linguine .

echo "Interpreter mode."

for f in syntax/*.ls; do
    echo -n "Running $f ... "
    ./linguine -i $f > out
    diff $f.out out
    rm out
    echo "ok."
done

echo "JIT mode."

for f in syntax/*.ls; do
    echo -n "Running $f ... "
    ./linguine $f > out
    diff $f.out out
    rm out
    echo "ok."
done

rm linguine

echo ''
echo 'All tests ok.'
