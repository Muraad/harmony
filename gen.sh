#!/bin/bash

gyp --depth=. -f make -I common.gypi --generator-output=./build_test build.gyp
