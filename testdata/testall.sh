#!/bin/bash

FINAL=0
function check_suffix() {
    COMMAND="$1"
    SUFFIX="$2"
    echo "checking $COMMAND for $SUFFIX"
    OUTPUT="$(timeout 5s $COMMAND)"
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
    OUTPUT=$(./gouda --backend=$BACKEND "$INFILE" -o a.out)
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

check_suffix "./demo.sh" " wanted output"
# check_suffix "./demo.sh" " unwanted output"
expect_pass "opencl" "fill_vec.cu" "Got: 0 1 2 3 4 5 6 7 8 9"
exit $FINAL
