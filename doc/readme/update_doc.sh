#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $DIR >> /dev/null

jitialize_dir=../../
export PYTHONIOENCODING="utf-8"

cat README.md.in | \
  python3 ${jitialize_dir}/doc/python.py | \
  python3 ${jitialize_dir}/doc/include.py
