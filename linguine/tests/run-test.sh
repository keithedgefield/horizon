#!/bin/bash

set -eu

cp ../build/linux/linguine .

for f in syntax/*.ls; do
    echo -n "Running $f ... "
    ./linguine -i $f > out
    diff $f.out out
    rm out
    echo "ok."
done

rm linguine

echo ''
echo 'All tests ok.'
