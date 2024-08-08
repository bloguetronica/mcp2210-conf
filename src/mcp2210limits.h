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


#ifndef MCP2210LIMITS_H
#define MCP2210LIMITS_H

// Includes
#include <QtGlobal>
#include "mcp2210.h"

namespace MCP2210Limits
{
const quint8 MAXPOW_MAX = 0xfa;
const quint16 PID_MAX = 0xffff;
const quint16 PID_MIN = 0x0001;
const quint16 VID_MAX = 0xffff;
const quint16 VID_MIN = 0x0001;
}

#endif  // MCP2210LIMITS_H
