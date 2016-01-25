#!/bin/bash

set -e

. assert/assert.sh


USAGE="Usage: forest dimensionX dimensionY output simlength [log]"


# better build it first
make clean && make forest

# stdout tests

# test no args
assert "./forest" "$USAGE"
# test bad number of args
assert "./forest 32" "$USAGE"
assert "./forest 32 32" "$USAGE"
assert "./forest 32 32 fake" "$USAGE"
assert "./forest 32 32 null 100 log 100" "$USAGE"

# test bad args
assert "./forest 32 31 null 100" "Error: please use mod8 grid dimensions\n$USAGE"
assert "./forest 31 32 null 100" "Error: please use mod8 grid dimensions\n$USAGE"
assert "./forest 32 32 fake 100" "Error: output should be one of 'png' 'ncurses' 'null' 'verify'\n$USAGE"
assert "./forest 32 32 null 0" "Error: invalid simlength, please enter an integer between 1 and 2147483647\n$USAGE"
assert "./forest 32 32 null -5" "Error: invalid simlength, please enter an integer between 1 and 2147483647\n$USAGE"
assert "./forest 32 32 null 2147483648" "Error: invalid simlength, please enter an integer between 1 and 2147483647\n$USAGE"
assert "./forest 32 32 null 100 notlog" "$USAGE"
assert_end parse_args

# test with null output
assert "./forest 32 32 null 100"
assert "./forest 160 80 null 100"
assert "./forest 80 160 null 100"

assert_end out_null

# test with png output
assert "./forest 32 32 png 100"
assert "rm out/*.png" # cleanup
assert "./forest 32 48 png 100"
assert "rm out/*.png"
assert "./forest 48 32 png 100"
assert "rm out/*.png"
assert "./forest 1280 720 png 50"
assert "rm out/*.png"

assert_end out_png


#test with ncurses output (hopefully this works)
assert "./forest 32 32 ncurses 50 >/dev/null"
assert "./forest 32 64 ncurses 50 >/dev/null"
assert "./forest 64 32 ncurses 50 >/dev/null"

assert_end out_ncurses

#test with verify output

assert "./forest 32 32 verify 100" "Calculating grid integrity...\ntreeCount and burnCount okay\nGrid checksum is: \$1\$cosc3500\$TpmwG7ASCs7ohuukdKSWs1"
assert "./forest 640 480 verify 250" "Calculating grid integrity...\ntreeCount and burnCount okay\nGrid checksum is: \$1\$cosc3500\$UNIeN3yUf8MIcnW9ZLQQI."
assert "./forest 1280 720 verify 50" "Calculating grid integrity...\ntreeCount and burnCount okay\nGrid checksum is: \$1\$cosc3500\$y6TrYLIbOV4b93KUwaSU1."

assert_end out_verify


