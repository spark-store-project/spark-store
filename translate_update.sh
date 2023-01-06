#!/bin/bash
# this file is used to auto-update .ts file.

cd $(dirname $0)

lupdate ./src/spark-store.pro -no-obsolete
