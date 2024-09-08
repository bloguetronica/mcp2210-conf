#!/bin/sh

echo Obtaining required packages...
apt-get -qq update
apt-get -qq install build-essential
apt-get -qq install libusb-1.0-0-dev
apt-get -qq install qt5-default
apt-get -qq install qtbase5-dev
echo Copying source code files...
mkdir -p /usr/local/src/mcp2210-conf/icons/buttons
mkdir -p /usr/local/src/mcp2210-conf/images
mkdir -p /usr/local/src/mcp2210-conf/misc
mkdir -p /usr/local/src/mcp2210-conf/translations
cp -f src/aboutdialog.cpp /usr/local/src/mcp2210-conf/.
cp -f src/aboutdialog.h /usr/local/src/mcp2210-conf/.
cp -f src/aboutdialog.ui /usr/local/src/mcp2210-conf/.
cp -f src/common.cpp /usr/local/src/mcp2210-conf/.
cp -f src/common.h /usr/local/src/mcp2210-conf/.
cp -f src/configuration.cpp /usr/local/src/mcp2210-conf/.
cp -f src/configuration.h /usr/local/src/mcp2210-conf/.
cp -f src/configurationreader.cpp /usr/local/src/mcp2210-conf/.
cp -f src/configurationreader.h /usr/local/src/mcp2210-conf/.
cp -f src/configurationwriter.cpp /usr/local/src/mcp2210-conf/.
cp -f src/configurationwriter.h /usr/local/src/mcp2210-conf/.
cp -f src/configuratorwindow.cpp /usr/local/src/mcp2210-conf/.
cp -f src/configuratorwindow.h /usr/local/src/mcp2210-conf/.
cp -f src/configuratorwindow.ui /usr/local/src/mcp2210-conf/.
cp -f src/GPL.txt /usr/local/src/mcp2210-conf/.
cp -f src/icons/active64.png /usr/local/src/mcp2210-conf/icons/.
cp -f src/icons/buttons/password-reveal.png /usr/local/src/mcp2210-conf/icons/buttons/.
cp -f src/icons/buttons/password-reveal.svg /usr/local/src/mcp2210-conf/icons/buttons/.
cp -f src/icons/greyed64.png /usr/local/src/mcp2210-conf/icons/.
cp -f src/icons/icon.svg /usr/local/src/mcp2210-conf/icons/.
cp -f src/icons/mcp2210-conf.png /usr/local/src/mcp2210-conf/icons/.
cp -f src/icons/selected64.png /usr/local/src/mcp2210-conf/icons/.
cp -f src/images/banner.png /usr/local/src/mcp2210-conf/images/.
cp -f src/images/banner.svg /usr/local/src/mcp2210-conf/images/.
cp -f src/LGPL.txt /usr/local/src/mcp2210-conf/.
cp -f src/libusb-extra.c /usr/local/src/mcp2210-conf/.
cp -f src/libusb-extra.h /usr/local/src/mcp2210-conf/.
cp -f src/main.cpp /usr/local/src/mcp2210-conf/.
cp -f src/mainwindow.cpp /usr/local/src/mcp2210-conf/.
cp -f src/mainwindow.h /usr/local/src/mcp2210-conf/.
cp -f src/mainwindow.ui /usr/local/src/mcp2210-conf/.
cp -f src/mcp2210-conf.pro /usr/local/src/mcp2210-conf/.
cp -f src/mcp2210.cpp /usr/local/src/mcp2210-conf/.
cp -f src/mcp2210.h /usr/local/src/mcp2210-conf/.
cp -f src/mcp2210eeprom.cpp /usr/local/src/mcp2210-conf/.
cp -f src/mcp2210eeprom.h /usr/local/src/mcp2210-conf/.
cp -f src/mcp2210limits.h /usr/local/src/mcp2210-conf/.
cp -f src/misc/mcp2210-conf.desktop /usr/local/src/mcp2210-conf/misc/.
cp -f src/passworddialog.cpp /usr/local/src/mcp2210-conf/.
cp -f src/passworddialog.h /usr/local/src/mcp2210-conf/.
cp -f src/passworddialog.ui /usr/local/src/mcp2210-conf/.
cp -f src/resources.qrc /usr/local/src/mcp2210-conf/.
cp -f src/statusdialog.cpp /usr/local/src/mcp2210-conf/.
cp -f src/statusdialog.h /usr/local/src/mcp2210-conf/.
cp -f src/statusdialog.ui /usr/local/src/mcp2210-conf/.
cp -f src/translations/mcp2210-conf_en.qm /usr/local/src/mcp2210-conf/translations/.
cp -f src/translations/mcp2210-conf_en.ts /usr/local/src/mcp2210-conf/translations/.
cp -f src/translations/mcp2210-conf_en_US.qm /usr/local/src/mcp2210-conf/translations/.
cp -f src/translations/mcp2210-conf_en_US.ts /usr/local/src/mcp2210-conf/translations/.
cp -f src/translations/mcp2210-conf_pt.qm /usr/local/src/mcp2210-conf/translations/.
cp -f src/translations/mcp2210-conf_pt.ts /usr/local/src/mcp2210-conf/translations/.
cp -f src/translations/mcp2210-conf_pt_PT.qm /usr/local/src/mcp2210-conf/translations/.
cp -f src/translations/mcp2210-conf_pt_PT.ts /usr/local/src/mcp2210-conf/translations/.
echo Building and installing application...
cd /usr/local/src/mcp2210-conf
qmake
make install clean
rm -f mcp2210-conf
echo Done!
