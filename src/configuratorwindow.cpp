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
        readDeviceConfiguration();
        if (err_) {  // If an error has occured
            handleError();
            this->deleteLater();  // Close window after the subsequent show() call
        } else {  // Device is now open
            this->setWindowTitle(tr("MCP2210 Device (S/N: %1)").arg(serialstr));
            displayConfiguration(deviceConfiguration_, true);
            serialstr_ = serialstr;  // Pass the serial number
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

// Applies the chip settings to the MCP2210 volatile memory area
void ConfiguratorWindow::applyChipSettings()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.configureChipSettings(editedConfiguration_.chipSettings, errcnt, errstr);
    validateOperation(tr("apply chip settings"), errcnt, errstr);
}

// Applies the SPI settings to the MCP2210 volatile memory area
void ConfiguratorWindow::applySPISettings()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.configureSPISettings(editedConfiguration_.spiSettings, errcnt, errstr);
    validateOperation(tr("apply SPI settings"), errcnt, errstr);
}

void ConfiguratorWindow::on_actionAbout_triggered()
{
    showAboutDialog();  // See "common.h" and "common.cpp"
}

void ConfiguratorWindow::on_actionLoadConfiguration_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Configuration from File"), xmlFilePath, tr("XML files (*.xml);;All files (*)"));
    if (!fileName.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not read from %1.\n\nPlease verify that you have read access to this file.").arg(QDir::toNativeSeparators(fileName)));
        } else {
            loadConfigurationFromFile(file);
            file.close();
            xmlFilePath = fileName;
        }
    }
}

void ConfiguratorWindow::on_actionReadEEPROM_triggered()
{
    err_ = false;
    MCP2210EEPROM eeprom = readEEPROM();
    if (err_) {  // If an error has occured
        handleError();
    } else {  // Successful read
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save EEPROM Contents to File"), binFilePath, tr("Binary files (*.bin);;All files (*)"));
        if (!fileName.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::critical(this, tr("Error"), tr("Could not write to %1.\n\nPlease verify that you have write access to this file.").arg(QDir::toNativeSeparators(fileName)));
            } else {
                QDataStream out(&file);
                out << eeprom;
                file.close();
                binFilePath = fileName;
            }
        }
    }
}

void ConfiguratorWindow::on_actionSaveConfiguration_triggered()
{
    if(showInvalidInput()) {
        QMessageBox::critical(this, tr("Error"), tr("One or more fields have invalid information.\n\nPlease correct the information in the fields highlighted in red."));
    } else {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Configuration to File"), xmlFilePath, tr("XML files (*.xml);;All files (*)"));
        if (!fileName.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::critical(this, tr("Error"), tr("Could not write to %1.\n\nPlease verify that you have write access to this file.").arg(QDir::toNativeSeparators(fileName)));
            } else {
                saveConfigurationToFile(file);
                file.close();
                xmlFilePath = fileName;
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
        MCP2210::ChipStatus chipStatus = mcp2210_.getChipStatus(errcnt, errstr);
        validateOperation(tr("retrieve device status"), errcnt, errstr);
        if (err_) {  // If an error has occured
            handleError();
        } else {  // Success
            statusDialog_ = new StatusDialog(this);
            statusDialog_->setAttribute(Qt::WA_DeleteOnClose);  // It is important to delete the dialog in memory once closed, in order to force the application to retrieve the device status if the window is opened again???
            statusDialog_->setWindowTitle(tr("Device Status (S/N: %1)").arg(serialstr_));
            statusDialog_->setBusRequestValueLabelText(chipStatus.busreq);
            statusDialog_->setBusOwnerValueLabelText(chipStatus.busowner);
            statusDialog_->setPasswordStatusValueLabelText(chipStatus.pwok);
            statusDialog_->setPasswordTriesValueLabelText(chipStatus.pwtries);
            statusDialog_->show();
        }
    } else {
        statusDialog_->showNormal();  // Required if the dialog is minimized
        statusDialog_->activateWindow();  // Set focus on the previous dialog (dialog is raised and selected)
    }
}

void ConfiguratorWindow::on_actionUsePassword_triggered()
{
    if (validatePassword()) {
        QMessageBox::information(this, tr("Access Granted"), tr("The password was successfully entered and full write access to the NVRAM is now granted."));
    }
}

void ConfiguratorWindow::on_actionVerifyEEPROM_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Verify EEPROM Contents against File"), binFilePath, tr("Binary files (*.bin);;All files (*)"));
    if (!fileName.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not read from %1.\n\nPlease verify that you have read access to this file.").arg(QDir::toNativeSeparators(fileName)));
        } else if (file.bytesAvailable() != MCP2210::EEPROM_SIZE) {
            QMessageBox::critical(this, tr("Error"), tr("The selected file is not a valid MCP2210 EEPROM binary file."));
        } else {
            MCP2210EEPROM eeprom, eepromFromFile;
            QDataStream in(&file);
            in >> eepromFromFile;
            file.close();
            binFilePath = fileName;
            err_ = false;
            eeprom = readEEPROM();
            if (err_) {  // If an error has occured
                handleError();
            } else if (eeprom == eepromFromFile) {  // Non-error cases
                QMessageBox::information(this, tr("EEPROM Verified"), tr("EEPROM was successfully verified."));
            } else {
                QMessageBox::warning(this, tr("EEPROM Mismatch"), tr("Verification failed because EEPROM contents don't match file contents."));
            }
        }
    }
}

void ConfiguratorWindow::on_actionWriteEEPROM_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load EEPROM Contents from File"), binFilePath, tr("Binary files (*.bin);;All files (*)"));
    if (!fileName.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not read from %1.\n\nPlease verify that you have read access to this file.").arg(QDir::toNativeSeparators(fileName)));
        } else if (file.bytesAvailable() != MCP2210::EEPROM_SIZE) {
            QMessageBox::critical(this, tr("Error"), tr("The selected file is not a valid MCP2210 EEPROM binary file."));
        } else {
            MCP2210EEPROM eepromFromFile;
            QDataStream in(&file);
            in >> eepromFromFile;
            file.close();
            binFilePath = fileName;
            err_ = false;
            writeEEPROM(eepromFromFile);
            if (err_) {  // If an error has occured
                handleError();
            } else {  // Success
                QMessageBox::information(this, tr("EEPROM Written"), tr("EEPROM was successfully written."));
            }
        }
    }
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

void ConfiguratorWindow::on_doubleSpinBoxBitRate_editingFinished()
{
    err_ = false;
    ui->doubleSpinBoxBitRate->setValue(getNearestCompatibleBitRate(static_cast<quint32>(1000 * ui->doubleSpinBoxBitRate->value())) / 1000.0);  // Note that getNearestCompatibleBitRate() is guaranteed to return a valid bit rate value
    if (err_) {  // If an error has occured
        handleError();
    }
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
    if (ui->lineEditNewPassword->text() != ui->lineEditRepeatPassword->text()) {
        ui->lineEditNewPassword->setStyleSheet("background: rgb(255, 204, 0);");
        ui->lineEditRepeatPassword->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditNewPassword->setStyleSheet("");
        ui->lineEditRepeatPassword->setStyleSheet("");
    }
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
    if (ui->lineEditNewPassword->text() != ui->lineEditRepeatPassword->text()) {
        ui->lineEditNewPassword->setStyleSheet("background: rgb(255, 204, 0);");
        ui->lineEditRepeatPassword->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditNewPassword->setStyleSheet("");
        ui->lineEditRepeatPassword->setStyleSheet("");
    }
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
    displayConfiguration(deviceConfiguration_, false);
}

void ConfiguratorWindow::on_pushButtonWrite_clicked()
{
    if (mcp2210_.isOpen()) {  // It is important to check if the device is open, since resetDevice() is non-blocking (a device reset could still be underway)
        if(showInvalidInput()) {
            QMessageBox::critical(this, tr("Error"), tr("One or more fields have invalid information.\n\nPlease correct the information in the fields highlighted in red."));
        } else {
            getEditedConfiguration();
            if (editedConfiguration_ == deviceConfiguration_ && (editedConfiguration_.accessMode != MCP2210::ACPASSWORD || ui->checkBoxDoNotChangePassword->isChecked())) {
                QMessageBox::information(this, tr("No Changes Done"), tr("No changes were effected, because no values were modified."));
            } else {
                int qmret = QMessageBox::question(this, tr("Write Configuration?"), tr("This will write the changes to the NVRAM of your device.\n\nDo you wish to proceed?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (qmret == QMessageBox::Yes && (deviceConfiguration_.accessMode == MCP2210::ACNONE || (deviceConfiguration_.accessMode == MCP2210::ACPASSWORD && (passwordIsValid_ || validatePassword())))) {
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
    ui->checkBoxDoNotChangePassword->setChecked(checked && deviceConfiguration_.accessMode == MCP2210::ACPASSWORD);
    ui->checkBoxDoNotChangePassword->setEnabled(checked && deviceConfiguration_.accessMode == MCP2210::ACPASSWORD);
    ui->lineEditNewPassword->setEnabled(checked && !ui->checkBoxDoNotChangePassword->isChecked());
    ui->pushButtonRevealNewPassword->setEnabled(checked && !ui->checkBoxDoNotChangePassword->isChecked() && !ui->lineEditNewPassword->text().isEmpty());
    ui->lineEditRepeatPassword->setEnabled(checked && !ui->checkBoxDoNotChangePassword->isChecked());
    ui->pushButtonRevealRepeatPassword->setEnabled(checked && !ui->checkBoxDoNotChangePassword->isChecked() && !ui->lineEditNewPassword->text().isEmpty());
}

void ConfiguratorWindow::on_spinBoxCPHA_valueChanged(int i)
{
    ui->spinBoxMode->setValue(2 * ui->spinBoxCPOL->value() + i);
}

void ConfiguratorWindow::on_spinBoxCPOL_valueChanged(int i)
{
    ui->spinBoxMode->setValue(2 * i + ui->spinBoxCPHA->value());
}

void ConfiguratorWindow::on_spinBoxMode_valueChanged(int i)
{
    ui->spinBoxCPOL->setValue(i / 2);
    ui->spinBoxCPHA->setValue(i % 2);
}

// Verifies the MCP2210 configuration against the input configuration
void ConfiguratorWindow::verifyConfiguration()
{
    readDeviceConfiguration();
    if (!err_) {
        displayConfiguration(deviceConfiguration_, true);
        if (deviceConfiguration_ != editedConfiguration_) {
            err_ = true;
            errmsg_ = tr("Failed verification.");
        }
    }
}

// Writes the chip settings to the MCP2210 NVRAM
void ConfiguratorWindow::writeChipSettings()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.writeNVChipSettings(editedConfiguration_.chipSettings, MCP2210::ACNONE, "", errcnt, errstr);  // TODO Implement password protection
    validateOperation(tr("write chip settings"), errcnt, errstr);
}

// Writes the manufacturer descriptor to the MCP2210 NVRAM
void ConfiguratorWindow::writeManufacturerDesc()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.writeManufacturerDesc(editedConfiguration_.manufacturer, errcnt, errstr);
    validateOperation(tr("write manufacturer desc"), errcnt, errstr);
}

// Writes the product descriptor to the MCP2210 NVRAM
void ConfiguratorWindow::writeProductDesc()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.writeProductDesc(editedConfiguration_.product, errcnt, errstr);
    validateOperation(tr("write product desc"), errcnt, errstr);
}

// Writes the SPI settings to the MCP2210 NVRAM
void ConfiguratorWindow::writeSPISettings()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.writeNVSPISettings(editedConfiguration_.spiSettings, errcnt, errstr);
    validateOperation(tr("write SPI settings"), errcnt, errstr);
}

// Writes the USB parameters to the MCP2210 NVRAM
void ConfiguratorWindow::writeUSBParameters()
{
    int errcnt = 0;
    QString errstr;
    mcp2210_.writeUSBParameters(editedConfiguration_.usbParameters, errcnt, errstr);
    validateOperation(tr("write USB parameters"), errcnt, errstr);
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
            handleError();
            QMessageBox::critical(this, tr("Error"), tr("The device configuration could not be completed."));
            break;  // Abort the configuration
        }
    }
    if (!err_) {  // Successful configuration
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
void ConfiguratorWindow::displayChipSettings(const MCP2210::ChipSettings &chipSettings)
{
    ui->comboBoxGP0->setCurrentIndex(chipSettings.gp0 == MCP2210::PCGPIO ? (0x01 & chipSettings.gpdir) == 0x00 : chipSettings.gp0 + 1);
    ui->checkBoxGP0DefaultValue->setChecked((0x01 & chipSettings.gpout) != 0x00);
    ui->comboBoxGP1->setCurrentIndex(chipSettings.gp1 == MCP2210::PCGPIO ? (0x02 & chipSettings.gpdir) == 0x00 : chipSettings.gp1 + 1);
    ui->checkBoxGP1DefaultValue->setChecked((0x02 & chipSettings.gpout) != 0x00);
    ui->comboBoxGP2->setCurrentIndex(chipSettings.gp2 == MCP2210::PCGPIO ? (0x04 & chipSettings.gpdir) == 0x00 : chipSettings.gp2 + 1);
    ui->checkBoxGP2DefaultValue->setChecked((0x04 & chipSettings.gpout) != 0x00);
    ui->comboBoxGP3->setCurrentIndex(chipSettings.gp3 == MCP2210::PCGPIO ? (0x08 & chipSettings.gpdir) == 0x00 : chipSettings.gp3 + 1);
    ui->checkBoxGP3DefaultValue->setChecked((0x08 & chipSettings.gpout) != 0x00);
    ui->comboBoxGP4->setCurrentIndex(chipSettings.gp4 == MCP2210::PCGPIO ? (0x10 & chipSettings.gpdir) == 0x00 : chipSettings.gp4 + 1);
    ui->checkBoxGP4DefaultValue->setChecked((0x10 & chipSettings.gpout) != 0x00);
    ui->comboBoxGP5->setCurrentIndex(chipSettings.gp5 == MCP2210::PCGPIO ? (0x20 & chipSettings.gpdir) == 0x00 : chipSettings.gp5 + 1);
    ui->checkBoxGP5DefaultValue->setChecked((0x20 & chipSettings.gpout) != 0x00);
    ui->comboBoxGP6->setCurrentIndex(chipSettings.gp6 == MCP2210::PCGPIO ? (0x40 & chipSettings.gpdir) == 0x00 : chipSettings.gp6 + 1);
    ui->checkBoxGP6DefaultValue->setChecked((0x40 & chipSettings.gpout) != 0x00);
    ui->comboBoxGP7->setCurrentIndex(chipSettings.gp7 == MCP2210::PCGPIO ? (0x80 & chipSettings.gpdir) == 0x00 : chipSettings.gp7 + 1);
    ui->checkBoxGP7DefaultValue->setChecked((0x80 & chipSettings.gpout) != 0x00);
    ui->comboBoxGP8->setCurrentIndex(chipSettings.gp8 == MCP2210::PCGPIO ? 0 : 1);
    ui->comboBoxInterruptMode->setCurrentIndex(chipSettings.intmode);
    ui->checkBoxRemoteWakeUp->setChecked(chipSettings.rmwakeup);
    ui->checkBoxSPIBusCaptive->setChecked(chipSettings.nrelspi);
}

// This is the main display routine, used to display the given configuration, updating all fields accordingly
void ConfiguratorWindow::displayConfiguration(const Configuration &configuration, bool fullUpdate)
{
    displayManufacturer(configuration.manufacturer);
    displayProduct(configuration.product);
    displayUSBParameters(configuration.usbParameters);
    displayNVRAMAccessMode(configuration.accessMode);
    displayChipSettings(configuration.chipSettings);
    displaySPISettings(configuration.spiSettings);
    if (fullUpdate) {
        setUsePasswordEnabled(!passwordIsLocked_ && configuration.accessMode == MCP2210::ACPASSWORD && !passwordIsValid_);
        setGeneralSettingsEnabled(!passwordIsLocked_ && configuration.accessMode != MCP2210::ACLOCKED);
        setChipSettingsEnabled(!passwordIsLocked_ && configuration.accessMode != MCP2210::ACLOCKED);
        setSPISettingsEnabled(!passwordIsLocked_ && configuration.accessMode != MCP2210::ACLOCKED);
        setWriteEnabled(!passwordIsLocked_ && configuration.accessMode != MCP2210::ACLOCKED);
    }
}

// Updates the manufacturer descriptor field
void ConfiguratorWindow::displayManufacturer(const QString &manufacturer)
{
    ui->lineEditManufacturer->setText(manufacturer);
}

// Updates controls inside the "NVRAM Access Mode" group box
void ConfiguratorWindow::displayNVRAMAccessMode(quint8 accessMode)
{
    switch (accessMode) {
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
void ConfiguratorWindow::displaySPISettings(const MCP2210::SPISettings &spiSettings)
{
    ui->spinBoxBytesPerTransaction->setValue(spiSettings.nbytes);
    ui->doubleSpinBoxBitRate->setValue(spiSettings.bitrate / 1000.0);
    ui->spinBoxMode->setValue(spiSettings.mode);
    ui->checkBoxActiveCS0->setChecked((0x01 & spiSettings.actcs) != 0x00);
    ui->checkBoxActiveCS1->setChecked((0x02 & spiSettings.actcs) != 0x00);
    ui->checkBoxActiveCS2->setChecked((0x04 & spiSettings.actcs) != 0x00);
    ui->checkBoxActiveCS3->setChecked((0x08 & spiSettings.actcs) != 0x00);
    ui->checkBoxActiveCS4->setChecked((0x10 & spiSettings.actcs) != 0x00);
    ui->checkBoxActiveCS5->setChecked((0x20 & spiSettings.actcs) != 0x00);
    ui->checkBoxActiveCS6->setChecked((0x40 & spiSettings.actcs) != 0x00);
    ui->checkBoxActiveCS7->setChecked((0x80 & spiSettings.actcs) != 0x00);
    ui->checkBoxIdleCS0->setChecked((0x01 & spiSettings.idlcs) != 0x00);
    ui->checkBoxIdleCS1->setChecked((0x02 & spiSettings.idlcs) != 0x00);
    ui->checkBoxIdleCS2->setChecked((0x04 & spiSettings.idlcs) != 0x00);
    ui->checkBoxIdleCS3->setChecked((0x08 & spiSettings.idlcs) != 0x00);
    ui->checkBoxIdleCS4->setChecked((0x10 & spiSettings.idlcs) != 0x00);
    ui->checkBoxIdleCS5->setChecked((0x20 & spiSettings.idlcs) != 0x00);
    ui->checkBoxIdleCS6->setChecked((0x40 & spiSettings.idlcs) != 0x00);
    ui->checkBoxIdleCS7->setChecked((0x80 & spiSettings.idlcs) != 0x00);
    ui->spinBoxCSToDataDelay->setValue(spiSettings.csdtdly);
    ui->spinBoxDataToCSDelay->setValue(spiSettings.dtcsdly);
    ui->spinBoxInterByteDelay->setValue(spiSettings.itbytdly);
}

// Updates all fields pertaining to USB parameters
void ConfiguratorWindow::displayUSBParameters(const MCP2210::USBParameters &usbParameters)
{
    ui->lineEditVID->setText(QString("%1").arg(usbParameters.vid, 4, 16, QChar('0')));  // This will autofill with up to four leading zeros
    ui->lineEditPID->setText(QString("%1").arg(usbParameters.pid, 4, 16, QChar('0')));  // Same as before
    ui->lineEditMaxPower->setText(QString::number(2 * usbParameters.maxpow));
    ui->lineEditMaxPowerHex->setText(QString("%1").arg(usbParameters.maxpow, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
    ui->comboBoxPowerMode->setCurrentIndex(usbParameters.powmode);
    ui->checkBoxRemoteWakeUpCapable->setChecked(usbParameters.rmwakeup);
}

// Retrieves the user set configuration from the fields
void ConfiguratorWindow::getEditedConfiguration()
{
    editedConfiguration_.manufacturer = ui->lineEditManufacturer->text();
    editedConfiguration_.product = ui->lineEditProduct->text();
    editedConfiguration_.usbParameters.vid = static_cast<quint16>(ui->lineEditVID->text().toUShort(nullptr, 16));  // Cast done for sanity purposes
    editedConfiguration_.usbParameters.pid = static_cast<quint16>(ui->lineEditPID->text().toUShort(nullptr, 16));  // Cast done for sanity purposes
    editedConfiguration_.usbParameters.maxpow = static_cast<quint8>(ui->lineEditMaxPowerHex->text().toUShort(nullptr, 16));
    editedConfiguration_.usbParameters.powmode = static_cast<quint8>(ui->comboBoxPowerMode->currentIndex());
    editedConfiguration_.usbParameters.rmwakeup = ui->checkBoxRemoteWakeUpCapable->isChecked();
    editedConfiguration_.chipSettings.gp0 = static_cast<quint8>(ui->comboBoxGP0->currentIndex() > 0 ? ui->comboBoxGP0->currentIndex() - 1 : MCP2210::PCGPIO);
    editedConfiguration_.chipSettings.gp1 = static_cast<quint8>(ui->comboBoxGP1->currentIndex() > 0 ? ui->comboBoxGP1->currentIndex() - 1 : MCP2210::PCGPIO);
    editedConfiguration_.chipSettings.gp2 = static_cast<quint8>(ui->comboBoxGP2->currentIndex() > 0 ? ui->comboBoxGP2->currentIndex() - 1 : MCP2210::PCGPIO);
    editedConfiguration_.chipSettings.gp3 = static_cast<quint8>(ui->comboBoxGP3->currentIndex() > 0 ? ui->comboBoxGP3->currentIndex() - 1 : MCP2210::PCGPIO);
    editedConfiguration_.chipSettings.gp4 = static_cast<quint8>(ui->comboBoxGP4->currentIndex() > 0 ? ui->comboBoxGP4->currentIndex() - 1 : MCP2210::PCGPIO);
    editedConfiguration_.chipSettings.gp5 = static_cast<quint8>(ui->comboBoxGP5->currentIndex() > 0 ? ui->comboBoxGP5->currentIndex() - 1 : MCP2210::PCGPIO);
    editedConfiguration_.chipSettings.gp6 = static_cast<quint8>(ui->comboBoxGP6->currentIndex() > 0 ? ui->comboBoxGP6->currentIndex() - 1 : MCP2210::PCGPIO);
    editedConfiguration_.chipSettings.gp7 = static_cast<quint8>(ui->comboBoxGP7->currentIndex() > 0 ? ui->comboBoxGP7->currentIndex() - 1 : MCP2210::PCGPIO);
    editedConfiguration_.chipSettings.gp8 = static_cast<quint8>(ui->comboBoxGP8->currentIndex() == 0 ? MCP2210::PCGPIO : MCP2210::PCFUNC);
    editedConfiguration_.chipSettings.gpdir = static_cast<quint8>((ui->comboBoxGP7->currentIndex() != 1) << 7 |  // All pins have their direction set to input by default, except those pins that are specifically set to be outputs
                                                                  (ui->comboBoxGP6->currentIndex() != 1) << 6 |
                                                                  (ui->comboBoxGP5->currentIndex() != 1) << 5 |
                                                                  (ui->comboBoxGP4->currentIndex() != 1) << 4 |
                                                                  (ui->comboBoxGP3->currentIndex() != 1) << 3 |
                                                                  (ui->comboBoxGP2->currentIndex() != 1) << 2 |
                                                                  (ui->comboBoxGP1->currentIndex() != 1) << 1 |
                                                                  (ui->comboBoxGP0->currentIndex() != 1));
    editedConfiguration_.chipSettings.gpout = static_cast<quint8>(ui->checkBoxGP7DefaultValue->isChecked() << 7 |
                                                                  ui->checkBoxGP6DefaultValue->isChecked() << 6 |
                                                                  ui->checkBoxGP5DefaultValue->isChecked() << 5 |
                                                                  ui->checkBoxGP4DefaultValue->isChecked() << 4 |
                                                                  ui->checkBoxGP3DefaultValue->isChecked() << 3 |
                                                                  ui->checkBoxGP2DefaultValue->isChecked() << 2 |
                                                                  ui->checkBoxGP1DefaultValue->isChecked() << 1 |
                                                                  ui->checkBoxGP0DefaultValue->isChecked());
    editedConfiguration_.chipSettings.rmwakeup = ui->checkBoxRemoteWakeUp->isChecked();
    editedConfiguration_.chipSettings.intmode = static_cast<quint8>(ui->comboBoxInterruptMode->currentIndex());
    editedConfiguration_.chipSettings.nrelspi = ui->checkBoxSPIBusCaptive->isChecked();
    editedConfiguration_.spiSettings.nbytes = static_cast<quint16>(ui->spinBoxBytesPerTransaction->value());
    editedConfiguration_.spiSettings.bitrate = static_cast<quint32>(1000 * ui->doubleSpinBoxBitRate->value());
    editedConfiguration_.spiSettings.mode = static_cast<quint8>(ui->spinBoxMode->value());
    editedConfiguration_.spiSettings.actcs = static_cast<quint8>(ui->checkBoxActiveCS7->isChecked() << 7 |
                                                                 ui->checkBoxActiveCS6->isChecked() << 6 |
                                                                 ui->checkBoxActiveCS6->isChecked() << 6 |
                                                                 ui->checkBoxActiveCS5->isChecked() << 5 |
                                                                 ui->checkBoxActiveCS4->isChecked() << 4 |
                                                                 ui->checkBoxActiveCS3->isChecked() << 3 |
                                                                 ui->checkBoxActiveCS2->isChecked() << 2 |
                                                                 ui->checkBoxActiveCS1->isChecked() << 1 |
                                                                 ui->checkBoxActiveCS0->isChecked());
    editedConfiguration_.spiSettings.idlcs = static_cast<quint8>(ui->checkBoxIdleCS7->isChecked() << 7 |
                                                                 ui->checkBoxIdleCS6->isChecked() << 6 |
                                                                 ui->checkBoxIdleCS6->isChecked() << 6 |
                                                                 ui->checkBoxIdleCS5->isChecked() << 5 |
                                                                 ui->checkBoxIdleCS4->isChecked() << 4 |
                                                                 ui->checkBoxIdleCS3->isChecked() << 3 |
                                                                 ui->checkBoxIdleCS2->isChecked() << 2 |
                                                                 ui->checkBoxIdleCS1->isChecked() << 1 |
                                                                 ui->checkBoxIdleCS0->isChecked());
    editedConfiguration_.spiSettings.csdtdly = static_cast<quint16>(ui->spinBoxCSToDataDelay->value());
    editedConfiguration_.spiSettings.dtcsdly = static_cast<quint16>(ui->spinBoxDataToCSDelay->value());
    editedConfiguration_.spiSettings.itbytdly = static_cast<quint16>(ui->spinBoxInterByteDelay->value());
    if (ui->radioButtonPasswordProtected->isChecked()) {
        editedConfiguration_.accessMode = MCP2210::ACPASSWORD;
    } else if (ui->radioButtonPermanentlyLocked->isChecked()) {
        editedConfiguration_.accessMode = MCP2210::ACLOCKED;
    } else {
        editedConfiguration_.accessMode = MCP2210::ACNONE;
    }
}

// Returns the nearest compatible bit rate, given a bit rate
quint32 ConfiguratorWindow::getNearestCompatibleBitRate(quint32 bitrate)
{
    quint32 retval;
    int errcnt = 0;
    QString errstr;
    MCP2210::SPISettings initialSPISettings = mcp2210_.getSPISettings(errcnt, errstr);  // Keep these initial volatile SPI settings
    MCP2210::SPISettings testSPISettings = initialSPISettings;  // Settings used to test bitrate values
    quint32 testBitrate = static_cast<quint32>(1.5 * bitrate);  // Variable used for testing and finding compatible bit rates (multiplier value was determined empirically)
    quint32 nearestLowerBitrate = MCP2210Limits::BITRATE_MIN, nearestUpperBitrate = MCP2210Limits::BITRATE_MAX;  // These variables are assigned here, not just for correctness, but to allow algorithmic simplification
    while (errcnt == 0) {
        testSPISettings.bitrate = testBitrate;
        mcp2210_.configureSPISettings(testSPISettings, errcnt, errstr);
        quint32 returnedBitrate = mcp2210_.getSPISettings(errcnt, errstr).bitrate;
        if (returnedBitrate == testBitrate) {
            if (testBitrate >= bitrate) {
                nearestUpperBitrate = testBitrate;  // Can be equal to the input value, if the latter is found to be a compatible bit rate
            }
            if (testBitrate <= bitrate) {
                nearestLowerBitrate = testBitrate;  // Again, can be equal to the input value, if the latter is found to be a compatible bit rate
                break;
            }
            --testBitrate;
        } else {  // Incidentally, "returnedBitrate" is expected to be less than "testBitrate"
            testBitrate = returnedBitrate;
        }
    }
    mcp2210_.configureSPISettings(initialSPISettings, errcnt, errstr);  // Restore the previously kept volatile SPI settings
    validateOperation(tr("get nearest compatible bit rate"), errcnt, errstr);
    if (nearestUpperBitrate - bitrate > bitrate - nearestLowerBitrate) {  // Half-way cases are approximated to the nearest upper bit rate
        retval = nearestLowerBitrate;
    } else {
        retval = nearestUpperBitrate;
    }
    return retval;
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
    ConfigurationReader configReader(editedConfiguration_);
    if (!configReader.readFrom(&file)) {
        QMessageBox::critical(this, tr("Error"), configReader.errorString());
    } else {
        err_ = false;
        editedConfiguration_.spiSettings.bitrate = getNearestCompatibleBitRate(editedConfiguration_.spiSettings.bitrate);  // Note that getNearestCompatibleBitRate() is guaranteed to return a valid bit rate value
        if (err_) {  // If an error has occured
            handleError();
        }
        displayConfiguration(editedConfiguration_, false);
    }
}

// Prepares the task list, by checking which fields changed, while also setting optional tasks according to the user's requirements
QStringList ConfiguratorWindow::prepareTaskList()
{
    QStringList tasks;
    if (editedConfiguration_.manufacturer != deviceConfiguration_.manufacturer) {
        tasks += "writeManufacturerDesc";
    }
    if (editedConfiguration_.product != deviceConfiguration_.product) {
        tasks += "writeProductDesc";
    }
    if (editedConfiguration_.usbParameters != deviceConfiguration_.usbParameters) {
        tasks += "writeUSBParameters";
    }
    if (editedConfiguration_.spiSettings != deviceConfiguration_.spiSettings) {
        tasks += "writeSPISettings";
    }
    if (editedConfiguration_.chipSettings != deviceConfiguration_.chipSettings) {
        tasks += "writeChipSettings";  // Depending on the user's choice, this may protect or even lock the device
    }
    tasks += "verifyConfiguration";
    if (ui->checkBoxApplyImmediately->isChecked()) {
        if (editedConfiguration_.chipSettings != deviceConfiguration_.chipSettings) {
            tasks += "applyChipSettings";
        }
        if (editedConfiguration_.spiSettings != deviceConfiguration_.spiSettings) {
            tasks += "applySPISettings";
        }
    }
    return tasks;
}

// This is the routine that reads the configuration from the MCP2210 NVRAM
void ConfiguratorWindow::readDeviceConfiguration()
{
    int errcnt = 0;
    QString errstr;
    deviceConfiguration_.manufacturer = mcp2210_.getManufacturerDesc(errcnt, errstr);
    deviceConfiguration_.product = mcp2210_.getProductDesc(errcnt, errstr);
    deviceConfiguration_.usbParameters = mcp2210_.getUSBParameters(errcnt, errstr);
    deviceConfiguration_.chipSettings = mcp2210_.getNVChipSettings(errcnt, errstr);
    deviceConfiguration_.spiSettings = mcp2210_.getNVSPISettings(errcnt, errstr);
    deviceConfiguration_.accessMode = mcp2210_.getAccessControlMode(errcnt, errstr);
    MCP2210::ChipStatus chipStatus = mcp2210_.getChipStatus(errcnt, errstr);
    passwordIsLocked_ = chipStatus.pwtries > 4;
    passwordIsValid_ = chipStatus.pwok;
    validateOperation(tr("read device configuration"), errcnt, errstr);
}

// Reads the contents from the MCP2210 EEPROM
MCP2210EEPROM ConfiguratorWindow::readEEPROM()
{
    MCP2210EEPROM eeprom;
    this->setCursor(Qt::WaitCursor);  // This task takes quite a few tenths of a second, so it is a good idea to change the cursor to reflect that
    int errcnt = 0;
    QString errstr;
    for (size_t i = 0; i < MCP2210::EEPROM_SIZE; ++i) {
        eeprom.bytes[i] = mcp2210_.readEEPROMByte(i, errcnt, errstr);
        validateOperation(tr("read EEPROM"), errcnt, errstr);
        if (err_) {  // If an error has occured
            break;  // Abort
        }
    }
    this->unsetCursor();
    return eeprom;
}

// Saves the current configuration to a given file
void ConfiguratorWindow::saveConfigurationToFile(QFile &file)
{
    getEditedConfiguration();
    ConfigurationWriter configWriter(editedConfiguration_);
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

// Enables or disables all fields pertaining to general settings
void ConfiguratorWindow::setGeneralSettingsEnabled(bool value)
{
    ui->lineEditManufacturer->setReadOnly(!value);
    ui->lineEditProduct->setReadOnly(!value);
    ui->lineEditVID->setReadOnly(!value);
    ui->lineEditPID->setReadOnly(!value);
    ui->lineEditMaxPower->setReadOnly(!value);
    ui->lineEditMaxPowerHex->setReadOnly(!value);
    ui->comboBoxPowerMode->setEnabled(value);
    ui->checkBoxRemoteWakeUpCapable->setEnabled(value);
    ui->groupBoxNVRAMAccessMode->setEnabled(value);
}

// Enables or disables all fields pertaining to SPI settings
void ConfiguratorWindow::setSPISettingsEnabled(bool value)
{
    ui->spinBoxBytesPerTransaction->setEnabled(value);
    ui->doubleSpinBoxBitRate->setEnabled(value);
    ui->spinBoxMode->setEnabled(value);
    ui->spinBoxCPOL->setEnabled(value);
    ui->spinBoxCPHA->setEnabled(value);
    ui->groupBoxActiveCSValues->setEnabled(value);
    ui->groupBoxIdleCSValues->setEnabled(value);
    ui->groupBoxDelays->setEnabled(value);
}

// Enables or disables the "Use Password" menu option (File > Use Password)
void ConfiguratorWindow::setUsePasswordEnabled(bool value)
{
    ui->actionUsePassword->setEnabled(value);
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
    if (ui->lineEditNewPassword->text() != ui->lineEditRepeatPassword->text()) {
        ui->lineEditNewPassword->setStyleSheet("background: rgb(255, 102, 102);");
        ui->lineEditRepeatPassword->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    return retval;
}

// Checks for errors and validates device operations
void ConfiguratorWindow::validateOperation(const QString &operation, int errcnt, QString errstr)  // The variable "errstr" is passed by value here, because it needs to be modified locally!
{
    if (errcnt > 0) {
        err_ = true;
        if (mcp2210_.disconnected()) {
            errmsg_ = tr("Device disconnected.\n\nPlease reconnect it and try again.");
        } else {
            errstr.chop(1);  // Remove the last character, which is always a newline
            errmsg_ = tr("Failed to %1. The operation returned the following error(s):\n– %2", "", errcnt).arg(operation, errstr.replace("\n", "\n– "));
        }
    }
}

// Prompts the user for a password, validating it and returning true if valid
bool ConfiguratorWindow::validatePassword()
{
    bool retval = false;
    PasswordDialog passwordDialog(this);
    if (passwordDialog.exec() == QDialog::Accepted) {
        err_ = false;
        int errcnt = 0;
        QString errstr;
        quint8 response = mcp2210_.usePassword(passwordDialog.passwordLineEditText(), errcnt, errstr);
        MCP2210::ChipStatus chipStatus = mcp2210_.getChipStatus(errcnt, errstr);
        validateOperation(tr("use password"), errcnt, errstr);
        if (err_) {  // If an error has occured
            handleError();
        } else {  // Success
            if (!statusDialog_.isNull()) {  // This updates the status dialog, but only the fields pertaining to the password, if such is open
                statusDialog_->setPasswordStatusValueLabelText(chipStatus.pwok);
                statusDialog_->setPasswordTriesValueLabelText(chipStatus.pwtries);
            }
            if (response == MCP2210::COMPLETED) {  // If error check passes and password is verified
                setUsePasswordEnabled(false);  // Disable "Use Password" action
                // TODO Keep valid password
                passwordIsValid_ = true;
                retval = true;
            } else if (response == MCP2210::BLOCKED || chipStatus.pwtries > 4) {  // If access is blocked (redundancy is necessary)
                passwordIsLocked_ = true;  // From this point on, the device will be viewed as if it was locked
                displayConfiguration(deviceConfiguration_, true);
                QMessageBox::warning(this, tr("Access Blocked"), tr("The password was not accepted and access is temporarily blocked. Please disconnect and reconnect your device, and try again."));
            } else if (response == MCP2210::REJECTED) {  // If access is somehow rejected
                QMessageBox::warning(this, tr("Access Rejected"), tr("Full write access to the NVRAM was rejected for unknown reasons."));
            } else if (response == MCP2210::WRONG_PASSWORD) {  // If password is not verified
                QMessageBox::warning(this, tr("Access Denied"), tr("The password was not accepted. Please try again."));
            }
        }
    }
    return retval;
}

// Overwrites the contents of the MCP2210 EEPROM
void ConfiguratorWindow::writeEEPROM(MCP2210EEPROM eeprom)
{
    this->setCursor(Qt::WaitCursor);  // This task takes several tenths of a second, so it is a good idea to change the cursor to reflect that
    int errcnt = 0;
    QString errstr;
    for (size_t i = 0; i < MCP2210::EEPROM_SIZE; ++i) {
        eeprom.bytes[i] = mcp2210_.writeEEPROMByte(i, eeprom.bytes[i], errcnt, errstr);
        validateOperation(tr("write EEPROM"), errcnt, errstr);
        if (err_) {  // If an error has occured
            break;  // Abort
        }
    }
    this->unsetCursor();
}
