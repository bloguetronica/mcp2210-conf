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


#ifndef CONFIGURATIONWRITER_H
#define CONFIGURATIONWRITER_H

// Includes
#include <QIODevice>
#include <QString>
#include <QXmlStreamWriter>
#include "configuration.h"

class ConfigurationWriter
{
private:
    const Configuration &configuration_;
    QXmlStreamWriter xmlWriter_;

    void writeBitRate();
    void writeByteGeneric(const QString &name, quint8 value);
    void writeConfiguration();
    void writeDelay(const QString &name, quint16 delay);
    void writeDelays();
    void writeDescriptor(const QString &name, QString value);
    void writeGP(int number, quint8 mode);
    void writeInterrupt();
    void writeMode();
    void writeNBytes();
    void writePins();
    void writePower();
    void writeRemoteWakeup();
    void writeSPIBus();
    void writeSPISettings();
    void writeWordGeneric(const QString &name, quint16 value);

public:
    explicit ConfigurationWriter(const Configuration &configuration);

    void writeTo(QIODevice *device);
};

#endif  // CONFIGURATIONWRITER_H
