/* MCP2210 Configurator - Version 1.0.5 for Debian Linux
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


#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Includes
#include <QString>
#include "mcp2210.h"

struct Configuration
{
    QString manufacturer;
    QString product;
    MCP2210::USBParameters usbParameters;
    MCP2210::ChipSettings chipSettings;
    MCP2210::SPISettings spiSettings;
    quint8 accessMode;

    bool operator ==(const Configuration &other) const;
    bool operator !=(const Configuration &other) const;
};

#endif  // CONFIGURATION_H
