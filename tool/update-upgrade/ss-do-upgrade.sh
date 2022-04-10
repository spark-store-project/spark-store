#!/bin/bash
set -e
sudo apt upgrade -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0"

