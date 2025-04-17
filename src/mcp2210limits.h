/* MCP2210 Configurator - Version 1.0.3 for Debian Linux
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


#ifndef MCP2210LIMITS_H
#define MCP2210LIMITS_H

// Includes
#include <QtGlobal>
#include "mcp2210.h"

namespace MCP2210Limits
{
const quint8 ACTCS_MAX = 0xff;
const quint32 BITRATE_MAX = MCP2210::BRT12M;
const quint32 BITRATE_MIN = MCP2210::BRT1K464;
const quint16 CSDTDLY_MAX = 65535;
const quint16 DTCSDLY_MAX = 65535;
const quint8 GP0_MAX = MCP2210::PCCS;
const quint8 GP1_MAX = MCP2210::PCCS;
const quint8 GP2_MAX = MCP2210::PCFUNC;
const quint8 GP3_MAX = MCP2210::PCFUNC;
const quint8 GP4_MAX = MCP2210::PCFUNC;
const quint8 GP5_MAX = MCP2210::PCFUNC;
const quint8 GP6_MAX = MCP2210::PCFUNC;
const quint8 GP7_MAX = MCP2210::PCFUNC;
const quint8 GP8_MAX = MCP2210::PCFUNC;
const quint8 GPDIR_MAX = 0xff;
const quint8 GPOUT_MAX = 0xff;
const quint8 IDLCS_MAX = 0xff;
const quint8 INTMODE_MAX = MCP2210::IMCNTHP;
const quint16 ITBYTDLY_MAX = 65535;
const quint8 MAXPOW_MAX = 0xfa;
const quint16 NBYTES_MAX = 65535;
const quint16 PID_MAX = 0xffff;
const quint16 PID_MIN = 0x0001;
const quint8 SPIMODE_MAX = MCP2210::SPIMODE3;
const quint16 VID_MAX = 0xffff;
const quint16 VID_MIN = 0x0001;
}

#endif  // MCP2210LIMITS_H
