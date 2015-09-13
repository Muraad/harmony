#!/bin/bash

gyp -DOS=linux --depth=. -f make -I common.gypi --generator-output=./build_test build.gyp
