#!/bin/sh
# Build with Xcode command line tools (macOS). Produces an arm64 dylib, then sign with ldid.
set -e
SDK=$(xcrun --sdk iphoneos --show-sdk-path)
clang -isysroot "$SDK" -arch arm64 -miphoneos-version-min=12.0 -O2 -Wall \
  -dynamiclib -o AMUIFullScreen.dylib AMUIFullScreen.c
ldid -S AMUIFullScreen.dylib
