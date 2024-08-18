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
#include "mcp2210.h"
#include "configurationwriter.h"

// Generic procedure to write a named element with a byte value in hexadecimal as its attribute (used for pin configurations)
void ConfigurationWriter::writeByteGeneric(const QString &name, quint8 value)
{
    writeWordGeneric(name, value);
}

// Writes the sub-elements of "mcp2210config" element, which is the root element
void ConfigurationWriter::writeConfiguration()
{
    writeDescriptor("manufacturer", configuration_.manufacturer);
    writeDescriptor("product", configuration_.product);
    writeWordGeneric("vid", configuration_.usbParameters.vid);
    writeWordGeneric("pid", configuration_.usbParameters.pid);
    writePower();
    writeRemoteWakeup();
    writePins();
    writeInterrupt();
    writeSPIBus();
    // TODO
}

// Writes descriptor element (used for manufacturer and product descriptors)
void ConfigurationWriter::writeDescriptor(const QString &name, QString value)
{
    xmlWriter_.writeStartElement(name);
    xmlWriter_.writeAttribute("string", value);
    xmlWriter_.writeEndElement();
}

// Writes GP element
void ConfigurationWriter::writeGP(int number, quint8 mode)
{
    xmlWriter_.writeStartElement(QString("gp%1").arg(number));
    xmlWriter_.writeAttribute("mode", QString::number(mode));
    xmlWriter_.writeEndElement();
}

// Writes "interrupt" element
void ConfigurationWriter::writeInterrupt()
{
    xmlWriter_.writeStartElement("interrupt");
    xmlWriter_.writeAttribute("mode", QString::number(configuration_.chipSettings.intmode));
    xmlWriter_.writeEndElement();
}

// Writes "pins" element
void ConfigurationWriter::writePins()
{
    xmlWriter_.writeStartElement("pins");
    QVector<quint8> pinModes{
        configuration_.chipSettings.gp0,
        configuration_.chipSettings.gp1,
        configuration_.chipSettings.gp2,
        configuration_.chipSettings.gp3,
        configuration_.chipSettings.gp4,
        configuration_.chipSettings.gp5,
        configuration_.chipSettings.gp6,
        configuration_.chipSettings.gp7,
        configuration_.chipSettings.gp8
    };
    int numberOfPins = pinModes.size();
    for (int i = 0; i < numberOfPins; ++i) {
        writeGP(i, pinModes.at(i));
    }
    writeByteGeneric("gpdir", configuration_.chipSettings.gpdir);
    writeByteGeneric("gpout", configuration_.chipSettings.gpout);
    xmlWriter_.writeEndElement();
}

// Writes "power" element
void ConfigurationWriter::writePower()
{
    xmlWriter_.writeStartElement("power");
    xmlWriter_.writeAttribute("maximum", QString::number(configuration_.usbParameters.maxpow, 16));
    xmlWriter_.writeAttribute("self", (configuration_.usbParameters.powmode ? "true" : "false"));
    xmlWriter_.writeEndElement();
}

// Writes "remotewakeup" element
void ConfigurationWriter::writeRemoteWakeup()
{
    xmlWriter_.writeStartElement("remotewakeup");
    xmlWriter_.writeAttribute("capable", (configuration_.usbParameters.rmwakeup ? "true" : "false"));
    xmlWriter_.writeAttribute("enabled", (configuration_.chipSettings.rmwakeup ? "true" : "false"));
    xmlWriter_.writeEndElement();
}

// Writes "spibus" element
void ConfigurationWriter::writeSPIBus()
{
    xmlWriter_.writeStartElement("spibus");
    xmlWriter_.writeAttribute("captive", (configuration_.chipSettings.nrelspi ? "true" : "false"));
    xmlWriter_.writeEndElement();
}

// Generic procedure to write a named element with a word value in hexadecimal as its attribute (used for VID and PID)
void ConfigurationWriter::writeWordGeneric(const QString &name, quint16 value)
{
    xmlWriter_.writeStartElement(name);
    xmlWriter_.writeAttribute("value", QString::number(value, 16));
    xmlWriter_.writeEndElement();
}

ConfigurationWriter::ConfigurationWriter(const Configuration &configuration) :
    configuration_(configuration)
{
}

// Writes the current configuration to a given file
void ConfigurationWriter::writeTo(QIODevice *device)
{
    xmlWriter_.setDevice(device);
    xmlWriter_.setAutoFormatting(true);
    xmlWriter_.writeStartDocument();
    xmlWriter_.writeStartElement("mcp2210config");  // Root element
    xmlWriter_.writeAttribute("version", "1.0");
    writeConfiguration();
    xmlWriter_.writeEndElement();
}
