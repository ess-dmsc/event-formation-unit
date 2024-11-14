#!/bin/sh

CMD="$BINARY_PATH -b $BROKER_ADDR -t $BROKER_TOPIC --nohwcheck"

if [ -n "$GRAPHITE" ]; then
    CMD="$CMD -g $GRAPHITE"
fi

if [ -n "$FILE" ]; then
    CMD="$CMD -f ./config/$FILE"
fi

if [ -n "$CALIBRATION" ]; then
    CMD="$CMD -c ./config/$CALIBRATION"
fi
echo "Running $CMD"

exec $CMD
