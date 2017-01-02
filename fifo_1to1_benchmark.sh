#!/bin/bash

if [ $# -ne 3 ];then
	echo "USAGE: $0 runcount testype datanum"
	exit 1;
fi

RUN_COUNT=$1
TEST_TYPE=$2
DATA_NUM=$3


TYPE_DESC=""
if [ $TEST_TYPE -ne 0 ];then
	TYPE_DESC="Lock Version"
else
	TYPE_DESC="Lock-Free Version"
fi

echo "$TYPE_DESC benchmark start"
START_SEC=`date +%s`
START_NOS=`date +%N`

for((i=1;i<=$RUN_COUNT;i++))
do 
	./fifo_1to1_benchmark $TEST_TYPE $DATA_NUM > /dev/null
done

END_SEC=`date +%s`
END_NOS=`date +%N`

CONSUME_MS=$((($END_SEC - $START_SEC)*1000+$END_NOS/1000000-$START_NOS/1000000))
AVG_MS=$(($CONSUME_MS/$RUN_COUNT))

echo "$TYPE_DESC test result: run count $RUN_COUNT, total consume $CONSUME_MS ms, average consume $AVG_MS ms"


echo "$TYPE_DESC benchmark end"
