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


#ifndef CONFIGURATORWINDOW_H
#define CONFIGURATORWINDOW_H

// Includes
#include <QMainWindow>
#include <QString>
#include "configuration.h"
#include "mcp2210.h"

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
    void on_lineEditManufacturer_textEdited();
    void on_lineEditPID_textChanged();
    void on_lineEditPID_textEdited();
    void on_lineEditProduct_textEdited();
    void on_lineEditVID_textChanged();
    void on_lineEditVID_textEdited();

private:
    Ui::ConfiguratorWindow *ui;
    Configuration deviceConfig_, editedConfig_;
    MCP2210 mcp2210_;
    QString serialstr_;
    quint16 pid_, vid_;
    bool deviceLocked_ = true, viewEnabled_ = false;

    void displayConfiguration(const Configuration &config);
    void displayManufacturer(const QString &manufacturer);
    void displayProduct(const QString &product);
    void displayUSBParameters(const MCP2210::USBParameters &usbparameters);
    void readDeviceConfiguration();
    void setManufacturerEnabled(bool value);
    void setPIDEnabled(bool value);
    void setProductEnabled(bool value);
    void setVIDEnabled(bool value);
};

#endif  // CONFIGURATORWINDOW_H
