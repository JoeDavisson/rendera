#!/usr/bin/env bash

MOUNTPOINT="/Volumes/MountPoint"
IFS="
"
hdiutil attach -mountpoint $MOUNTPOINT ${1}

for app in `find $MOUNTPOINT -type d -maxdepth 2 -name \*.app `; do
    cp -a "$app" /Applications/
done

hdiutil detach $MOUNTPOINT
