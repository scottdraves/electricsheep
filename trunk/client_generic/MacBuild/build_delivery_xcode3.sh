#!/bin/bash

DEST=$1
XCODE_ROOT=$2

PM_CMD=$XCODE_ROOT/usr/bin/packagemaker

XCODE_CMD=$XCODE_ROOT/usr/bin/xcodebuild

BASE_DIR=`dirname "$0"`

cd "$BASE_DIR"

"$XCODE_CMD" -target All -configuration Release -project ElectricSheep-Xcode3.xcodeproj clean

"$XCODE_CMD" -target All -configuration Release -project ElectricSheep-Xcode3.xcodeproj build

./build_installer_xcode3.sh "$1" "$2"

cd -
