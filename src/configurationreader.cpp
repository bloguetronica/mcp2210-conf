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
#include <QObject>
#include "mcp2210.h"
#include "mcp2210limits.h"
#include "configurationreader.h"

// Generic procedure to read a named element with a byte value in hexadecimal as it's attribute (used for pin configurations)
void ConfigurationReader::readByteGeneric(const QString &name, quint8 &toVariable, quint8 min, quint8 max)
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == name);

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {
        if (attr.name().toString() == "value") {
            bool ok;
            ushort value = attr.value().toUShort(&ok, 16);
            if (!ok || value > max || value < min) {
                xmlReader_.raiseError(QObject::tr("In \"%1\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between %2 and %3.").arg(name).arg(min, 0, 16).arg(max, 0, 16));
            } else {
                toVariable = static_cast<quint8>(value);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads the sub-elements of "mcp2210config" element, which is the root element
void ConfigurationReader::readConfiguration()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("mcp2210config"));

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("manufacturer")) {
            readDescriptor("manufacturer", configuration_.manufacturer);
        } else if (xmlReader_.name() == QLatin1String("product")) {
            readDescriptor("product", configuration_.product);
        } else if (xmlReader_.name() == QLatin1String("vid")) {
            readWordGeneric("vid", configuration_.usbParameters.vid, MCP2210Limits::VID_MIN, MCP2210Limits::VID_MAX);
        } else if (xmlReader_.name() == QLatin1String("pid")) {
            readWordGeneric("pid", configuration_.usbParameters.pid, MCP2210Limits::PID_MIN, MCP2210Limits::PID_MAX);
        } else if (xmlReader_.name() == QLatin1String("power")) {
            readPower();
        } else if (xmlReader_.name() == QLatin1String("remotewakeup")) {
            readRemoteWakeup();
        } else if (xmlReader_.name() == QLatin1String("pins")) {
            readPins();
        } else if (xmlReader_.name() == QLatin1String("interrupt")) {
            readInterrupt();
        } else if (xmlReader_.name() == QLatin1String("spibus")) {
            readSPIBus();
        }
        // TODO
    }
}

// Reads descriptor element (used for manufacturer and product descriptors)
void ConfigurationReader::readDescriptor(const QString &name, QString &toVariable)
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == name);

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {
        if (attr.name().toString() == "string") {
            QString descriptor = attr.value().toString();
            if (static_cast<size_t>(descriptor.size()) > MCP2210::DESC_MAXLEN) {
                xmlReader_.raiseError(QObject::tr("In \"%1\" element, the \"string\" attribute contains an invalid value. It should contain a valid manufacturer string, having no more than %2 characters.").arg(name).arg(MCP2210::DESC_MAXLEN));
            } else {
                toVariable = descriptor;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads GP element
void ConfigurationReader::readGP(int number, quint8 &toVariable, quint8 max)
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QString("gp%1").arg(number));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio = attr.value().toUShort(&ok);
            if (!ok || gpio > max) {
                xmlReader_.raiseError(QObject::tr("In \"gp%1\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and %2.").arg(number).arg(max));
            } else {
                toVariable = static_cast<quint8>(gpio);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "interrupt" element
void ConfigurationReader::readInterrupt()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("interrupt"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort intmode = attr.value().toUShort(&ok);
            if (!ok || intmode > MCP2210Limits::INTMODE_MAX) {
                xmlReader_.raiseError(QObject::tr("In \"interrupt\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and %1.").arg(MCP2210Limits::INTMODE_MAX));
            } else {
                configuration_.chipSettings.intmode = static_cast<quint8>(intmode);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads the sub-elements of "pins" element
void ConfigurationReader::readPins()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("pins"));

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("gp0")) {
            readGP(0, configuration_.chipSettings.gp0, MCP2210Limits::GP0_MAX);
        } else if (xmlReader_.name() == QLatin1String("gp1")) {
            readGP(1, configuration_.chipSettings.gp1, MCP2210Limits::GP1_MAX);
        } else if (xmlReader_.name() == QLatin1String("gp2")) {
            readGP(2, configuration_.chipSettings.gp2, MCP2210Limits::GP2_MAX);
        } else if (xmlReader_.name() == QLatin1String("gp3")) {
            readGP(3, configuration_.chipSettings.gp3, MCP2210Limits::GP3_MAX);
        } else if (xmlReader_.name() == QLatin1String("gp4")) {
            readGP(4, configuration_.chipSettings.gp4, MCP2210Limits::GP4_MAX);
        } else if (xmlReader_.name() == QLatin1String("gp5")) {
            readGP(5, configuration_.chipSettings.gp5, MCP2210Limits::GP5_MAX);
        } else if (xmlReader_.name() == QLatin1String("gp6")) {
            readGP(6, configuration_.chipSettings.gp6, MCP2210Limits::GP6_MAX);
        } else if (xmlReader_.name() == QLatin1String("gp7")) {
            readGP(7, configuration_.chipSettings.gp7, MCP2210Limits::GP7_MAX);
        } else if (xmlReader_.name() == QLatin1String("gp8")) {
            readGP(8, configuration_.chipSettings.gp8, MCP2210Limits::GP8_MAX);
        } else if (xmlReader_.name() == QLatin1String("gpdir")) {
            readByteGeneric("gpdir", configuration_.chipSettings.gpdir, 0x00, MCP2210Limits::GPDIR_MAX);
        } else if (xmlReader_.name() == QLatin1String("gpout")) {
            readByteGeneric("gpout", configuration_.chipSettings.gpout, 0x00, MCP2210Limits::GPOUT_MAX);
        } else {
            xmlReader_.skipCurrentElement();
        }
    }
}

// Reads "power" element
void ConfigurationReader::readPower()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("power"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {
        if (attr.name().toString() == "maximum") {
            bool ok;
            ushort maxpow = attr.value().toUShort(&ok, 16);
            if (!ok || maxpow > MCP2210Limits::MAXPOW_MAX) {
                xmlReader_.raiseError(QObject::tr("In \"power\" element, the \"maximum\" attribute contains an invalid value. It should be an hexadecimal integer between 0 and %1.").arg(MCP2210Limits::MAXPOW_MAX, 0, 16));
            } else {
                configuration_.usbParameters.maxpow = static_cast<quint8>(maxpow);
            }
        } else if (attr.name().toString() == "self") {
            QString selfpow = attr.value().toString();
            if (selfpow != "true" && selfpow != "false" && selfpow != "1" && selfpow != "0") {
                xmlReader_.raiseError(QObject::tr("In \"power\" element, the \"self\" attribute contains an invalid value. It should be \"true\", \"false\", \"1\" or \"0\"."));
            } else {
                configuration_.usbParameters.powmode = selfpow == "true" || selfpow == "1";
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "remotewakeup" element
void ConfigurationReader::readRemoteWakeup()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("remotewakeup"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {
        if (attr.name().toString() == "capable") {
            QString rmcapable = attr.value().toString();
            if (rmcapable != "true" && rmcapable != "false" && rmcapable != "1" && rmcapable != "0") {
                xmlReader_.raiseError(QObject::tr("In \"remotewakeup\" element, the \"capable\" attribute contains an invalid value. It should be \"true\", \"false\", \"1\" or \"0\"."));
            } else {
                configuration_.usbParameters.rmwakeup = rmcapable == "true" || rmcapable == "1";
            }
        } else if (attr.name().toString() == "enabled") {
            QString rmenabled = attr.value().toString();
            if (rmenabled != "true" && rmenabled != "false" && rmenabled != "1" && rmenabled != "0") {
                xmlReader_.raiseError(QObject::tr("In \"remotewakeup\" element, the \"enabled\" attribute contains an invalid value. It should be \"true\", \"false\", \"1\" or \"0\"."));
            } else {
                configuration_.chipSettings.rmwakeup = rmenabled == "true" || rmenabled == "1";
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "spibus" element
void ConfigurationReader::readSPIBus()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("spibus"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {
        if (attr.name().toString() == "captive") {
            QString spicaptive = attr.value().toString();
            if (spicaptive != "true" && spicaptive != "false" && spicaptive != "1" && spicaptive != "0") {
                xmlReader_.raiseError(QObject::tr("In \"spibus\" element, the \"captive\" attribute contains an invalid value. It should be \"true\", \"false\", \"1\" or \"0\"."));
            } else {
                configuration_.chipSettings.nrelspi = spicaptive == "true" || spicaptive == "1";
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Generic procedure to read a named element with a word value in hexadecimal as it's attribute (used for VID and PID)
void ConfigurationReader::readWordGeneric(const QString &name, quint16 &toVariable, quint16 min, quint16 max)
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == name);

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {
        if (attr.name().toString() == "value") {
            bool ok;
            quint16 value = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Cast done for sanity purposes
            if (!ok || value > max || value < min) {
                xmlReader_.raiseError(QObject::tr("In \"%1\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between %2 and %3.").arg(name).arg(min, 0, 16).arg(max, 0, 16));
            } else {
                toVariable = value;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

ConfigurationReader::ConfigurationReader(Configuration &configuration) :
    configuration_(configuration)
{
}

// Returns an error string
QString ConfigurationReader::errorString() const
{
    return QObject::tr("Line %1, column %2: %3").arg(xmlReader_.lineNumber()).arg(xmlReader_.columnNumber()).arg(xmlReader_.errorString());
}

// Reads the configuration from a given file, returning false in case of error or true if it succeeds
bool ConfigurationReader::readFrom(QIODevice *device)
{
    xmlReader_.setDevice(device);
    if (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("mcp2210config")) {
            readConfiguration();
        } else {
            xmlReader_.raiseError(QObject::tr("Unknown root element.\n\nThe selected file is not a valid MCP2210 configuration file."));
        }
    }
    return xmlReader_.error() == QXmlStreamReader::NoError;
}
