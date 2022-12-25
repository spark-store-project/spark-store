#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.

cd $(dirname $0)

lrelease ./src/spark-store.pro
