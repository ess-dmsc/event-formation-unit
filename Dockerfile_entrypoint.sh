#!/bin/sh

CMD="$BINARY_PATH -b $BROKER_ADDR -t $BROKER_TOPIC --nohwcheck"

if [ -n "$GRAPHITE" ]; then
    CMD="$CMD -g $GRAPHITE"
fi

if [ -z "$FILE" ]; then
    echo "Please specify a config file"
    exit 1
elif [ ! -r "./config/$FILE" ]; then
    echo "File $FILE does not exist or is not readable"
    exit 1
else
    CMD="$CMD -f ./config/$FILE"
fi


if [ -n "$CALIBRATION" ]; then
    CMD="$CMD --calibration ./config/$CALIBRATION"
fi
echo "Running $CMD"

exec $CMD
