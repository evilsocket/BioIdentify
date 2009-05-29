#!/bin/bash
echo "Creating fingerprints biometric records database ..."
for i in $( ls fingerprints/*.bmp ); do
    ./bioid -f $i -b database/`basename $i`.BIR -A enroll -l "$i.`date`" | grep Computing
done
