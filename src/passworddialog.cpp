/* MCP2210 Configurator - Version 1.0.1 for Debian Linux
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
#include <QRegExp>
#include <QRegExpValidator>
#include "passworddialog.h"
#include "ui_passworddialog.h"

PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);
    ui->lineEditPassword->setValidator(new QRegExpValidator(QRegExp("[!-~]+"), this));  // All printable ASCII characters except space
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}

// Returns the text of "lineEditPassword"
QString PasswordDialog::passwordLineEditText()
{
    return ui->lineEditPassword->text();
}

void PasswordDialog::on_lineEditPassword_textChanged(const QString &text)
{
    ui->pushButtonRevealPassword->setEnabled(!text.isEmpty());
}

void PasswordDialog::on_pushButtonRevealPassword_pressed()
{
    ui->lineEditPassword->setEchoMode(QLineEdit::Normal);
}

void PasswordDialog::on_pushButtonRevealPassword_released()
{
    ui->lineEditPassword->setEchoMode(QLineEdit::Password);
}
