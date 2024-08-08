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


#ifndef CONFIGURATORWINDOW_H
#define CONFIGURATORWINDOW_H

// Includes
#include <QFile>
#include <QMainWindow>
#include <QPointer>
#include <QString>
#include <QStringList>
#include "configuration.h"
#include "mcp2210.h"
#include "statusdialog.h"

namespace Ui {
class ConfiguratorWindow;
}

class ConfiguratorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConfiguratorWindow(QWidget *parent = nullptr);
    ~ConfiguratorWindow();

    bool isViewEnabled();
    void openDevice(quint16 vid, quint16 pid, const QString &serialstr);

private slots:
    void on_actionAbout_triggered();
    void on_actionLoadConfiguration_triggered();
    void on_actionReadEEPROM_triggered();
    void on_actionSaveConfiguration_triggered();
    void on_actionStatus_triggered();
    void on_actionUsePassword_triggered();
    void on_actionVerifyEEPROM_triggered();
    void on_actionWriteEEPROM_triggered();
    void on_checkBoxDoNotChangePassword_stateChanged(int state);
    void on_lineEditManufacturer_textEdited(QString text);
    void on_lineEditMaxPower_editingFinished();
    void on_lineEditMaxPower_textChanged(const QString &text);
    void on_lineEditMaxPower_textEdited(QString text);
    void on_lineEditMaxPowerHex_editingFinished();
    void on_lineEditMaxPowerHex_textChanged(const QString &text);
    void on_lineEditMaxPowerHex_textEdited(const QString &text);
    void on_lineEditNewPassword_textChanged(const QString &text);
    void on_lineEditPID_textChanged(const QString &text);
    void on_lineEditPID_textEdited(const QString &text);
    void on_lineEditProduct_textEdited();
    void on_lineEditRepeatPassword_textChanged(const QString &text);
    void on_lineEditVID_textChanged(const QString &text);
    void on_lineEditVID_textEdited(const QString &text);
    void on_pushButtonRevealNewPassword_pressed();
    void on_pushButtonRevealNewPassword_released();
    void on_pushButtonRevealRepeatPassword_pressed();
    void on_pushButtonRevealRepeatPassword_released();
    void on_pushButtonRevert_clicked();
    void on_pushButtonWrite_clicked();
    void on_radioButtonPasswordProtected_toggled(bool checked);
    void verifyConfiguration();
    void writeManufacturerDesc();
    void writeProductDesc();
    void writeUSBParameters();

private:
    Ui::ConfiguratorWindow *ui;
    Configuration deviceConfig_, editedConfig_;
    MCP2210 mcp2210_;
    QPointer<StatusDialog> statusDialog_;
    QString errmsg_, serialstr_;
    quint8 accessMode_;
    bool err_, viewEnabled_ = false;

    void configureDevice();
    void disableView();
    void displayChipSettings(const MCP2210::ChipSettings &chipsettings);
    void displayConfiguration(const Configuration &config);
    void displayManufacturer(const QString &manufacturer);
    void displayNVRAMAccessMode();
    void displayProduct(const QString &product);
    void displaySPISettings(const MCP2210::SPISettings &spisettings);
    void displayUSBParameters(const MCP2210::USBParameters &usbparameters);
    void getEditedConfiguration();
    void handleError();
    void loadConfigurationFromFile(QFile &file);
    void opCheck(const QString &op, int errcnt, QString errstr);
    QStringList prepareTaskList();
    void readDeviceConfiguration();
    void saveConfigurationToFile(QFile &file);
    void setChipSettingsEnabled(bool value);
    void setManufacturerEnabled(bool value);
    void setMaxPowerEnabled(bool value);
    void setNVRAMAccessModeEnabled(bool value);
    void setPIDEnabled(bool value);
    void setPowerModeEnabled(bool value);
    void setProductEnabled(bool value);
    void setRemoteWakeUpCapableEnabled(bool value);
    void setSPISettingsEnabled(bool value);
    void setUsePasswordEnabled(bool value);
    void setVIDEnabled(bool value);
    void setWriteEnabled(bool value);
    bool showInvalidInput();
};

#endif  // CONFIGURATORWINDOW_H
