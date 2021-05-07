#!/bin/sh

rm -rf trace-executor
mkdir ./trace-executor
cp -a ../[^d]* trace-executor
make -C trace-executor clean

docker build -t smtcomp .
