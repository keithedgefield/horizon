#!/bin/bash

set -eu

cp ../build/macos/linguine .

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
