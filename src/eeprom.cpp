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
#include "eeprom.h"

// "Equal to" operator
bool EEPROM::operator ==(const EEPROM &other) const
{
    bool equal = true;
    for (size_t i = 0; i < EEPROM_SIZE; ++i) {
        if (bytes[i] != other.bytes[i]) {
            equal = false;
            break;
        }
    }
    return equal;
}

// "In" operator
QDataStream &EEPROM::operator <<(QDataStream &dataStream)
{
    for (size_t i = 0; i < EEPROM_SIZE; ++i) {
        dataStream << bytes[i];
    }
    return dataStream;
}

// "Out" operator
QDataStream &EEPROM::operator >>(QDataStream &dataStream)
{
    for (size_t i = 0; i < EEPROM_SIZE; ++i) {
        dataStream >> bytes[i];
    }
    return dataStream;
}
