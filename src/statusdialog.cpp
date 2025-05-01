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


// Includes
#include "mcp2210.h"
#include "statusdialog.h"
#include "ui_statusdialog.h"

StatusDialog::StatusDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatusDialog)
{
    ui->setupUi(this);
}

StatusDialog::~StatusDialog()
{
    delete ui;
}

// Sets the text of "labelBusOnwerValue"
void StatusDialog::setBusOwnerValueLabelText(quint8 busOwner)
{
    QString owner;
    switch (busOwner) {
    case MCP2210::BONO:
        owner = tr("No owner");
        break;
    case MCP2210::BOOWN:
        owner = tr("This master");
        break;
    case MCP2210::BOEXT:
        owner = tr("External master");
        break;
    default:
        owner = tr("Unknown");
    }
    ui->labelBusOwnerValue->setText(owner);
}

// Sets the text of "labelBusRequestValue"
void StatusDialog::setBusRequestValueLabelText(bool busRequest)
{
    ui->labelBusRequestValue->setText(busRequest ? tr("Pending") : tr("None"));
}

// Sets the text of "labelPasswordStatusValue"
void StatusDialog::setPasswordStatusValueLabelText(bool passwordOk)
{
    ui->labelPasswordStatusValue->setText(passwordOk ? tr("Guessed") : tr("Not guessed"));
}

// Sets the text of "labelPasswordTriesValue"
void StatusDialog::setPasswordTriesValueLabelText(quint8 passwordTries)
{
    ui->labelPasswordTriesValue->setText(tr("%1 of 5").arg(passwordTries));
}
