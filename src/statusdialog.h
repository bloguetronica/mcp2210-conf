/* MCP2210 Configurator - Version 1.0.4 for Debian Linux
   Copyright (c) 2023-2025 Samuel Lourenço

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


#ifndef STATUSDIALOG_H
#define STATUSDIALOG_H

// Includes
#include <QDialog>

namespace Ui {
class StatusDialog;
}

class StatusDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatusDialog(QWidget *parent = nullptr);
    ~StatusDialog();

    void setBusOwnerValueLabelText(quint8 busOwner);
    void setBusRequestValueLabelText(bool busRequest);
    void setPasswordStatusValueLabelText(bool passwordOk);
    void setPasswordTriesValueLabelText(quint8 passwordTries);

private:
    Ui::StatusDialog *ui;
};

#endif  // STATUSDIALOG_H
