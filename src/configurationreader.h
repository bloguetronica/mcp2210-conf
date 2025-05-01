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


#ifndef CONFIGURATIONREADER_H
#define CONFIGURATIONREADER_H

// Includes
#include <QIODevice>
#include <QString>
#include <QXmlStreamReader>
#include "configuration.h"

class ConfigurationReader
{
private:
    Configuration &configuration_;
    QXmlStreamReader xmlReader_;

    void readBitRate();
    void readByteGeneric(const QString &name, quint8 &toVariable, quint8 min, quint8 max);
    void readConfiguration();
    void readDelay(const QString &name, quint16 &toVariable, quint16 max);
    void readDelays();
    void readDescriptor(const QString &name, QString &toVariable);
    void readGP(int number, quint8 &toVariable, quint8 max);
    void readInterrupt();
    void readMode();
    void readNBytes();
    void readPins();
    void readPower();
    void readRemoteWakeup();
    void readSPIBus();
    void readSPISettings();
    void readWordGeneric(const QString &name, quint16 &toVariable, quint16 min, quint16 max);

public:
    explicit ConfigurationReader(Configuration &configuration);

    QString errorString() const;

    bool readFrom(QIODevice *device);
};

#endif  // CONFIGURATIONREADER_H
