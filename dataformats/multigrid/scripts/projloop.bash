#!/bin/bash


./genpixids -s 1 -e 10000 output/nfsoffline_01
./scripts/projections.py output/nfsoffline_01

./genpixids -s 1 -e 20000 output/nfsoffline_01
./scripts/projections.py output/nfsoffline_01

./genpixids -s 1 -e 30000 output/nfsoffline_01
./scripts/projections.py output/nfsoffline_01

./genpixids -s 1 -e 40000 output/nfsoffline_01
./scripts/projections.py output/nfsoffline_01
