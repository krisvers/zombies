#!/usr/bin/env bash

cd working
if [ "$1" == "-p" ]; then
  pylauncher ../build/zombies -p
else
  ../build/zombies
fi
cd ..
