#!/bin/bash

DEST=$1
XCODE_ROOT=$2

PM_CMD=$XCODE_ROOT/usr/bin/packagemaker

XCODE_CMD=$XCODE_ROOT/usr/bin/xcodebuild

BASE_DIR=`dirname "$0"`

cd "$BASE_DIR"

mkdir -p "$DEST"

"$PM_CMD" -d installer.pmdoc -o "$DEST/Electric Sheep_unsigned.mpkg"

productsign --sign "Developer ID Application: Scott Draves (D7639HSC8D)" "$DEST/Electric Sheep_unsigned.mpkg" "$DEST/Electric Sheep.mpkg"

rm -rf "$DEST/Electric Sheep_unsigned.mpkg"

hdiutil create -srcfolder "$DEST" "$DEST"

cd -
