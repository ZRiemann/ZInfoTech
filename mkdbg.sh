#!/bin/sh

# fast rebuilde debug version.

make clean
make VER=debug
make VER=debug test

exit 0