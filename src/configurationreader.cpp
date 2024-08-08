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
            readWordGeneric("vid", configuration_.usbparameters.vid, MCP2210Limits::VID_MIN, MCP2210Limits::VID_MAX);
        } else if (xmlReader_.name() == QLatin1String("pid")) {
            readWordGeneric("pid", configuration_.usbparameters.pid, MCP2210Limits::PID_MIN, MCP2210Limits::PID_MAX);
        } else if (xmlReader_.name() == QLatin1String("power")) {
            readPower();
        }
        // TODO
    }
}

// Reads descriptor element (used for manufacturer and product descriptors)
void ConfigurationReader::readDescriptor(QString name, QString &toVariable)
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
                configuration_.usbparameters.maxpow = static_cast<quint8>(maxpow);
            }
        } else if (attr.name().toString() == "self-powered") {
            QString selfpow = attr.value().toString();
            if (selfpow != "true" && selfpow != "false" && selfpow != "1" && selfpow != "0") {
                xmlReader_.raiseError(QObject::tr("In \"power\" element, the \"self-powered\" attribute contains an invalid value. It should be \"true\", \"false\", \"1\" or \"0\"."));
            } else {
                configuration_.usbparameters.powmode = selfpow == "true" || selfpow == "1";
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Generic procedure to read a named element with a word value in hexadecimal as it's attribute (used for VID and PID)
void ConfigurationReader::readWordGeneric(QString name, quint16 &toVariable, quint16 min, quint16 max)
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == name);

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {
        if (attr.name().toString() == "value") {
            bool ok;
            quint16 word = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Cast done for sanity purposes
            if (!ok || word > max || word < min) {
                xmlReader_.raiseError(QObject::tr("In \"%1\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between %2 and %3.").arg(name).arg(min, 0, 16).arg(max, 0, 16));
            } else {
                toVariable = word;
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
