#!/bin/sh

echo Removing rules...
rm -f /etc/udev/rules.d/70-mchp-generic.rules
services=$(service --status-all)
if echo $services | grep -q eudev; then
    service eudev restart
elif echo $services | grep -q udev; then
    service udev restart
fi
echo Done!
