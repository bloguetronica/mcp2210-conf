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
#include "configurationwriter.h"

// Writes the sub-elements of "mcp2210config" element, which is the root element
void ConfigurationWriter::writeConfiguration()
{
    writeDescriptor("manufacturer", configuration_.manufacturer);
    writeDescriptor("product", configuration_.product);
    writeWordGeneric("vid", configuration_.usbparameters.vid);
    writeWordGeneric("pid", configuration_.usbparameters.pid);
    writePower();
    writeRemoteWakeup();
    // TODO
}

// Writes descriptor element (used for manufacturer and product descriptors)
void ConfigurationWriter::writeDescriptor(QString name, QString value)
{
    xmlWriter_.writeStartElement(name);
    xmlWriter_.writeAttribute("string", value);
    xmlWriter_.writeEndElement();
}

// Writes "power" element
void ConfigurationWriter::writePower()
{
    xmlWriter_.writeStartElement("power");
    xmlWriter_.writeAttribute("maximum", QString::number(configuration_.usbparameters.maxpow, 16));
    xmlWriter_.writeAttribute("self", (configuration_.usbparameters.powmode ? "true" : "false"));
    xmlWriter_.writeEndElement();
}

// Writes "remotewakeup" element
void ConfigurationWriter::writeRemoteWakeup()
{
    xmlWriter_.writeStartElement("remotewakeup");
    xmlWriter_.writeAttribute("capable", (configuration_.usbparameters.rmwakeup ? "true" : "false"));
    xmlWriter_.writeEndElement();
}

// Generic procedure to write a named element with a word value in hexadecimal as its attribute (used for VID and PID)
void ConfigurationWriter::writeWordGeneric(QString name, quint16 value)
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
