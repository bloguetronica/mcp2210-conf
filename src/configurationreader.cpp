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
