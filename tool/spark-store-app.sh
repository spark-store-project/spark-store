SELF=$(readlink -f "$0")
HERE=${SELF%/*}

export PATH="${HERE}/bin${PATH:+:$PATH}"
export LD_LIBRARY_PATH="${HERE}/lddpath${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${HERE}/plugins"

exec spark-store $*
