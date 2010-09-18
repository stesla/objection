#!/bin/bash

for test in ${@:-*.test}; do
    printf "%s..." $(basename -s .test $test)
    expected=$(sed -e '1,/RESULT/d' -e '/^$/d'< $test)
    actual=$(sed -e '/RESULT/,$d' -e '/^$/d' < $test | ${program:=$PWD/object -d -} 2>&1)
    if [ $? -ne 0 ]; then
        status=1
        echo ERROR
        echo "  $actual"
    elif [ "$actual" != "$expected" ]; then
        status=1
        echo FAIL
        echo "  Expected: $expected"
        echo "    Actual: $actual"
    else
        echo OK
    fi
done
exit $status