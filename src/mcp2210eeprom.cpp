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


// Includes
#include "mcp2210eeprom.h"

// "Equal to" operator
bool MCP2210EEPROM::operator ==(const MCP2210EEPROM &other) const
{
    bool equal = true;
    for (size_t i = 0; i < MCP2210::EEPROM_SIZE; ++i) {
        if (bytes[i] != other.bytes[i]) {
            equal = false;
            break;
        }
    }
    return equal;
}

// "Out" operator
QDataStream &operator <<(QDataStream &dataStream, const MCP2210EEPROM &eeprom)
{
    for (size_t i = 0; i < MCP2210::EEPROM_SIZE; ++i) {
        dataStream << eeprom.bytes[i];
    }
    return dataStream;
}

// "In" operator
QDataStream &operator >>(QDataStream &dataStream, MCP2210EEPROM &eeprom)
{
    for (size_t i = 0; i < MCP2210::EEPROM_SIZE; ++i) {
        dataStream >> eeprom.bytes[i];
    }
    return dataStream;
}
