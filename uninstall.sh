#!/bin/sh

echo Removing application...
rm -f /usr/local/share/applications/mcp2210-conf.desktop
rmdir --ignore-fail-on-non-empty /usr/local/share/applications
rm -f /usr/local/share/icons/hicolor/128x128/apps/mcp2210-conf.png
rmdir --ignore-fail-on-non-empty /usr/local/share/icons/hicolor/128x128/apps
rmdir --ignore-fail-on-non-empty /usr/local/share/icons/hicolor/128x128
rmdir --ignore-fail-on-non-empty /usr/local/share/icons/hicolor
rmdir --ignore-fail-on-non-empty /usr/local/share/icons
rm -f /usr/local/bin/mcp2210-conf
echo Removing source code files...
rm -rf /usr/local/src/mcp2210-conf
echo Done!
