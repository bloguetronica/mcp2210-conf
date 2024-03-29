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
#include <QMainWindow>
#include <QPointer>
#include <QString>
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
    void on_actionReadEEPROM_triggered();
    void on_actionStatus_triggered();
    void on_actionVerifyEEPROM_triggered();
    void on_actionWriteEEPROM_triggered();
    void on_lineEditManufacturer_textEdited();
    void on_lineEditMaxPower_editingFinished();
    void on_lineEditMaxPower_textChanged();
    void on_lineEditMaxPower_textEdited();
    void on_lineEditMaxPowerHex_editingFinished();
    void on_lineEditMaxPowerHex_textChanged();
    void on_lineEditMaxPowerHex_textEdited();
    void on_lineEditPID_textChanged();
    void on_lineEditPID_textEdited();
    void on_lineEditProduct_textEdited();
    void on_lineEditVID_textChanged();
    void on_lineEditVID_textEdited();
    void on_pushButtonRevert_clicked();
    void on_pushButtonWrite_clicked();

private:
    Ui::ConfiguratorWindow *ui;
    Configuration deviceConfig_, editedConfig_;
    MCP2210 mcp2210_;
    QPointer<StatusDialog> statusDialog_;
    QString errmsg_, serialstr_;
    quint16 pid_, vid_;
    bool deviceLocked_ = true, err_, viewEnabled_ = false;

    void disableView();
    void displayChipSettings(const MCP2210::ChipSettings &chipsettings);
    void displayConfiguration(const Configuration &config);
    void displayManufacturer(const QString &manufacturer);
    void displayProduct(const QString &product);
    void displaySPISettings(const MCP2210::SPISettings &spisettings);
    void displayUSBParameters(const MCP2210::USBParameters &usbparameters);
    void getEditedConfiguration();
    void handleError();
    void opCheck(const QString &op, int errcnt, QString errstr);
    void readDeviceConfiguration();
    void setChipSettingsEnabled(bool value);
    void setManufacturerEnabled(bool value);
    void setMaxPowerEnabled(bool value);
    void setPIDEnabled(bool value);
    void setPowerModeEnabled(bool value);
    void setProductEnabled(bool value);
    void setRemoteWakeUpCapableEnabled(bool value);
    void setSPISettingsEnabled(bool value);
    void setVIDEnabled(bool value);
    void setWriteEnabled(bool value);
};

#endif  // CONFIGURATORWINDOW_H
