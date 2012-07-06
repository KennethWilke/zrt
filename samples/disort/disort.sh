#!/bin/bash

cd ../../zvm

#ALL 10SRC, 10DST
#SRC_NODES_LIST="2 3 4 5 6 7 8 9 10 11"
#DST_NODES_LIST="12 13 14 15 16 17 18 19 20 21"

#config for 10src, 10dst nodes
SRC_FIRST=2
SRC_LAST=11
DST_FIRST=12
DST_LAST=21

COUNTER=$SRC_FIRST
while [  $COUNTER -le $SRC_LAST ]; do
	gnome-terminal --geometry=80x20 -t "zerovm sortsrc$COUNTER.manifest" -x sh -c "./zerovm -M../samples/disort/manifest/sortsrc"$COUNTER".manifest"
    let COUNTER=COUNTER+1 
done

COUNTER=$DST_FIRST
while [  $COUNTER -le $DST_LAST ]; do
    gnome-terminal --geometry=80x20 -t "zerovm sortdst$COUNTER.manifest" -x sh -c "./zerovm -M../samples/disort/manifest/sortdst"$COUNTER".manifest"
    let COUNTER=COUNTER+1 
done

date > /tmp/time
./zerovm -M../samples/disort/manifest/sortman.manifest
date >> /tmp/time

cd ../samples/disort
cat log/sortman.stderr.log
echo Manager node working time: 
cat /tmp/time

