#!/bin/bash

gcc -Wall -pedantic -O3 -fopenmp -D PROFILE src/*.c -lm -o PROG
function profile {
    ENC_CMD="./PROG -e $2 $3 g9-enc"
    DEC_CMD="./PROG -d g9-enc g9-dec"
    TIME=$( { time $ENC_CMD; } 2>&1 )
    ETVAL=$(echo $TIME | sed 's/[0-9,]*[ ]*real 0m\([0-9.]*\)s.*/\1/')
    EMVAL=$(echo $TIME | sed 's/\([0-9]*\),[0-9]*,[0-9]*[ ]*real 0m[0-9.]*s.*/\1/')
    ASVAL=$(echo $TIME | sed 's/[0-9]*,\([0-9]*\),[0-9]*[ ]*real 0m[0-9.]*s.*/\1/')
    MSVAL=$(echo $TIME | sed 's/[0-9]*,[0-9]*,\([0-9]*\)[ ]*real 0m[0-9.]*s.*/\1/')
    TIME=$( { time $DEC_CMD; } 2>&1 )
    DTVAL=$(echo $TIME | sed 's/[0-9,]*[ ]*real 0m\([0-9.]*\)s.*/\1/')
    DMVAL=$(echo $TIME | sed 's/\([0-9]*\),[0-9]*,[0-9]*[ ]*real 0m[0-9.]*s.*/\1/')
    val="Failure!"
    cmp g9-dec $3 && val="Success!"
    LL=$(ls -l g9-enc)
    SIZE=$(echo $LL | sed 's/.*root //g' | sed 's/ .*//')
    echo "$1,$ETVAL,$EMVAL,$DTVAL,$DMVAL,$SIZE,$ASVAL,$MSVAL"
    rm g9-dec
    rm g9-enc
}
function full_profile {
    profile "$1" "$2" "$3"
    profile "$1" "$2" "$3"
    # profile "$1" "$2" "$3"
    # profile "$1" "$2" "$3"
    # profile "$1" "$2" "$3"
}
echo "arrangement,ETVAL,EMVAL,DTVAL,DMVAL,SIZE,ANS,MSB" > "_kmsb.csv"
for i in $(seq 0 10 100); do full_profile "$i" "-b -a 16 -m -M 6 -k $i" "fcc.data" >> "_kmsb.csv"; done
# echo "arrangement,ETVAL,EMVAL,DTVAL,DMVAL,SIZE,ANS,MSB" > "_msb.csv"
# for i in $(seq 1 31); do full_profile "$i" "-b -a 16 -m -M $i" "fcc.data" >> "_msb.csv"; done
# echo "arrangement,ETVAL,EMVAL,DTVAL,DMVAL,SIZE,ANS,MSB" > "_nmsb.csv"
# for i in $(seq 1 31); do full_profile "$i" "-b -a 16 -n -M $i" "fcc.data" >> "_nmsb.csv"; done
# echo "arrangement,ETVAL,EMVAL,DTVAL,DMVAL,SIZE,ANS,MSB" > "_smsb.csv"
# for i in $(seq 1 31); do full_profile "$i" "-b -a 16 -m -M $i -S" "fcc.data" >> "_smsb.csv"; done




rm PROG