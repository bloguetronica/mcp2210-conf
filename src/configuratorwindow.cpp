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
#include <QRegExp>
#include <QRegExpValidator>
#include "common.h"
#include "configuratorwindow.h"
#include "ui_configuratorwindow.h"

// Definitions
const int POWER_LIMIT = 500;  // Maximum current consumption limit, as per the USB 2.0 specification

ConfiguratorWindow::ConfiguratorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConfiguratorWindow)
{
    ui->setupUi(this);
    ui->lineEditVID->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditPID->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditMaxPower->setValidator(new QRegExpValidator(QRegExp("[\\d]+"), this));
    ui->lineEditMaxPowerHex->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
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
        readDeviceConfiguration();
        this->setWindowTitle(tr("MCP2210 Device (S/N: %1)").arg(serialstr_));
        displayConfiguration(deviceConfig_);
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

void ConfiguratorWindow::on_actionAbout_triggered()
{
    showAboutDialog();
}

void ConfiguratorWindow::on_actionStatus_triggered()
{
    if (statusDialog_.isNull()) {  // If the dialog is not open
        int errcnt = 0;
        QString errstr;
        // Obtain information here????
        opCheck(tr("device-status-retrieval-op"), errcnt, errstr);  // The string "device-status-retrieval-op" should be translated to "Device status retrieval"
        if (err_) {
            handleError();
        } else {  // If error check passes
            statusDialog_ = new StatusDialog(this);
            statusDialog_->setAttribute(Qt::WA_DeleteOnClose);  // It is important to delete the dialog in memory once closed, in order to force the application to retrieve the device status if the window is opened again???
            statusDialog_->setWindowTitle(tr("Device Status (S/N: %1)").arg(serialstr_));
            // To implement set texts
            statusDialog_->show();
        }
    } else {
        statusDialog_->showNormal();  // Required if the dialog is minimized
        statusDialog_->activateWindow();  // Set focus on the previous dialog (dialog is raised and selected)
    }
}

void ConfiguratorWindow::on_comboBoxGP0_currentIndexChanged(int index)
{
    ui->checkBoxGP0DefaultValue->setEnabled(index == 1 && !deviceLocked_);
}

void ConfiguratorWindow::on_comboBoxGP1_currentIndexChanged(int index)
{
    ui->checkBoxGP1DefaultValue->setEnabled(index == 1 && !deviceLocked_);
}

void ConfiguratorWindow::on_comboBoxGP2_currentIndexChanged(int index)
{
    ui->checkBoxGP2DefaultValue->setEnabled(index == 1 && !deviceLocked_);
}

void ConfiguratorWindow::on_comboBoxGP3_currentIndexChanged(int index)
{
    ui->checkBoxGP3DefaultValue->setEnabled(index == 1 && !deviceLocked_);
}

void ConfiguratorWindow::on_comboBoxGP4_currentIndexChanged(int index)
{
    ui->checkBoxGP4DefaultValue->setEnabled(index == 1 && !deviceLocked_);
}

void ConfiguratorWindow::on_comboBoxGP5_currentIndexChanged(int index)
{
    ui->checkBoxGP5DefaultValue->setEnabled(index == 1 && !deviceLocked_);
}

void ConfiguratorWindow::on_comboBoxGP6_currentIndexChanged(int index)
{
    ui->checkBoxGP6DefaultValue->setEnabled(index == 1 && !deviceLocked_);
}

void ConfiguratorWindow::on_comboBoxGP7_currentIndexChanged(int index)
{
    ui->checkBoxGP7DefaultValue->setEnabled(index == 1 && !deviceLocked_);
}

void ConfiguratorWindow::on_lineEditManufacturer_textEdited()
{
    int curPosition = ui->lineEditManufacturer->cursorPosition();
    ui->lineEditManufacturer->setText(ui->lineEditManufacturer->text().replace('\n', ' '));
    ui->lineEditManufacturer->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_lineEditMaxPower_editingFinished()
{
    ui->lineEditMaxPower->setText(QString::number(2 * (ui->lineEditMaxPower->text().toInt() / 2)));  // This removes any leading zeros and also rounds to the previous even number, if the value is odd
}

void ConfiguratorWindow::on_lineEditMaxPower_textChanged()
{
    if (ui->lineEditMaxPower->text().isEmpty()) {
        ui->lineEditMaxPower->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditMaxPower->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditMaxPower_textEdited()
{
    QString maxPowerStr = ui->lineEditMaxPower->text();
    int maxPower = maxPowerStr.toInt();
    if (maxPower > POWER_LIMIT) {
        maxPowerStr.chop(1);
        ui->lineEditMaxPower->setText(maxPowerStr);
        maxPower /= 10;
    }
    ui->lineEditMaxPowerHex->setText(QString("%1").arg(maxPower / 2, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
}

void ConfiguratorWindow::on_lineEditMaxPowerHex_editingFinished()
{
    if (ui->lineEditMaxPowerHex->text().size() < 2) {
        ui->lineEditMaxPowerHex->setText(QString("%1").arg(ui->lineEditMaxPowerHex->text().toInt(nullptr, 16), 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
    }
}

void ConfiguratorWindow::on_lineEditMaxPowerHex_textChanged()
{
    if (ui->lineEditMaxPowerHex->text().isEmpty()) {
        ui->lineEditMaxPowerHex->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditMaxPowerHex->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditMaxPowerHex_textEdited()
{
    int curPosition = ui->lineEditMaxPowerHex->cursorPosition();
    ui->lineEditMaxPowerHex->setText(ui->lineEditMaxPowerHex->text().toLower());
    int maxPower = 2 * ui->lineEditMaxPowerHex->text().toInt(nullptr, 16);
    if (maxPower > POWER_LIMIT) {
        maxPower = POWER_LIMIT;
        ui->lineEditMaxPowerHex->setText(QString("%1").arg(POWER_LIMIT / 2, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
    }
    ui->lineEditMaxPowerHex->setCursorPosition(curPosition);
    ui->lineEditMaxPower->setText(QString::number(maxPower));
}

void ConfiguratorWindow::on_lineEditPID_textChanged()
{
    if (ui->lineEditPID->text().size() < 4 || ui->lineEditPID->text() == "0000") {
        ui->lineEditPID->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditPID->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditPID_textEdited()
{
    int curPosition = ui->lineEditPID->cursorPosition();
    ui->lineEditPID->setText(ui->lineEditPID->text().toLower());
    ui->lineEditPID->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_lineEditProduct_textEdited()
{
    int curPosition = ui->lineEditProduct->cursorPosition();
    ui->lineEditProduct->setText(ui->lineEditProduct->text().replace('\n', ' '));
    ui->lineEditProduct->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_lineEditVID_textChanged()
{
    if (ui->lineEditVID->text().size() < 4 || ui->lineEditVID->text() == "0000") {
        ui->lineEditVID->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditVID->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditVID_textEdited()
{
    int curPosition = ui->lineEditVID->cursorPosition();
    ui->lineEditVID->setText(ui->lineEditVID->text().toLower());
    ui->lineEditVID->setCursorPosition(curPosition);
}

// Partially disables configurator window
void ConfiguratorWindow::disableView()
{
    ui->actionStatus->setEnabled(false);
    ui->actionClose->setText(tr("&Close Window"));
    ui->centralWidget->setEnabled(false);
    viewEnabled_ = false;
}

// Updates all fields pertaining to the MCP2210 chip settings
void ConfiguratorWindow::displayChipSettings(const MCP2210::ChipSettings &chipsettings)
{
    ui->comboBoxGP0->setCurrentIndex(chipsettings.gp0 == MCP2210::PCGPIO ? (0x01 & chipsettings.gpdir) == 0x00 : chipsettings.gp0 + 1);
    ui->checkBoxGP0DefaultValue->setChecked((0x01 & chipsettings.gpout) != 0x00);
    ui->comboBoxGP1->setCurrentIndex(chipsettings.gp1 == MCP2210::PCGPIO ? (0x02 & chipsettings.gpdir) == 0x00 : chipsettings.gp1 + 1);
    ui->checkBoxGP1DefaultValue->setChecked((0x02 & chipsettings.gpout) != 0x00);
    ui->comboBoxGP2->setCurrentIndex(chipsettings.gp2 == MCP2210::PCGPIO ? (0x04 & chipsettings.gpdir) == 0x00 : chipsettings.gp2 + 1);
    ui->checkBoxGP2DefaultValue->setChecked((0x04 & chipsettings.gpout) != 0x00);
    ui->comboBoxGP3->setCurrentIndex(chipsettings.gp3 == MCP2210::PCGPIO ? (0x08 & chipsettings.gpdir) == 0x00 : chipsettings.gp3 + 1);
    ui->checkBoxGP3DefaultValue->setChecked((0x08 & chipsettings.gpout) != 0x00);
    ui->comboBoxGP4->setCurrentIndex(chipsettings.gp4 == MCP2210::PCGPIO ? (0x10 & chipsettings.gpdir) == 0x00 : chipsettings.gp4 + 1);
    ui->checkBoxGP4DefaultValue->setChecked((0x10 & chipsettings.gpout) != 0x00);
    ui->comboBoxGP5->setCurrentIndex(chipsettings.gp5 == MCP2210::PCGPIO ? (0x20 & chipsettings.gpdir) == 0x00 : chipsettings.gp5 + 1);
    ui->checkBoxGP5DefaultValue->setChecked((0x20 & chipsettings.gpout) != 0x00);
    ui->comboBoxGP6->setCurrentIndex(chipsettings.gp6 == MCP2210::PCGPIO ? (0x40 & chipsettings.gpdir) == 0x00 : chipsettings.gp6 + 1);
    ui->checkBoxGP6DefaultValue->setChecked((0x40 & chipsettings.gpout) != 0x00);
    ui->comboBoxGP7->setCurrentIndex(chipsettings.gp7 == MCP2210::PCGPIO ? (0x80 & chipsettings.gpdir) == 0x00 : chipsettings.gp7 + 1);
    ui->checkBoxGP7DefaultValue->setChecked((0x80 & chipsettings.gpout) != 0x00);
    ui->comboBoxGP8->setCurrentIndex(chipsettings.gp8 == MCP2210::PCGPIO ? 0 : 1);
}

// This is the main display routine, used to display the given configuration, updating all fields accordingly
void ConfiguratorWindow::displayConfiguration(const Configuration &config)
{
    displayManufacturer(config.manufacturer);
    setManufacturerEnabled(!deviceLocked_);
    displayProduct(config.product);
    setProductEnabled(!deviceLocked_);
    displayUSBParameters(config.usbparameters);
    setVIDEnabled(!deviceLocked_);
    setPIDEnabled(!deviceLocked_);
    setMaxPowerEnabled(!deviceLocked_);
    setPowerModeEnabled(!deviceLocked_);
    setRemoteWakeUpCapableEnabled(!deviceLocked_);
    displayChipSettings(config.chipsettings);
    setChipSettingsEnabled(!deviceLocked_);
}

// Updates the manufacturer descriptor field
void ConfiguratorWindow::displayManufacturer(const QString &manufacturer)
{
    ui->lineEditManufacturer->setText(manufacturer);
}

// Updates the product descriptor field
void ConfiguratorWindow::displayProduct(const QString &product)
{
    ui->lineEditProduct->setText(product);
}

// Updates all fields pertaining to USB parameters
void ConfiguratorWindow::displayUSBParameters(const MCP2210::USBParameters &usbparameters)
{
    ui->lineEditVID->setText(QString("%1").arg(usbparameters.vid, 4, 16, QChar('0')));  // This will autofill with up to four leading zeros
    ui->lineEditPID->setText(QString("%1").arg(usbparameters.pid, 4, 16, QChar('0')));  // Same as before
    ui->lineEditMaxPower->setText(QString::number(2 * usbparameters.maxpow));
    ui->lineEditMaxPowerHex->setText(QString("%1").arg(usbparameters.maxpow, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
    ui->comboBoxPowerMode->setCurrentIndex(usbparameters.powmode);
    ui->checkBoxRemoteWakeUpCapable->setChecked(usbparameters.rmwakeup);
}

// Determines the type of error and acts accordingly, always showing a message
void ConfiguratorWindow::handleError()
{
    if (mcp2210_.disconnected() || !mcp2210_.isOpen()) {
        disableView();  // Disable configurator window
        mcp2210_.close();  // If the device is already closed, this will have no effect
    }
    QMessageBox::critical(this, tr("Error"), errmsg_);
}

// Checks for errors and validates device operations
void ConfiguratorWindow::opCheck(const QString &op, int errcnt, QString errstr)
{
    if (errcnt > 0) {
        err_ = true;
        if (mcp2210_.disconnected()) {
            errmsg_ = tr("Device disconnected.\n\nPlease reconnect it and try again.");
        } else {
            errstr.chop(1);  // Remove the last character, which is always a newline
            errmsg_ = tr("%1 operation returned the following error(s):\n– %2", "", errcnt).arg(op, errstr.replace("\n", "\n– "));
        }
    }
}

// This is the routine that reads the configuration from the MCP2210 NVRAM
void ConfiguratorWindow::readDeviceConfiguration()
{
    int errcnt = 0;
    QString errstr;
    deviceConfig_.manufacturer = mcp2210_.getManufacturerDesc(errcnt, errstr);
    deviceConfig_.product = mcp2210_.getProductDesc(errcnt, errstr);
    deviceConfig_.usbparameters = mcp2210_.getUSBParameters(errcnt, errstr);
    deviceConfig_.chipsettings = mcp2210_.getNVChipSettings(errcnt, errstr);
    deviceLocked_ = mcp2210_.getAccessControlMode(errcnt, errstr) == MCP2210::ACLOCKED;
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

// Enables or disables all fields pertaining to the MCP2210 chip settings
void ConfiguratorWindow::setChipSettingsEnabled(bool value)
{
    ui->comboBoxGP0->setEnabled(value);
    ui->comboBoxGP1->setEnabled(value);
    ui->comboBoxGP2->setEnabled(value);
    ui->comboBoxGP3->setEnabled(value);
    ui->comboBoxGP4->setEnabled(value);
    ui->comboBoxGP5->setEnabled(value);
    ui->comboBoxGP6->setEnabled(value);
    ui->comboBoxGP7->setEnabled(value);
    ui->comboBoxGP8->setEnabled(value);
}

// Enables or disables the manufacturer descriptor field
void ConfiguratorWindow::setManufacturerEnabled(bool value)
{
    ui->lineEditManufacturer->setReadOnly(!value);
}

// Enables or disables the maximum power consuption configuration fields
void ConfiguratorWindow::setMaxPowerEnabled(bool value)
{
    ui->lineEditMaxPower->setReadOnly(!value);
    ui->lineEditMaxPowerHex->setReadOnly(!value);
}

// Enables or disables the PID field
void ConfiguratorWindow::setPIDEnabled(bool value)
{
    ui->lineEditPID->setReadOnly(!value);
}

// Enables or disables the power mode configuration combo box
void ConfiguratorWindow::setPowerModeEnabled(bool value)
{
    ui->comboBoxPowerMode->setEnabled(value);
}

// Enables or disables the product descriptor field
void ConfiguratorWindow::setProductEnabled(bool value)
{
    ui->lineEditProduct->setReadOnly(!value);
}

// Enables or disables the remote wake-up capability check box
void ConfiguratorWindow::setRemoteWakeUpCapableEnabled(bool value)
{
    ui->checkBoxRemoteWakeUpCapable->setEnabled(value);
}

// Enables or disables the VID field
void ConfiguratorWindow::setVIDEnabled(bool value)
{
    ui->lineEditVID->setReadOnly(!value);
}
