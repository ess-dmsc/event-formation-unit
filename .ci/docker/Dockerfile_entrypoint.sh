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
    if [ ! -r "./calib/$CALIBRATION" ]; then
        echo "File $CALIBRATION does not exist or is not readable"
        exit 1
    else
        CMD="$CMD --calibration ./calib/$CALIBRATION"
    fi
fi

if [ -n "$KAFKA_CONFIG_PATH" ]; then
    if [ ! -r "$KAFKA_CONFIG_PATH" ]; then
        echo "File $KAFKA_CONFIG_PATH does not exist or is not readable"
        exit 1
    else
        CMD="$CMD --kafka_config $KAFKA_CONFIG_PATH"
    fi
fi

if [ -n "$REGION" ]; then
    CMD="$CMD --region $REGION"
fi

if [ -n "$EXTRA_ARGS" ]; then
    CMD="$CMD $EXTRA_ARGS"
fi

echo "Running $CMD"

exec $CMD
