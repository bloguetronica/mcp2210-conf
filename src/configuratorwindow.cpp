/* MCP2210 Configurator - Version 1.0.0 for Debian Linux
   Copyright (c) 2023-2024 Samuel Lourenço

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
#include <cstring>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaObject>
#include <QRegExp>
#include <QRegExpValidator>
#include "common.h"
#include "configurationreader.h"
#include "configurationwriter.h"
#include "mcp2210limits.h"
#include "passworddialog.h"
#include "configuratorwindow.h"
#include "ui_configuratorwindow.h"

ConfiguratorWindow::ConfiguratorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConfiguratorWindow)
{
    ui->setupUi(this);
    ui->lineEditVID->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditPID->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditMaxPower->setValidator(new QRegExpValidator(QRegExp("[\\d]+"), this));
    ui->lineEditMaxPowerHex->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditNewPassword->setValidator(new QRegExpValidator(QRegExp("[!-~]+"), this));  // All printable ASCII characters except space
    ui->lineEditRepeatPassword->setValidator(new QRegExpValidator(QRegExp("[!-~]+"), this));  // Same as above
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
        err_ = false;
        serialstr_ = serialstr;  // Pass the serial number
        readDeviceConfiguration();
        if (err_) {
            handleError();
            this->deleteLater();  // Close window after the subsequent show() call
        } else {
            this->setWindowTitle(tr("MCP2210 Device (S/N: %1)").arg(serialstr_));
            displayConfiguration(deviceConfig_);
            viewEnabled_ = true;
        }
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
    showAboutDialog();  // See "common.h" and "common.cpp"
}

void ConfiguratorWindow::on_actionLoadConfiguration_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Configuration from File"), filePath, tr("XML files (*.xml);;All files (*)"));
    if (!fileName.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not read from %1.\n\nPlease verify that you have read access to this file.").arg(QDir::toNativeSeparators(fileName)));
        } else {
            loadConfigurationFromFile(file);
            file.close();
            filePath = fileName;
        }
    }
}

void ConfiguratorWindow::on_actionReadEEPROM_triggered()
{
    // TODO
}

void ConfiguratorWindow::on_actionSaveConfiguration_triggered()
{
    if(showInvalidInput()) {
        QMessageBox::critical(this, tr("Error"), tr("One or more fields have invalid information.\n\nPlease correct the information in the fields highlighted in red."));
    } else {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Configuration to File"), filePath, tr("XML files (*.xml);;All files (*)"));
        if (!fileName.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::critical(this, tr("Error"), tr("Could not write to %1.\n\nPlease verify that you have write access to this file.").arg(QDir::toNativeSeparators(fileName)));
            } else {
                saveConfigurationToFile(file);
                file.close();
                filePath = fileName;
            }
        }
    }
}

void ConfiguratorWindow::on_actionStatus_triggered()
{
    if (statusDialog_.isNull()) {  // If the dialog is not open
        err_ = false;
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

void ConfiguratorWindow::on_actionUsePassword_triggered()
{
    PasswordDialog passwordDialog(this);
    if (passwordDialog.exec() == QDialog::Accepted) {
        err_ = false;
        int errcnt = 0;
        QString errstr;
        quint8 response = mcp2210_.usePassword(passwordDialog.passwordLineEditText(), errcnt, errstr);
        opCheck(tr("use-password-op"), errcnt, errstr);  // The string "use-password-op" should be translated to "Use password"
        if (err_) {
            handleError();
        } else if (response == MCP2210::COMPLETED) {  // If error check passes and password is verified
            // TODO Disable "Use Password" action?
            QMessageBox::information(this, tr("Access Granted"), tr("The password was sucessfully entered and full write access to the NVRAM is now granted."));
        } else if (response == MCP2210::BLOCKED) {  // If error check passes and access is blocked
            // TODO Disable "Write" button?
            QMessageBox::warning(this, tr("Access Blocked"), tr("The password was not accepted and access is temporarily blocked. Please disconnect and reconnect your device, and try again."));
        } else if (response == MCP2210::REJECTED) {  // If error check passes and access is somehow rejected
            // TODO Disable "Write" button?
            QMessageBox::warning(this, tr("Access Rejected"), tr("Full write access to the NVRAM was rejected for unknown reasons."));
        } else if (response == MCP2210::WRONG_PASSWORD) {  // If error check passes and password is not verified
            QMessageBox::warning(this, tr("Access Denied"), tr("The password was not accepted. Please try again."));
        }
    }
}

void ConfiguratorWindow::on_actionVerifyEEPROM_triggered()
{
    // TODO
}

void ConfiguratorWindow::on_actionWriteEEPROM_triggered()
{
    // TODO
}

void ConfiguratorWindow::on_checkBoxDoNotChangePassword_stateChanged(int state)
{
    if (state == Qt::Checked) {
        ui->lineEditNewPassword->clear();
        ui->lineEditRepeatPassword->clear();
    }
    ui->lineEditNewPassword->setEnabled(state == Qt::Unchecked);
    ui->pushButtonRevealNewPassword->setEnabled(state == Qt::Unchecked && !ui->lineEditNewPassword->text().isEmpty());
    ui->lineEditRepeatPassword->setEnabled(state == Qt::Unchecked);
    ui->pushButtonRevealRepeatPassword->setEnabled(state == Qt::Unchecked && !ui->lineEditRepeatPassword->text().isEmpty());
}

void ConfiguratorWindow::on_lineEditManufacturer_textEdited(QString text)  // The variable "text" is passed by value here, because it needs to be modified locally!
{
    int curPosition = ui->lineEditManufacturer->cursorPosition();
    ui->lineEditManufacturer->setText(text.replace('\n', ' '));
    ui->lineEditManufacturer->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_lineEditMaxPower_editingFinished()
{
    ui->lineEditMaxPower->setText(QString::number(2 * (ui->lineEditMaxPower->text().toInt() / 2)));  // This removes any leading zeros and also rounds to the previous even number, if the value is odd
}

void ConfiguratorWindow::on_lineEditMaxPower_textChanged(const QString &text)
{
    if (text.isEmpty()) {
        ui->lineEditMaxPower->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditMaxPower->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditMaxPower_textEdited(QString text)  // The variable "text" is passed by value here, because it needs to be modified locally!
{
    int maxPower = text.toInt();
    if (maxPower > 2 * MCP2210Limits::MAXPOW_MAX) {
        text.chop(1);
        ui->lineEditMaxPower->setText(text);
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

void ConfiguratorWindow::on_lineEditMaxPowerHex_textChanged(const QString &text)
{
    if (text.isEmpty()) {
        ui->lineEditMaxPowerHex->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditMaxPowerHex->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditMaxPowerHex_textEdited(const QString &text)
{
    int curPosition = ui->lineEditMaxPowerHex->cursorPosition();
    ui->lineEditMaxPowerHex->setText(text.toLower());
    int maxPowerHex = text.toInt(nullptr, 16);
    if (maxPowerHex > MCP2210Limits::MAXPOW_MAX) {
        maxPowerHex = MCP2210Limits::MAXPOW_MAX;
        ui->lineEditMaxPowerHex->setText(QString("%1").arg(MCP2210Limits::MAXPOW_MAX, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
    }
    ui->lineEditMaxPowerHex->setCursorPosition(curPosition);
    ui->lineEditMaxPower->setText(QString::number(2 * maxPowerHex));
}

void ConfiguratorWindow::on_lineEditNewPassword_textChanged(const QString &text)
{
    ui->pushButtonRevealNewPassword->setEnabled(!text.isEmpty());
}

void ConfiguratorWindow::on_lineEditPID_textChanged(const QString &text)
{
    if (text.size() < 4 || text == "0000") {
        ui->lineEditPID->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditPID->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditPID_textEdited(const QString &text)
{
    int curPosition = ui->lineEditPID->cursorPosition();
    ui->lineEditPID->setText(text.toLower());
    ui->lineEditPID->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_lineEditProduct_textEdited()
{
    int curPosition = ui->lineEditProduct->cursorPosition();
    ui->lineEditProduct->setText(ui->lineEditProduct->text().replace('\n', ' '));
    ui->lineEditProduct->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_lineEditRepeatPassword_textChanged(const QString &text)
{
    ui->pushButtonRevealRepeatPassword->setEnabled(!text.isEmpty());
}

void ConfiguratorWindow::on_lineEditVID_textChanged(const QString &text)
{
    if (text.size() < 4 || text == "0000") {
        ui->lineEditVID->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditVID->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditVID_textEdited(const QString &text)
{
    int curPosition = ui->lineEditVID->cursorPosition();
    ui->lineEditVID->setText(text.toLower());
    ui->lineEditVID->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_pushButtonRevealNewPassword_pressed()
{
    ui->lineEditNewPassword->setEchoMode(QLineEdit::Normal);
}

void ConfiguratorWindow::on_pushButtonRevealNewPassword_released()
{
    ui->lineEditNewPassword->setEchoMode(QLineEdit::Password);
}

void ConfiguratorWindow::on_pushButtonRevealRepeatPassword_pressed()
{
    ui->lineEditRepeatPassword->setEchoMode(QLineEdit::Normal);
}

void ConfiguratorWindow::on_pushButtonRevealRepeatPassword_released()
{
    ui->lineEditRepeatPassword->setEchoMode(QLineEdit::Password);
}

void ConfiguratorWindow::on_pushButtonRevert_clicked()
{
    displayConfiguration(deviceConfig_);
}

void ConfiguratorWindow::on_pushButtonWrite_clicked()
{
    if (mcp2210_.isOpen()) {  // It is important to check if the device is open, since resetDevice() is non-blocking (a device reset could still be underway)
        if(showInvalidInput()) {
            QMessageBox::critical(this, tr("Error"), tr("One or more fields have invalid information.\n\nPlease correct the information in the fields highlighted in red."));
        } else {
            getEditedConfiguration();
            if (editedConfig_ == deviceConfig_) {
                QMessageBox::information(this, tr("No Changes Done"), tr("No changes were effected, because no values were modified."));
            } else {
                int qmret = QMessageBox::question(this, tr("Write Configuration?"), tr("This will write the changes to the OTP ROM of your device. These changes will be permanent.\n\nDo you wish to proceed?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (qmret == QMessageBox::Yes) {
                    configureDevice();
                }
            }
        }
    }
}

void ConfiguratorWindow::on_radioButtonPasswordProtected_toggled(bool checked)
{
    if (checked == false) {
        ui->checkBoxDoNotChangePassword->setChecked(false);
        ui->lineEditNewPassword->clear();
        ui->lineEditRepeatPassword->clear();
    }
    ui->checkBoxDoNotChangePassword->setEnabled(checked && accessMode_ == MCP2210::ACPASSWORD);
    ui->lineEditNewPassword->setEnabled(checked && !ui->checkBoxDoNotChangePassword->isChecked());
    ui->pushButtonRevealNewPassword->setEnabled(checked && !ui->checkBoxDoNotChangePassword->isChecked() && !ui->lineEditNewPassword->text().isEmpty());
    ui->lineEditRepeatPassword->setEnabled(checked && !ui->checkBoxDoNotChangePassword->isChecked());
    ui->pushButtonRevealRepeatPassword->setEnabled(checked && !ui->checkBoxDoNotChangePassword->isChecked() && !ui->lineEditNewPassword->text().isEmpty());
}

// Verifies the MCP2210 configuration against the input configuration
void ConfiguratorWindow::verifyConfiguration()
{
    readDeviceConfiguration();
    if (!err_) {
        displayConfiguration(deviceConfig_);
        if (deviceConfig_ != editedConfig_) {
            err_ = true;
            errmsg_ = tr("Failed verification.");
        }
    }
}

// Writes the manufacturer descriptor to the MCP2210 NVRAM
void ConfiguratorWindow::writeManufacturerDesc()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.writeManufacturerDesc(editedConfig_.manufacturer, errcnt, errstr);
    opCheck(tr("write-manufacturer-desc-op"), errcnt, errstr);  // The string "write-manufacturer-desc-op" should be translated to "Write manufacturer descriptor"
}

// Writes the product descriptor to the MCP2210 NVRAM
void ConfiguratorWindow::writeProductDesc()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.writeProductDesc(editedConfig_.product, errcnt, errstr);
    opCheck(tr("write-product-desc-op"), errcnt, errstr);  // The string "write-product-desc-op" should be translated to "Write product descriptor"
}

// Writes the USB parameters to the MCP2210 NVRAM
void ConfiguratorWindow::writeUSBParameters()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.writeUSBParameters(editedConfig_.usbparameters, errcnt, errstr);
    opCheck(tr("write-usb-parameters-op"), errcnt, errstr);  // The string "write-usb-parameters-op" should be translated to "Write USB parameters"
}

// This is the main configuration routine, used to configure the MCP2210 NVRAM according to the tasks in the task list
void ConfiguratorWindow::configureDevice()
{
    err_ = false;
    QStringList tasks = prepareTaskList();  // Create a new task list
    int nTasks = tasks.size();
    for (int i = 0; i < nTasks; ++i) {  // Iterate through the newly created task list
        QMetaObject::invokeMethod(this, tasks[i].toStdString().c_str());  // The task list entry is converted to a C string
        if (err_) {  // If an error has occured
            break;  // Abort the configuration
        }
    }
    if (err_) {  // If an error occured
        handleError();
        QMessageBox::critical(this, tr("Error"), tr("The device configuration could not be completed."));
    } else {  // Successul configuration
        QMessageBox::information(this, tr("Device Configured"), tr("Device was successfully configured."));
    }
}

// Partially disables configurator window
void ConfiguratorWindow::disableView()
{
    ui->actionStatus->setEnabled(false);
    ui->actionLoadConfiguration->setEnabled(false);
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
    ui->comboBoxGP8->setCurrentIndex(chipsettings.gp8);
    ui->comboBoxInterruptMode->setCurrentIndex(chipsettings.intmode);
    ui->checkBoxRemoteWakeUp->setChecked(chipsettings.rmwakeup);
    ui->checkBoxSPIBusCaptive->setChecked(chipsettings.nrelspi);
}

// This is the main display routine, used to display the given configuration, updating all fields accordingly
void ConfiguratorWindow::displayConfiguration(const Configuration &config)
{
    setUsePasswordEnabled(accessMode_ == MCP2210::ACPASSWORD);
    displayManufacturer(config.manufacturer);
    setManufacturerEnabled(accessMode_ != MCP2210::ACLOCKED);
    displayProduct(config.product);
    setProductEnabled(accessMode_ != MCP2210::ACLOCKED);
    displayUSBParameters(config.usbparameters);
    setVIDEnabled(accessMode_ != MCP2210::ACLOCKED);
    setPIDEnabled(accessMode_ != MCP2210::ACLOCKED);
    setMaxPowerEnabled(accessMode_ != MCP2210::ACLOCKED);
    setPowerModeEnabled(accessMode_ != MCP2210::ACLOCKED);
    setRemoteWakeUpCapableEnabled(accessMode_ != MCP2210::ACLOCKED);
    displayNVRAMAccessMode();
    setNVRAMAccessModeEnabled(accessMode_ != MCP2210::ACLOCKED);
    displayChipSettings(config.chipsettings);
    setChipSettingsEnabled(accessMode_ != MCP2210::ACLOCKED);
    displaySPISettings(config.spisettings);
    setSPISettingsEnabled(accessMode_ != MCP2210::ACLOCKED);
    setWriteEnabled(accessMode_ != MCP2210::ACLOCKED);
}

// Updates the manufacturer descriptor field
void ConfiguratorWindow::displayManufacturer(const QString &manufacturer)
{
    ui->lineEditManufacturer->setText(manufacturer);
}

// Updates controls inside the "NVRAM Access Mode" group box
void ConfiguratorWindow::displayNVRAMAccessMode()
{
    switch (accessMode_) {
        case MCP2210::ACPASSWORD:
            ui->radioButtonPasswordProtected->setChecked(true);
            ui->checkBoxDoNotChangePassword->setChecked(true);
            ui->checkBoxDoNotChangePassword->setEnabled(true);
            ui->lineEditNewPassword->setEnabled(false);
            ui->lineEditRepeatPassword->setEnabled(false);
            break;
        case MCP2210::ACLOCKED:
            ui->radioButtonPermanentlyLocked->setChecked(true);
            break;
        default:
            ui->radioButtonFullAccess->setChecked(true);
    }
}

// Updates the product descriptor field
void ConfiguratorWindow::displayProduct(const QString &product)
{
    ui->lineEditProduct->setText(product);
}

// Updates all fields pertaining to SPI settings
void ConfiguratorWindow::displaySPISettings(const MCP2210::SPISettings &spisettings)
{
    ui->doubleSpinBoxBitRate->setValue(spisettings.bitrate / 1000.0);
    ui->spinBoxSPIMode->setValue(spisettings.mode);
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

// Retrieves the user set configuration from the fields
void ConfiguratorWindow::getEditedConfiguration()
{
    editedConfig_.manufacturer = ui->lineEditManufacturer->text();
    editedConfig_.product = ui->lineEditProduct->text();
    editedConfig_.usbparameters.vid = static_cast<quint16>(ui->lineEditVID->text().toUShort(nullptr, 16));  // Cast done for sanity purposes
    editedConfig_.usbparameters.pid = static_cast<quint16>(ui->lineEditPID->text().toUShort(nullptr, 16));  // Cast done for sanity purposes
    editedConfig_.usbparameters.maxpow = static_cast<quint8>(ui->lineEditMaxPowerHex->text().toUShort(nullptr, 16));
    editedConfig_.usbparameters.powmode = static_cast<quint8>(ui->comboBoxPowerMode->currentIndex());
    editedConfig_.usbparameters.rmwakeup = ui->checkBoxRemoteWakeUpCapable->isChecked();
    editedConfig_.chipsettings.gp0 = static_cast<quint8>(ui->comboBoxGP0->currentIndex() > 0 ? ui->comboBoxGP0->currentIndex() - 1 : 0);
    editedConfig_.chipsettings.gp1 = static_cast<quint8>(ui->comboBoxGP1->currentIndex() > 0 ? ui->comboBoxGP1->currentIndex() - 1 : 0);
    editedConfig_.chipsettings.gp2 = static_cast<quint8>(ui->comboBoxGP2->currentIndex() > 0 ? ui->comboBoxGP2->currentIndex() - 1 : 0);
    editedConfig_.chipsettings.gp3 = static_cast<quint8>(ui->comboBoxGP3->currentIndex() > 0 ? ui->comboBoxGP3->currentIndex() - 1 : 0);
    editedConfig_.chipsettings.gp4 = static_cast<quint8>(ui->comboBoxGP4->currentIndex() > 0 ? ui->comboBoxGP4->currentIndex() - 1 : 0);
    editedConfig_.chipsettings.gp5 = static_cast<quint8>(ui->comboBoxGP5->currentIndex() > 0 ? ui->comboBoxGP5->currentIndex() - 1 : 0);
    editedConfig_.chipsettings.gp6 = static_cast<quint8>(ui->comboBoxGP6->currentIndex() > 0 ? ui->comboBoxGP6->currentIndex() - 1 : 0);
    editedConfig_.chipsettings.gp7 = static_cast<quint8>(ui->comboBoxGP7->currentIndex() > 0 ? ui->comboBoxGP7->currentIndex() - 1 : 0);
    editedConfig_.chipsettings.gp8 = static_cast<quint8>(ui->comboBoxGP8->currentIndex());
    editedConfig_.chipsettings.gpdir = static_cast<quint8>((ui->comboBoxGP7->currentIndex() != 1) << 7 |  // All pins have their direction set to input by default, except those pins that are specifically set to be outputs
                                                           (ui->comboBoxGP6->currentIndex() != 1) << 6 |
                                                           (ui->comboBoxGP5->currentIndex() != 1) << 5 |
                                                           (ui->comboBoxGP4->currentIndex() != 1) << 4 |
                                                           (ui->comboBoxGP3->currentIndex() != 1) << 3 |
                                                           (ui->comboBoxGP2->currentIndex() != 1) << 2 |
                                                           (ui->comboBoxGP1->currentIndex() != 1) << 1 |
                                                           (ui->comboBoxGP0->currentIndex() != 1));
    editedConfig_.chipsettings.gpout = static_cast<quint8>(ui->checkBoxGP7DefaultValue->isChecked() << 7 |
                                                           ui->checkBoxGP6DefaultValue->isChecked() << 6 |
                                                           ui->checkBoxGP5DefaultValue->isChecked() << 5 |
                                                           ui->checkBoxGP4DefaultValue->isChecked() << 4 |
                                                           ui->checkBoxGP3DefaultValue->isChecked() << 3 |
                                                           ui->checkBoxGP2DefaultValue->isChecked() << 2 |
                                                           ui->checkBoxGP1DefaultValue->isChecked() << 1 |
                                                           ui->checkBoxGP0DefaultValue->isChecked());
    editedConfig_.chipsettings.rmwakeup = ui->checkBoxRemoteWakeUp->isChecked();
    editedConfig_.chipsettings.intmode = static_cast<quint8>(ui->comboBoxInterruptMode->currentIndex());
    editedConfig_.chipsettings.nrelspi = ui->checkBoxSPIBusCaptive->isChecked();
    // TODO
    editedConfig_.spisettings.nbytes = deviceConfig_.spisettings.nbytes;  // TODO To delete
    editedConfig_.spisettings.bitrate = deviceConfig_.spisettings.bitrate;  // TODO To delete
    editedConfig_.spisettings.mode = deviceConfig_.spisettings.mode;  // TODO To delete
    editedConfig_.spisettings.actcs = deviceConfig_.spisettings.actcs;  // TODO To delete
    editedConfig_.spisettings.idlcs = deviceConfig_.spisettings.idlcs;  // TODO To delete
    editedConfig_.spisettings.csdtdly = deviceConfig_.spisettings.csdtdly;  // TODO To delete
    editedConfig_.spisettings.dtcsdly = deviceConfig_.spisettings.dtcsdly;  // TODO To delete
    editedConfig_.spisettings.itbytdly = deviceConfig_.spisettings.itbytdly;  // TODO To delete
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

// Loads the configuration from a given file
void ConfiguratorWindow::loadConfigurationFromFile(QFile &file)
{
    getEditedConfiguration();
    ConfigurationReader configReader(editedConfig_);
    if (!configReader.readFrom(&file)) {
        QMessageBox::critical(this, tr("Error"), configReader.errorString());
    } else {
        displayConfiguration(editedConfig_);
    }
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

// Prepares the task list, by checking which fields changed, while also setting optional tasks according to the user's requirements
QStringList ConfiguratorWindow::prepareTaskList()
{
    QStringList tasks;
    if (editedConfig_.manufacturer != deviceConfig_.manufacturer) {
        tasks += "writeManufacturerDesc";
    }
    if (editedConfig_.product != deviceConfig_.product) {
        tasks += "writeProductDesc";
    }
    if (editedConfig_.usbparameters != deviceConfig_.usbparameters) {
        tasks += "writeUSBParameters";
    }
    // TODO
    tasks += "verifyConfiguration";
    return tasks;
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
    deviceConfig_.spisettings = mcp2210_.getNVSPISettings(errcnt, errstr);
    accessMode_ = mcp2210_.getAccessControlMode(errcnt, errstr);
    opCheck(tr("read-op"), errcnt, errstr);  // The string "read-op" should be translated to "Read"
}

// Saves the current configuration to a given file
void ConfiguratorWindow::saveConfigurationToFile(QFile &file)
{
    getEditedConfiguration();
    ConfigurationWriter configWriter(editedConfig_);
    configWriter.writeTo(&file);
}

// Enables or disables all fields pertaining to the MCP2210 chip settings
void ConfiguratorWindow::setChipSettingsEnabled(bool value)
{
    ui->comboBoxGP0->setEnabled(value);
    ui->checkBoxGP0DefaultValue->setEnabled(value);
    ui->comboBoxGP1->setEnabled(value);
    ui->checkBoxGP1DefaultValue->setEnabled(value);
    ui->comboBoxGP2->setEnabled(value);
    ui->checkBoxGP2DefaultValue->setEnabled(value);
    ui->comboBoxGP3->setEnabled(value);
    ui->checkBoxGP3DefaultValue->setEnabled(value);
    ui->comboBoxGP4->setEnabled(value);
    ui->checkBoxGP4DefaultValue->setEnabled(value);
    ui->comboBoxGP5->setEnabled(value);
    ui->checkBoxGP5DefaultValue->setEnabled(value);
    ui->comboBoxGP6->setEnabled(value);
    ui->checkBoxGP6DefaultValue->setEnabled(value);
    ui->comboBoxGP7->setEnabled(value);
    ui->checkBoxGP7DefaultValue->setEnabled(value);
    ui->comboBoxGP8->setEnabled(value);
    ui->comboBoxInterruptMode->setEnabled(value);
    ui->checkBoxRemoteWakeUp->setEnabled(value);
    ui->checkBoxSPIBusCaptive->setEnabled(value);
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

// Enables or disables the "NVRAM Access Mode" group box
void ConfiguratorWindow::setNVRAMAccessModeEnabled(bool value)
{
    ui->groupBoxNVRAMAccessMode->setEnabled(value);
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

// Enables or disables all fields pertaining to SPI settings
void ConfiguratorWindow::setSPISettingsEnabled(bool value)
{
    ui->doubleSpinBoxBitRate->setEnabled(value);
    ui->spinBoxSPIMode->setEnabled(value);
}

// Enables or disables the "Use Password" menu option (File > Use Password)
void ConfiguratorWindow::setUsePasswordEnabled(bool value)
{
    ui->actionUsePassword->setEnabled(value);
}

// Enables or disables the VID field
void ConfiguratorWindow::setVIDEnabled(bool value)
{
    ui->lineEditVID->setReadOnly(!value);
}

// Enables or disables editing related actions, buttons and checkboxes
void ConfiguratorWindow::setWriteEnabled(bool value)
{
    ui->actionLoadConfiguration->setEnabled(value);
    ui->pushButtonRevert->setEnabled(value);
    ui->checkBoxApplyImmediately->setEnabled(value);
    ui->pushButtonWrite->setEnabled(value);
}

// Checks user input, returning false if it is valid, or true otherwise, while also highlighting invalid fields
bool ConfiguratorWindow::showInvalidInput()
{
    bool retval = false;
    if (ui->lineEditVID->text().size() < 4 || ui->lineEditVID->text() == "0000") {
        ui->lineEditVID->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditPID->text().size() < 4 || ui->lineEditPID->text() == "0000") {
        ui->lineEditPID->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditMaxPower->text().isEmpty()) {
        ui->lineEditMaxPower->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditMaxPowerHex->text().isEmpty()) {
        ui->lineEditMaxPowerHex->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    return retval;
}
