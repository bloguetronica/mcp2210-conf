/* MCP2210 Configurator - Version 1.0 for Debian Linux
   Copyright (c) 2023 Samuel Lourenço

   This program is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation, either version 3 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License along
   with this program.  If not, see <https://www.gnu.org/licenses/>.


   Please feel free to contact me via e-mail: samuel.fmlourenco@gmail.com */


// Includes
#include <QMessageBox>
#include "configuratorwindow.h"
#include "ui_configuratorwindow.h"

ConfiguratorWindow::ConfiguratorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConfiguratorWindow)
{
    ui->setupUi(this);
}

ConfiguratorWindow::~ConfiguratorWindow()
{
    delete ui;
}

// Checks if the device window is currently fully enabled
bool ConfiguratorWindow::isViewEnabled()
{
    return viewEnabled_;
}

// Opens the device and prepares the corresponding window
void ConfiguratorWindow::openDevice(quint16 vid, quint16 pid, const QString &serialstr)
{
    int err = mcp2210_.open(vid, pid, serialstr);
    if (err == MCP2210::SUCCESS) {  // Device was successfully opened
        vid_ = vid;  // Pass VID
        pid_ = pid;  // and PID
        serialstr_ = serialstr;  // and the serial number as well
        //readDeviceConfiguration();
        this->setWindowTitle(tr("MCP2210 Device (S/N: %1)").arg(serialstr_));
        //displayConfiguration(deviceConfig_);
        viewEnabled_ = true;
    } else if (err == MCP2210::ERROR_INIT) {  // Failed to initialize libusb
        QMessageBox::critical(this, tr("Critical Error"), tr("Could not initialize libusb.\n\nThis is a critical error and execution will be aborted."));
        exit(EXIT_FAILURE);  // This error is critical because libusb failed to initialize
    } else {
        if (err == MCP2210::ERROR_NOT_FOUND) {  // Failed to find device
            QMessageBox::critical(this, tr("Error"), tr("Could not find device."));
        } else if (err == MCP2210::ERROR_BUSY) {  // Failed to claim interface
            QMessageBox::critical(this, tr("Error"), tr("Device is currently unavailable.\n\nPlease confirm that the device is not in use."));
        }
        this->deleteLater();  // Close window after the subsequent show() call
    }
}

// This is the routine that reads the configuration from the CP2130 OTP ROM
void ConfiguratorWindow::readDeviceConfiguration()
{
    int errcnt = 0;
    QString errstr;
    deviceConfig_.manufacturer = mcp2210_.getManufacturerDesc(errcnt, errstr);
    deviceConfig_.product = mcp2210_.getProductDesc(errcnt, errstr);
    if (errcnt > 0) {
        mcp2210_.close();
        if (mcp2210_.disconnected()) {
            QMessageBox::critical(this, tr("Error"), tr("Device disconnected.\n\nPlease reconnect it and try again."));
        } else {
            errstr.chop(1);  // Remove the last character, which is always a newline
            QMessageBox::critical(this, tr("Error"), tr("Read operation returned the following error(s):\n– %1\n\nPlease try accessing the device again.", "", errcnt).arg(errstr.replace("\n", "\n– ")));
        }
        this->deleteLater();  // In a context where the window is already visible, it has the same effect as this->close()
    }
}
