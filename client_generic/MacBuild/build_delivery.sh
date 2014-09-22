#!/bin/bash

WD=build/Release

DEST=$1

BASE_DIR=`dirname "$0"`

cd "$BASE_DIR"

xcodebuild -scheme All -configuration Release -project ElectricSheep.xcodeproj clean

xcodebuild -scheme All -configuration Release -project ElectricSheep.xcodeproj build

VERSION=$(defaults read "`pwd`/$WD/Electric Sheep.saver/Contents/Info" CFBundleVersion)

./build_installer.sh "$VERSION" "$DEST"

cd -
