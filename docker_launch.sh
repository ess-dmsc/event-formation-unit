#!/usr/bin/env bash

if [[ "${EXECUTABLE}" = "efu"] || [ -z "${EXECUTABLE}" ]]; then
  if [[ -z "${CONFIG_FILE}" ] && [ -z "${OPT_ARGS}" ]]; then
    /efu/bin/efu --help
  elif [[ -z "${OPT_ARGS}" ]]
    /efu/bin/efu --read_config=${CONFIG_FILE}
  else
    /efu/bin/efu ${OPT_ARGS}
  fi
elif [ ! -f /efu/bin/${EXECUTABLE} ]; then
    echo "Could not find executable with name ${EXECUTABLE}"
else
  if [[ -z "${OPT_ARGS}" ]]; then
    /efu/bin/${EXECUTABLE} --help
  else
    /efu/bin/${EXECUTABLE} ${OPT_ARGS}
  fi
fi
