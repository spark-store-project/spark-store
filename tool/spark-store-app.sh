#!/bin/bash


export PATH="/opt/durapps/spark-store/bin${PATH:+:$PATH}"
export LD_LIBRARY_PATH="/opt/durapps/spark-store/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH:/usr:/usr/local:/lib/x86_64-linux-gnu}"
export QT_PLUGIN_PATH="/opt/durapps/spark-store/plugins"

exec spark-store $*
