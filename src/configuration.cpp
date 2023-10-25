/* MCP2210 Configurator - Version 1.0.0 for Debian Linux
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
#include "configuration.h"

// "Equal to" operator for Configuration
bool Configuration::operator ==(const Configuration &other) const
{
    return manufacturer == other.manufacturer && product == other.product && usbparameters == other.usbparameters && chipsettings == other.chipsettings && spisettings == other.spisettings;
}

// "Not equal to" operator for SiliconVersion
bool Configuration::operator !=(const Configuration &other) const
{
    return !(operator ==(other));
}
