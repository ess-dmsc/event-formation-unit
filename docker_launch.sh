#!/usr/bin/env bash

# Run specified executable, or the efu by default
# If a config file or optional arguments are provided then pass
# them to the executable, otherwise print the help message

# if EXECUTABLE is given as "efu" or omitted, then run the efu
if [ "${EXECUTABLE}" = "efu" ] || [ -z "${EXECUTABLE}" ]; then
  if [ -z "${CONFIG_FILE}" ] && [ -z "${OPT_ARGS}" ]; then
    ./efu/bin/efu --help
  elif [ -z "${OPT_ARGS}" ]; then
    ./efu/bin/efu --read_config=${CONFIG_FILE}
  else
    ./efu/bin/efu ${OPT_ARGS}
  fi
  # if specified executable doesn't exist then print an error
elif [ ! -f ./efu/bin/${EXECUTABLE} ]; then
  echo "Could not find executable with name ${EXECUTABLE}"
  exit 1
  # if specified executable exists then run it
else
  if [ -z "${OPT_ARGS}" ]; then
    ./efu/bin/${EXECUTABLE} --help
  else
    ./efu/bin/${EXECUTABLE} ${OPT_ARGS}
  fi
fi
