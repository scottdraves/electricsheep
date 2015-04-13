#! /bin/bash

DL_URL=http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.bz2/download
ARCH_NAME=boost_1_56_0

CD=`dirname $0`

INCLUDE_ROOT="$CD/../include"

mkdir -p "$INCLUDE_ROOT"

TEST_FILE="$INCLUDE_ROOT/$ARCH_NAME"

if [ -e "$TEST_FILE" ] ; then
	exit 0
fi

if [ ! -e "$CD/$ARCH_NAME.tar.bz2" ] ; then
	echo "Downloading boost archive"
	/usr/bin/curl -L -o "$CD/$ARCH_NAME.tar.bz2" "$DL_URL"
fi

if [ ! -e "$CD/$ARCH_NAME.tar.bz2" ] ; then
	exit 1
fi


echo "Extracting boost headers"
tar -xy --strip-components=1 -C "$INCLUDE_ROOT" -f "$CD/$ARCH_NAME.tar.bz2" $ARCH_NAME/boost

rm "$CD/$ARCH_NAME.tar.bz2"

touch "$TEST_FILE"
