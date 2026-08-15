#!/bin/bash

# Use gtimeout from coreutils on mac.
TIMEOUT="timeout"
if [[ "$(uname)" == "Darwin" ]]; then
    TIMEOUT="gtimeout"
fi

FINAL=0
function check_suffix() {
    COMMAND="$1"
    SUFFIX="$2"
    echo "checking $COMMAND for $SUFFIX"
    OUTPUT="$($TIMEOUT 5s $COMMAND)"
    read -rd '' OUTPUT <<< "$OUTPUT"
    if [[ "$OUTPUT" == *"$SUFFIX" ]]; then
        printf "\tOK\n"
    else
        printf '\tOutput mismatch. Did not end with "%s"\n' "$SUFFIX"
        printf '\tHad: "%s"\n' "$OUTPUT"
        FINAL=1
    fi
}
function expect_pass() {
    BACKEND="$1"
    INFILE="$2"
    SUFFIX="$3"
    echo "$INFILE via $BACKEND"
    rm a.out
    OUTPUT=$(./gouda --backend $BACKEND "$INFILE" -o a.out)
    CODE=$?
    printf 'checking '%s'\n' "$OUTPUT"
    if [[ "$CODE" -ne "0" ]]; then
        printf "\tCompilation failed for $BACKEND on $INFILE.\n"
        printf "\tExit code: $CODE\n"
        printf '\tOutput: "%s"\n' "$OUTPUT"
        FINAL=1
    fi

    check_suffix "./a.out" "$SUFFIX"
}

function expect_all() {
    INFILE="$1"
    SUFFIX="$2"
    BACKENDS="opencl serial"
    echo "CI = $CI"
    if [[ "$CI" == "true" && "$(uname)" == "Darwin" ]]; then
        echo "CI DETECTED! Using only serial backend for mac."
        # Virtualized Mac, like our CI, doesn't support OpenCL.
        BACKENDS="serial"
    fi

    for backend in $BACKENDS; do
        expect_pass "$backend" "$INFILE" "$SUFFIX"
    done

    echo "Finished testing against ${BACKENDS}"
}

check_suffix "./demo.sh" " wanted output"
# check_suffix "./demo.sh" " unwanted output"
expect_all "fill_vec.cu" "Got: 0 1 2 3 4 5 6 7 8 9"
expect_all "fill_const.cu" "Got: 2 3 4 5 6 7 8 9 10 11"
expect_all "fill_device_multi.cu" "Got: 5 7 9 11 13 15 17 19 21 23"
exit $FINAL
