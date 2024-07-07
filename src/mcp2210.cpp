/* MCP2210 class for Qt - Version 1.2.2
   Copyright (c) 2022-2024 Samuel Lourenço

   This library is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library.  If not, see <https://www.gnu.org/licenses/>.


   Please feel free to contact me via e-mail: samuel.fmlourenco@gmail.com */


// Includes
#include <QByteArray>
#include <QObject>
#include "mcp2210.h"
extern "C" {
#include "libusb-extra.h"
}

// Definitions
const quint8 EPIN = 0x81;             // Address of endpoint assuming the IN direction
const quint8 EPOUT = 0x01;            // Address of endpoint assuming the OUT direction
const unsigned int TR_TIMEOUT = 500;  // Transfer timeout in milliseconds

// Private generic function that is used to get any descriptor
QString MCP2210::getDescGeneric(quint8 subcomid, int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_NVRAM_SETTINGS, subcomid  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    size_t maxSize = 2 * DESC_MAXLEN;  // Maximum size of the descriptor in bytes (the zero padding at the end takes two more bytes)
    size_t size = response.at(4) - 2;  // Descriptor actual size, excluding the padding
    size = size > maxSize ? maxSize : size;  // This also fixes an erroneous result due to a possible unsigned integer rollover (bug fixed in version 1.2.0)
    QString descriptor;
    for (size_t i = 0; i < size; i += 2) {
        descriptor += QChar(response.at(i + PREAMBLE_SIZE + 3) << 8 | response.at(i + PREAMBLE_SIZE + 2));  // UTF-16LE conversion as per the USB 2.0 specification
    }
    return descriptor;
}

// Private function that is used to perform interrupt transfers
void MCP2210::interruptTransfer(quint8 endpointAddr, unsigned char *data, int length, int *transferred, int &errcnt, QString &errstr)
{
    if (!isOpen()) {
        ++errcnt;
        errstr += QObject::tr("In interruptTransfer(): device is not open.\n");  // Program logic error
    } else {
        int result = libusb_interrupt_transfer(handle_, endpointAddr, data, length, transferred, TR_TIMEOUT);
        if (result != 0 || (transferred != nullptr && *transferred != length)) {  // The number of transferred bytes is also verified, as long as a valid (non-null) pointer is passed via "transferred"
            ++errcnt;
            if (endpointAddr < 0x80) {
                errstr += QObject::tr("Failed interrupt OUT transfer to endpoint %1 (address 0x%2).\n").arg(0x0f & endpointAddr).arg(endpointAddr, 2, 16, QChar('0'));
            } else {
                errstr += QObject::tr("Failed interrupt IN transfer from endpoint %1 (address 0x%2).\n").arg(0x0f & endpointAddr).arg(endpointAddr, 2, 16, QChar('0'));
            }
            if (result == LIBUSB_ERROR_NO_DEVICE || result == LIBUSB_ERROR_IO) {  // Note that libusb_interrupt_transfer() may return "LIBUSB_ERROR_IO" [-1] on device disconnect
                disconnected_ = true;  // This reports that the device has been disconnected
            }
        }
    }
}

// Private generic function that is used to write any descriptor
quint8 MCP2210::writeDescGeneric(const QString &descriptor, quint8 subcomid, int &errcnt, QString &errstr)
{
    int strLength = descriptor.size();  // Descriptor string length
    QVector<quint8> command(2 * strLength + PREAMBLE_SIZE + 2);
    command[0] = SET_NVRAM_SETTINGS;                      // Header
    command[1] = subcomid;
    command[2] = 0x00;
    command[3] = 0x00;
    command[4] = static_cast<quint8>(2 * strLength + 2);  // Descriptor length in bytes
    command[5] = 0x03;                                    // USB descriptor constant
    for (int i = 0; i < strLength; ++i) {
        command[2 * i + PREAMBLE_SIZE + 2] = static_cast<quint8>(descriptor[i].unicode());
        command[2 * i + PREAMBLE_SIZE + 3] = static_cast<quint8>(descriptor[i].unicode() >> 8);
    }
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// "Equal to" operator for ChipSettings
bool MCP2210::ChipSettings::operator ==(const MCP2210::ChipSettings &other) const
{
    return gp0 == other.gp0 && gp1 == other.gp1 && gp2 == other.gp2 && gp3 == other.gp3 && gp4 == other.gp4 && gp5 == other.gp5 && gp6 == other.gp6 && gp7 == other.gp7 && gp8 == other.gp8 && gpdir == other.gpdir && gpout == other.gpout && rmwakeup == other.rmwakeup && intmode == other.intmode && nrelspi == other.nrelspi;
}

// "Not equal to" operator for ChipSettings
bool MCP2210::ChipSettings::operator !=(const MCP2210::ChipSettings &other) const
{
    return !(operator ==(other));
}

// "Equal to" operator for ChipStatus
bool MCP2210::ChipStatus::operator ==(const MCP2210::ChipStatus &other) const
{
    return busreq == other.busreq && busowner == other.busowner && pwtries == other.pwtries && pwok == other.pwok;
}

// "Not equal to" operator for ChipStatus
bool MCP2210::ChipStatus::operator !=(const MCP2210::ChipStatus &other) const
{
    return !(operator ==(other));
}

// "Equal to" operator for SPISettings
bool MCP2210::SPISettings::operator ==(const MCP2210::SPISettings &other) const
{
    return nbytes == other.nbytes && bitrate == other.bitrate && mode == other.mode && actcs == other.actcs && idlcs == other.idlcs && csdtdly == other.csdtdly && dtcsdly == other.dtcsdly && itbytdly == other.itbytdly;
}

// "Not equal to" operator for SPISettings
bool MCP2210::SPISettings::operator !=(const MCP2210::SPISettings &other) const
{
    return !(operator ==(other));
}

// "Equal to" operator for USBParameters
bool MCP2210::USBParameters::operator ==(const MCP2210::USBParameters &other) const
{
    return vid == other.vid && pid == other.pid && maxpow == other.maxpow && powmode == other.powmode && rmwakeup == other.rmwakeup;
}

// "Not equal to" operator for USBParameters
bool MCP2210::USBParameters::operator !=(const MCP2210::USBParameters &other) const
{
    return !(operator ==(other));
}

MCP2210::MCP2210() :
    context_(nullptr),
    handle_(nullptr),
    disconnected_(false),
    kernelWasAttached_(false)
{
}

MCP2210::~MCP2210()
{
    close();  // The destructor is used to close the device, and this is essential so the device can be freed when the parent object is destroyed
}

// Diagnostic function used to verify if the device has been disconnected
bool MCP2210::disconnected() const
{
    return disconnected_;  // Returns true if the device has been disconnected, or false otherwise
}

// Checks if the device is open
bool MCP2210::isOpen() const
{
    return handle_ != nullptr;  // Returns true if the device is open, or false otherwise
}

// Cancels the ongoing SPI transfer
quint8 MCP2210::cancelSPITransfer(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        CANCEL_SPI_TRANSFER  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// Closes the device safely, if open
void MCP2210::close()
{
    if (isOpen()) {  // This condition avoids a segmentation fault if the calling algorithm tries, for some reason, to close the same device twice (e.g., if the device is already closed when the destructor is called)
        libusb_release_interface(handle_, 0);  // Release the interface
        if (kernelWasAttached_) {  // If a kernel driver was attached to the interface before
            libusb_attach_kernel_driver(handle_, 0);  // Reattach the kernel driver
        }
        libusb_close(handle_);  // Close the device
        libusb_exit(context_);  // Deinitialize libusb
        handle_ = nullptr;  // Required to mark the device as closed
    }
}

// Configures volatile chip settings
quint8 MCP2210::configureChipSettings(const ChipSettings &settings, int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        SET_CHIP_SETTINGS, 0x00, 0x00, 0x00,                                                             // Header
        settings.gp0,                                                                                    // GP0 pin configuration
        settings.gp1,                                                                                    // GP1 pin configuration
        settings.gp2,                                                                                    // GP2 pin configuration
        settings.gp3,                                                                                    // GP3 pin configuration
        settings.gp4,                                                                                    // GP4 pin configuration
        settings.gp5,                                                                                    // GP5 pin configuration
        settings.gp6,                                                                                    // GP6 pin configuration
        settings.gp7,                                                                                    // GP7 pin configuration
        settings.gp8,                                                                                    // GP8 pin configuration
        settings.gpout, 0x00,                                                                            // Default GPIO outputs (GPIO7 to GPIO0)
        settings.gpdir, 0x01,                                                                            // Default GPIO directions (GPIO7 to GPIO0)
        static_cast<quint8>(settings.rmwakeup << 4 | (0x07 & settings.intmode) << 1 | settings.nrelspi)  // Other chip settings
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// Configures volatile SPI transfer settings
quint8 MCP2210::configureSPISettings(const SPISettings &settings, int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        SET_SPI_SETTINGS, 0x00, 0x00, 0x00,                                                        // Header
        static_cast<quint8>(settings.bitrate), static_cast<quint8>(settings.bitrate >> 8),         // Bit rate
        static_cast<quint8>(settings.bitrate >> 16), static_cast<quint8>(settings.bitrate >> 24),
        settings.idlcs, 0x00,                                                                      // Idle chip select (CS7 to CS0)
        settings.actcs, 0x00,                                                                      // Active chip select (CS7 to CS0)
        static_cast<quint8>(settings.csdtdly), static_cast<quint8>(settings.csdtdly >> 8),         // Chip select to data delay
        static_cast<quint8>(settings.dtcsdly), static_cast<quint8>(settings.dtcsdly >> 8),         // Data to chip select delay
        static_cast<quint8>(settings.itbytdly), static_cast<quint8>(settings.itbytdly >> 8),       // Inter-byte delay
        static_cast<quint8>(settings.nbytes), static_cast<quint8>(settings.nbytes >> 8),           // Number of bytes per SPI transaction
        settings.mode                                                                              // SPI mode
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// Retrieves the access control mode from the MCP2210 NVRAM
quint8 MCP2210::getAccessControlMode(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_NVRAM_SETTINGS, NV_CHIP_SETTINGS  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(18);  // Access control mode corresponds to byte 18
}

// Returns applied chip settings
MCP2210::ChipSettings MCP2210::getChipSettings(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_CHIP_SETTINGS  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    ChipSettings settings;
    settings.gp0 = response.at(4);                                        // GP0 pin configuration corresponds to byte 4
    settings.gp1 = response.at(5);                                        // GP1 pin configuration corresponds to byte 5
    settings.gp2 = response.at(6);                                        // GP2 pin configuration corresponds to byte 6
    settings.gp3 = response.at(7);                                        // GP3 pin configuration corresponds to byte 7
    settings.gp4 = response.at(8);                                        // GP4 pin configuration corresponds to byte 8
    settings.gp5 = response.at(9);                                        // GP5 pin configuration corresponds to byte 9
    settings.gp6 = response.at(10);                                       // GP6 pin configuration corresponds to byte 10
    settings.gp7 = response.at(11);                                       // GP7 pin configuration corresponds to byte 11
    settings.gp8 = response.at(12);                                       // GP8 pin configuration corresponds to byte 12
    settings.gpdir = response.at(15);                                     // Default GPIO directions (GPIO7 to GPIO0) corresponds to byte 15
    settings.gpout = response.at(13);                                     // Default GPIO outputs (GPIO7 to GPIO0) corresponds to byte 13
    settings.rmwakeup = (0x10 & response.at(17)) != 0x00;                 // Remote wake-up corresponds to bit 4 of byte 17
    settings.intmode = static_cast<quint8>(0x07 & response.at(17) >> 1);  // Interrupt counting mode corresponds to bits 3:1 of byte 17
    settings.nrelspi = (0x01 & response.at(17)) != 0x00;                  // SPI bus release corresponds to bit 0 of byte 17
    return settings;
}

// Returns the current status
MCP2210::ChipStatus MCP2210::getChipStatus(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_CHIP_STATUS  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    ChipStatus status;
    status.busreq = response.at(2) != 0x01;  // SPI bus release external request status corresponds to byte 2
    status.busowner = response.at(3);        // SPI bus current owner corresponds to byte 3
    status.pwtries = response.at(4);         // Number of NVRAM password tries corresponds to byte 4
    status.pwok = response.at(5) != 0x00;    // Password validation status corresponds to byte 5
    return status;
}

// Gets the number of events from the interrupt pin
quint16 MCP2210::getEventCount(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_EVENT_COUNT,  // Header
        0x01              // Do not reset the event counter
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return static_cast<quint16>(response.at(5) << 8 | response.at(4));  // Event count corresponds to bytes 4 and 5 (little-endian conversion)
}

// Returns the value of a given GPIO pin on the MCP2210
bool MCP2210::getGPIO(int gpio, int &errcnt, QString &errstr)
{
    bool value;
    if (gpio < GPIO0 || gpio > GPIO8) {
        ++errcnt;
        errstr += QObject::tr("In getGPIO(): GPIO pin number must be between 0 and 8.\n");  // Program logic error
        value = false;
    } else {
        value = (0x0001 << gpio & getGPIOs(errcnt, errstr)) != 0x0000;
    }
    return value;
}

// Returns the direction of a given GPIO pin on the MCP2210
bool MCP2210::getGPIODirection(int gpio, int &errcnt, QString &errstr)
{
    bool direction;
    if (gpio < GPIO0 || gpio > GPIO7) {
        ++errcnt;
        errstr += QObject::tr("In getGPIODirection(): GPIO pin number must be between 0 and 7.\n");  // Program logic error
        direction = false;
    } else {
        direction = (0x01 << gpio & getGPIODirections(errcnt, errstr)) != 0x00;
    }
    return direction;
}

// Returns the directions of all GPIO pins on the MCP2210
quint8 MCP2210::getGPIODirections(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_GPIO_DIRECTIONS  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(4);  // GPIO directions (GPIO7 to GPIO0) corresponds to byte 4
}

// Returns the values of all GPIO pins on the MCP2210
quint16 MCP2210::getGPIOs(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_GPIO_VALUES  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return static_cast<quint16>((0x01 & response.at(5)) << 8 | response.at(4));  // GPIO values (GPIO8 to GPIO0) corresponds to bytes 4 and 5
}

// Retrieves the manufacturer descriptor from the MCP2210 NVRAM
QString MCP2210::getManufacturerDesc(int &errcnt, QString &errstr)
{
    return getDescGeneric(MANUFACTURER_NAME, errcnt, errstr);
}

// Retrieves the power-up (non-volatile) chip settings from the MCP2210 NVRAM
MCP2210::ChipSettings MCP2210::getNVChipSettings(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_NVRAM_SETTINGS, NV_CHIP_SETTINGS  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    ChipSettings settings;
    settings.gp0 = response.at(4);                                        // GP0 pin configuration corresponds to byte 4
    settings.gp1 = response.at(5);                                        // GP1 pin configuration corresponds to byte 5
    settings.gp2 = response.at(6);                                        // GP2 pin configuration corresponds to byte 6
    settings.gp3 = response.at(7);                                        // GP3 pin configuration corresponds to byte 7
    settings.gp4 = response.at(8);                                        // GP4 pin configuration corresponds to byte 8
    settings.gp5 = response.at(9);                                        // GP5 pin configuration corresponds to byte 9
    settings.gp6 = response.at(10);                                       // GP6 pin configuration corresponds to byte 10
    settings.gp7 = response.at(11);                                       // GP7 pin configuration corresponds to byte 11
    settings.gp8 = response.at(12);                                       // GP8 pin configuration corresponds to byte 12
    settings.gpdir = response.at(15);                                     // Default GPIO directions (GPIO7 to GPIO0) corresponds to byte 15
    settings.gpout = response.at(13);                                     // Default GPIO outputs (GPIO7 to GPIO0) corresponds to byte 13
    settings.rmwakeup = (0x10 & response.at(17)) != 0x00;                 // Remote wake-up corresponds to bit 4 of byte 17
    settings.intmode = static_cast<quint8>(0x07 & response.at(17) >> 1);  // Interrupt counting mode corresponds to bits 3:1 of byte 17
    settings.nrelspi = (0x01 & response.at(17)) != 0x00;                  // SPI bus release corresponds to bit 0 of byte 17
    return settings;
}

// Retrieves the power-up (non-volatile) SPI transfer settings from the MCP2210 NVRAM
MCP2210::SPISettings MCP2210::getNVSPISettings(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_NVRAM_SETTINGS, NV_SPI_SETTINGS  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    SPISettings settings;
    settings.nbytes = static_cast<quint16>(response.at(19) << 8 | response.at(18));                                               // Number of bytes per SPI transfer corresponds to bytes 18 and 19 (little-endian conversion)
    settings.bitrate = static_cast<quint32>(response.at(7) << 24 | response.at(6) << 16 | response.at(5) << 8 | response.at(4));  // Bit rate corresponds to bytes 4 to 7 (little-endian conversion)
    settings.mode = response.at(20);                                                                                              // SPI mode corresponds to byte 20
    settings.actcs = response.at(10);                                                                                             // Active chip select (CS7 to CS0) corresponds to byte 10
    settings.idlcs = response.at(8);                                                                                              // Idle chip select (CS7 to CS0) corresponds to byte 8
    settings.csdtdly = static_cast<quint16>(response.at(13) << 8 | response.at(12));                                              // Chip select to data corresponds to bytes 12 and 13 (little-endian conversion)
    settings.dtcsdly = static_cast<quint16>(response.at(15) << 8 | response.at(14));                                              // Data to chip select delay corresponds to bytes 14 and 15 (little-endian conversion)
    settings.itbytdly = static_cast<quint16>(response.at(17) << 8 | response.at(16));                                             // Inter-byte delay corresponds to bytes 16 and 17 (little-endian conversion)
    return settings;
}

// Retrieves the product descriptor from the MCP2210 NVRAM
QString MCP2210::getProductDesc(int &errcnt, QString &errstr)
{
    return getDescGeneric(PRODUCT_NAME, errcnt, errstr);
}

// Returns applied SPI transfer settings
MCP2210::SPISettings MCP2210::getSPISettings(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_SPI_SETTINGS  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    SPISettings settings;
    settings.nbytes = static_cast<quint16>(response.at(19) << 8 | response.at(18));                                               // Number of bytes per SPI transfer corresponds to bytes 18 and 19 (little-endian conversion)
    settings.bitrate = static_cast<quint32>(response.at(7) << 24 | response.at(6) << 16 | response.at(5) << 8 | response.at(4));  // Bit rate corresponds to bytes 4 to 7 (little-endian conversion)
    settings.mode = response.at(20);                                                                                              // SPI mode corresponds to byte 20
    settings.actcs = response.at(10);                                                                                             // Active chip select (CS7 to CS0) corresponds to byte 10
    settings.idlcs = response.at(8);                                                                                              // Idle chip select (CS7 to CS0) corresponds to byte 8
    settings.csdtdly = static_cast<quint16>(response.at(13) << 8 | response.at(12));                                              // Chip select to data corresponds to bytes 12 and 13 (little-endian conversion)
    settings.dtcsdly = static_cast<quint16>(response.at(15) << 8 | response.at(14));                                              // Data to chip select delay corresponds to bytes 14 and 15 (little-endian conversion)
    settings.itbytdly = static_cast<quint16>(response.at(17) << 8 | response.at(16));                                             // Inter-byte delay corresponds to bytes 16 and 17 (little-endian conversion)
    return settings;
}

// Gets the USB parameters, namely VID, PID and power settings
MCP2210::USBParameters MCP2210::getUSBParameters(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_NVRAM_SETTINGS, USB_PARAMETERS  // Header
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    USBParameters parameters;
    parameters.vid = static_cast<quint16>(response.at(13) << 8 | response.at(12));  // Vendor ID corresponds to bytes 12 and 13 (little-endian conversion)
    parameters.pid = static_cast<quint32>(response.at(15) << 8 | response.at(14));  // Product ID corresponds to bytes 14 and 15 (little-endian conversion)
    parameters.maxpow = response.at(30);                                            // Maximum consumption current corresponds to byte 30
    parameters.powmode = (0x40 & response.at(29)) != 0x00;                          // Power mode corresponds to bit 6 of byte 29 (bit 7 is redundant)
    parameters.rmwakeup = (0x20 & response.at(29)) != 0x00;                         // Remote wake-up capability corresponds to bit 5 of byte 29
    return parameters;
}

// Sends a HID command based on the given vector, and returns the response
// The command vector can be shorter or longer than 64 bytes, but the resulting command will either be padded with zeros or truncated in order to fit
QVector<quint8> MCP2210::hidTransfer(const QVector<quint8> &data, int &errcnt, QString &errstr)
{
    size_t vecSize = static_cast<size_t>(data.size());
    size_t bytesToFill = vecSize > COMMAND_SIZE ? COMMAND_SIZE : vecSize;
    unsigned char commandBuffer[COMMAND_SIZE] = {0x00};  // It is important to initialize the array in this manner, so that unused indexes are filled with zeros!
    for (size_t i = 0; i < bytesToFill; ++i) {
        commandBuffer[i] = data[i];
    }
    int preverrcnt = errcnt;
#if LIBUSB_API_VERSION >= 0x01000105
    interruptTransfer(EPOUT, commandBuffer, static_cast<int>(COMMAND_SIZE), nullptr, errcnt, errstr);
#else
    int bytesWritten;
    interruptTransfer(EPOUT, commandBuffer, static_cast<int>(COMMAND_SIZE), &bytesWritten, errcnt, errstr);
#endif
    unsigned char responseBuffer[COMMAND_SIZE];
    int bytesRead = 0;  // Important!
    interruptTransfer(EPIN, responseBuffer, static_cast<int>(COMMAND_SIZE), &bytesRead, errcnt, errstr);
    QVector<quint8> retdata(static_cast<int>(COMMAND_SIZE));
    for (int i = 0; i < bytesRead; ++i) {
        retdata[i] = responseBuffer[i];
    }
    if (errcnt == preverrcnt && (bytesRead < static_cast<int>(COMMAND_SIZE) || responseBuffer[0] != commandBuffer[0])) {  // This additional verification only makes sense if the error count does not increase
        ++errcnt;
        errstr += QObject::tr("Received invalid response to HID command.\n");
    }
    return retdata;
}

// Opens the device having the given VID, PID and, optionally, the given serial number, and assigns its handle
int MCP2210::open(quint16 vid, quint16 pid, const QString &serial)
{
    int retval;
    if (isOpen()) {  // Just in case the calling algorithm tries to open a device that was already sucessfully open, or tries to open different devices concurrently, all while using (or referencing to) the same object
        retval = SUCCESS;
    } else if (libusb_init(&context_) != 0) {  // Initialize libusb. In case of failure
        retval = ERROR_INIT;
    } else {  // If libusb is initialized
        if (serial.isNull()) {  // Note that serial, by omission, is a null QString
            handle_ = libusb_open_device_with_vid_pid(context_, vid, pid);  // If no serial number is specified, this will open the first device found with matching VID and PID
        } else {
            handle_ = libusb_open_device_with_vid_pid_serial(context_, vid, pid, reinterpret_cast<unsigned char *>(serial.toLatin1().data()));
        }
        if (handle_ == nullptr) {  // If the previous operation fails to get a device handle
            libusb_exit(context_);  // Deinitialize libusb
            retval = ERROR_NOT_FOUND;
        } else {  // If the device is successfully opened and a handle obtained
            if (libusb_kernel_driver_active(handle_, 0) == 1) {  // If a kernel driver is active on the interface
                libusb_detach_kernel_driver(handle_, 0);  // Detach the kernel driver
                kernelWasAttached_ = true;  // Flag that the kernel driver was attached
            } else {
                kernelWasAttached_ = false;  // The kernel driver was not attached
            }
            if (libusb_claim_interface(handle_, 0) != 0) {  // Claim the interface. In case of failure
                if (kernelWasAttached_) {  // If a kernel driver was attached to the interface before
                    libusb_attach_kernel_driver(handle_, 0);  // Reattach the kernel driver
                }
                libusb_close(handle_);  // Close the device
                libusb_exit(context_);  // Deinitialize libusb
                handle_ = nullptr;  // Required to mark the device as closed
                retval = ERROR_BUSY;
            } else {
                disconnected_ = false;  // Note that this flag is never assumed to be true for a device that was never opened - See constructor for details!
                retval = SUCCESS;
            }
        }
    }
    return retval;
}

// Reads a byte from the given EEPROM address
quint8 MCP2210::readEEPROMByte(quint8 address, int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        READ_EEPROM,  // Header
        address       // Address to be read
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(3);
}

// Reads the EEPROM within the specified range, returning a vector
// If an error occurs, the size of the vector will be smaller than expected
QVector<quint8> MCP2210::readEEPROMRange(quint8 begin, quint8 end, int &errcnt, QString &errstr)
{
    QVector<quint8> values;
    if (begin > end) {
        ++errcnt;
        errstr += QObject::tr("In readEEPROMRange(): the first address cannot be greater than the last address.\n");  // Program logic error
    } else {
        for (size_t i = begin; i <= end; ++i) {
            int preverrcnt = errcnt;
            quint8 value = readEEPROMByte(static_cast<quint8>(i), errcnt, errstr);
            if (errcnt != preverrcnt) {  // If an error occurs
                break;  // Abort
            }
            values.push_back(value);
        }
    }
    return values;
}

// Resets the interrupt event counter
quint8 MCP2210::resetEventCounter(int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        GET_EVENT_COUNT,  // Header
        0x00              // Reset the event counter
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// Sets the value of a given GPIO pin on the MCP2210
quint8 MCP2210::setGPIO(int gpio, bool value, int &errcnt, QString &errstr)
{
    quint8 retval;
    if (gpio < GPIO0 || gpio > GPIO7) {
        ++errcnt;
        errstr += QObject::tr("In setGPIO(): GPIO pin number must be between 0 and 7.\n");  // Program logic error
        retval = OTHER_ERROR;
    } else {
        int preverrcnt = errcnt;
        quint16 values = getGPIOs(errcnt, errstr);
        if (errcnt == preverrcnt) {
            quint16 mask = static_cast<quint16>(0x0001 << gpio);
            if (value) {  // If the selected GPIO pin value is set to be high
                values = static_cast<quint16>(mask | values);  // Set pin high
            } else {
                values = static_cast<quint16>(~mask & values);  // Set pin low
            }
            retval = setGPIOs(values, errcnt, errstr);
        } else {
            retval = OTHER_ERROR;
        }
    }
    return retval;
}

// Sets the direction of a given GPIO pin on the MCP2210
quint8 MCP2210::setGPIODirection(int gpio, bool direction, int &errcnt, QString &errstr)
{
    quint8 retval;
    if (gpio < GPIO0 || gpio > GPIO7) {
        ++errcnt;
        errstr += QObject::tr("In setGPIODirection(): GPIO pin number must be between 0 and 7.\n");  // Program logic error
        retval = OTHER_ERROR;
    } else {
        int preverrcnt = errcnt;
        quint8 directions = getGPIODirections(errcnt, errstr);
        if (errcnt == preverrcnt) {
            quint8 mask = static_cast<quint8>(0x01 << gpio);
            if (direction) {  // If the selected GPIO pin is to be used as an input
                directions = static_cast<quint8>(mask | directions);  // Set pin as input
            } else {
                directions = static_cast<quint8>(~mask & directions);  // Set pin as output
            }
            retval = setGPIODirections(directions, errcnt, errstr);
        } else {
            retval = OTHER_ERROR;
        }
    }
    return retval;
}

// Sets the directions of all GPIO pins on the MCP2210
quint8 MCP2210::setGPIODirections(quint8 directions, int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        SET_GPIO_DIRECTIONS, 0x00, 0x00, 0x00,  // Header
        directions, 0x01                        // GPIO directions (GPIO7 to GPIO0)
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// Sets the values of all GPIO pins on the MCP2210
quint8 MCP2210::setGPIOs(quint16 values, int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        SET_GPIO_VALUES, 0x00, 0x00, 0x00,  // Header
        static_cast<quint8>(values)         // GPIO values (GPIO7 to GPPIO0 - GPIO8 is an input only pin)
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// Performs a basic SPI transfer
// Note that the variable "status" is used to return either the SPI transfer engine status or, in case of error, the HID command response
QVector<quint8> MCP2210::spiTransfer(const QVector<quint8> &data, quint8 &status, int &errcnt, QString &errstr)
{
    QVector<quint8> retdata;
    size_t bytesToSend = static_cast<size_t>(data.size());
    if (bytesToSend > SPIDATA_MAXSIZE) {
        ++errcnt;
        errstr += QObject::tr("In spiTransfer(): vector size cannot exceed 60 bytes.\n");  // Program logic error
    } else {
        QVector<quint8> command(bytesToSend + PREAMBLE_SIZE);
        command[0] = TRANSFER_SPI_DATA;                 // Header
        command[1] = static_cast<quint8>(bytesToSend);  // Number of bytes to send
        for (size_t i = 0; i < bytesToSend; ++i) {
            command[i + PREAMBLE_SIZE] = data[i];
        }
        QVector<quint8> response = hidTransfer(command, errcnt, errstr);
        if (response.at(1) == COMPLETED) {  // If the HID transfer was completed
            status = response.at(3);  // The returned status corresponds to the obtained SPI transfer engine status
            size_t bytesReceived = response.at(2);
            retdata.resize(bytesReceived);
            for (size_t i = 0; i < bytesReceived; ++i) {
                retdata[i] = response.at(i + PREAMBLE_SIZE);
            }
        } else {
            status = response.at(1);  // The returned status corresponds to the obtained HID command response (it can be "BUSY" [0xf7] or "IN_PROGRESS" [0xf8])
        }
    }
    return retdata;
}

// Toggles (inverts the value of) a given GPIO pin on the MCP2210
quint8 MCP2210::toggleGPIO(int gpio, int &errcnt, QString &errstr)
{
    quint8 retval;
    if (gpio < GPIO0 || gpio > GPIO7) {
        ++errcnt;
        errstr += QObject::tr("In toggleGPIO(): GPIO pin number must be between 0 and 7.\n");  // Program logic error
        retval = OTHER_ERROR;
    } else {
        int preverrcnt = errcnt;
        quint16 values = getGPIOs(errcnt, errstr);
        if (errcnt == preverrcnt) {
            quint16 mask = static_cast<quint16>(0x0001 << gpio);
            if ((mask & values) == 0x0000) {  // If the selected GPIO pin is low
                values = static_cast<quint16>(mask | values);  // Toggle pin high
            } else {
                values = static_cast<quint16>(~mask & values);  // Toggle pin low
            }
            retval = setGPIOs(values, errcnt, errstr);
        } else {
            retval = OTHER_ERROR;
        }
    }
    return retval;
}

// Sends password over to the MCP2210
// This function should be called before modifying a setting in the NVRAM, if a password is set
quint8 MCP2210::usePassword(const QString &password, int &errcnt, QString &errstr)
{
    quint8 retval;
    QByteArray passwordLatin1 = password.toLatin1();
    int passwordLength = passwordLatin1.size();
    if (passwordLength > static_cast<int>(PASSWORD_MAXLEN)) {
        ++errcnt;
        errstr += "In usePassword(): password cannot be longer than 8 characters.\n";  // Program logic error
        retval = OTHER_ERROR;
    } else if (password != passwordLatin1) {
        ++errcnt;
        errstr += "In usePassword(): password cannot have non-latin characters.\n";  // Program logic error
        retval = OTHER_ERROR;
    } else {
        QVector<quint8> command(passwordLength + PREAMBLE_SIZE);
        command[0] = SEND_PASSWORD;  // Header
        for (int i = 0; i < passwordLength; ++i) {
            command[i + PREAMBLE_SIZE] = static_cast<quint8>(passwordLatin1[i]);
        }
        QVector<quint8> response = hidTransfer(command, errcnt, errstr);
        retval = response.at(1);
    }
    return retval;
}

// Writes a byte to a given EEPROM address
quint8 MCP2210::writeEEPROMByte(quint8 address, quint8 value, int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        WRITE_EEPROM,  // Header
        address,       // Address to be written
        value          // Value
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// Writes over the EEPROM, within the specified range and based on the given vector
quint8 MCP2210::writeEEPROMRange(quint8 begin, quint8 end, const QVector<quint8> &values, int &errcnt, QString &errstr)
{
    quint8 retval;
    if (begin > end) {
        ++errcnt;
        errstr += QObject::tr("In writeEEPROMRange(): the first address cannot be greater than the last address.\n");  // Program logic error
        retval = OTHER_ERROR;
    } else {
        int vecSize = values.size();
        if (vecSize != end - begin + 1) {
            ++errcnt;
            errstr += QObject::tr("In writeEEPROMRange(): vector size does not match range size.\n");  // Program logic error
            retval = OTHER_ERROR;
        } else {
            retval = COMPLETED;  // Fix applied in version 1.1.0
            for (int i = 0; i < vecSize; ++i) {
                int preverrcnt = errcnt;
                retval = writeEEPROMByte(static_cast<quint8>(begin + i), values[i], errcnt, errstr);
                if (errcnt != preverrcnt || retval != COMPLETED) {  // If an error occurs
                    break;  // Abort
                }
            }
        }
    }
    return retval;
}

// Writes the manufacturer descriptor to the MCP2210 OTP NVRAM
quint8 MCP2210::writeManufacturerDesc(const QString &manufacturer, int &errcnt, QString &errstr)
{
    quint8 retval;
    if (static_cast<size_t>(manufacturer.size()) > DESC_MAXLEN) {
        ++errcnt;
        errstr += QObject::tr("In writeManufacturerDesc(): manufacturer descriptor string cannot be longer than 28 characters.\n");  // Program logic error
        retval = OTHER_ERROR;
    } else {
        retval = writeDescGeneric(manufacturer, MANUFACTURER_NAME, errcnt, errstr);
    }
    return retval;
}

// Writes the given chip transfer settings to the MCP2210 OTP NVRAM, while also setting the access control mode and password (expanded in version 1.1.0)
// Note that using an empty string for the password will have the effect of leaving it unchanged
quint8 MCP2210::writeNVChipSettings(const ChipSettings &settings, quint8 accessControlMode, const QString &password, int &errcnt, QString &errstr)
{
    quint8 retval;
    QByteArray passwordLatin1 = password.toLatin1();
    int passwordLength = passwordLatin1.size();
    if (accessControlMode != ACNONE && accessControlMode != ACPASSWORD && accessControlMode != ACLOCKED) {
        ++errcnt;
        errstr += "In writeNVChipSettings(): the specified access control mode is not supported.\n";  // Program logic error
        retval = OTHER_ERROR;
    } else if (passwordLength > static_cast<int>(PASSWORD_MAXLEN)) {
        ++errcnt;
        errstr += "In writeNVChipSettings(): password cannot be longer than 8 characters.\n";  // Program logic error
        retval = OTHER_ERROR;
    } else if (password != passwordLatin1) {
        ++errcnt;
        errstr += "In writeNVChipSettings(): password cannot have non-latin characters.\n";  // Program logic error
        retval = OTHER_ERROR;
    } else {
        QVector<quint8> command(passwordLength + 19);
        command[0] = SET_NVRAM_SETTINGS;                                                                                // Header
        command[1] = NV_CHIP_SETTINGS;
        command[4] = settings.gp0;                                                                                      // GP0 pin configuration
        command[5] = settings.gp1;                                                                                      // GP1 pin configuration
        command[6] = settings.gp2;                                                                                      // GP2 pin configuration
        command[7] = settings.gp3;                                                                                      // GP3 pin configuration
        command[8] = settings.gp4;                                                                                      // GP4 pin configuration
        command[9] = settings.gp5;                                                                                      // GP5 pin configuration
        command[10] = settings.gp6;                                                                                     // GP6 pin configuration
        command[11] = settings.gp7;                                                                                     // GP7 pin configuration
        command[12] = settings.gp8,                                                                                     // GP8 pin configuration
        command[13] = settings.gpout;                                                                                   // Default GPIO outputs (GPIO7 to GPIO0)
        command[15] = settings.gpdir;                                                                                   // Default GPIO directions (GPIO7 to GPIO0)
        command[16] = 0x01;
        command[17] = static_cast<quint8>(settings.rmwakeup << 4 | (0x07 & settings.intmode) << 1 | settings.nrelspi);  // Other chip settings
        command[18] = accessControlMode;                                                                                // Access control mode
        for (int i = 0; i < passwordLength; ++i) {
            command[i + 19] = static_cast<quint8>(passwordLatin1[i]);
        }
        QVector<quint8> response = hidTransfer(command, errcnt, errstr);
        retval = response.at(1);
    }
    return retval;
}

// Writes the given chip transfer settings to the MCP2210 OTP NVRAM (this overloaded function is functionally equivalent to the implementation of writeNVChipSettings() that is found in version 1.0.0)
// The use of this variant of writeNVChipSettings() sets the access control mode to "ACNONE" [0x00], but the password is kept unchanged
quint8 MCP2210::writeNVChipSettings(const ChipSettings &settings, int &errcnt, QString &errstr)
{
    return writeNVChipSettings(settings, ACNONE, "", errcnt, errstr);
}

// Writes the given SPI transfer settings to the MCP2210 OTP NVRAM
quint8 MCP2210::writeNVSPISettings(const SPISettings &settings, int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        SET_NVRAM_SETTINGS, NV_SPI_SETTINGS, 0x00, 0x00,                                           // Header
        static_cast<quint8>(settings.bitrate), static_cast<quint8>(settings.bitrate >> 8),         // Bit rate
        static_cast<quint8>(settings.bitrate >> 16), static_cast<quint8>(settings.bitrate >> 24),
        settings.idlcs, 0x00,                                                                      // Idle chip select (CS7 to CS0)
        settings.actcs, 0x00,                                                                      // Active chip select (CS7 to CS0)
        static_cast<quint8>(settings.csdtdly), static_cast<quint8>(settings.csdtdly >> 8),         // Chip select to data delay
        static_cast<quint8>(settings.dtcsdly), static_cast<quint8>(settings.dtcsdly >> 8),         // Data to chip select delay
        static_cast<quint8>(settings.itbytdly), static_cast<quint8>(settings.itbytdly >> 8),       // Inter-byte delay
        static_cast<quint8>(settings.nbytes), static_cast<quint8>(settings.nbytes >> 8),           // Number of bytes per SPI transaction
        settings.mode                                                                              // SPI mode
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// Writes the product descriptor to the MCP2210 OTP NVRAM
quint8 MCP2210::writeProductDesc(const QString &product, int &errcnt, QString &errstr)
{
    quint8 retval;
    if (static_cast<size_t>(product.size()) > DESC_MAXLEN) {
        ++errcnt;
        errstr += QObject::tr("In writeProductDesc(): product descriptor string cannot be longer than 28 characters.\n");  // Program logic error
        retval = OTHER_ERROR;
    } else {
        retval = writeDescGeneric(product, PRODUCT_NAME, errcnt, errstr);
    }
    return retval;
}

// Writes the USB parameters to the MCP2210 OTP NVRAM
quint8 MCP2210::writeUSBParameters(const USBParameters &parameters, int &errcnt, QString &errstr)
{
    QVector<quint8> command{
        SET_NVRAM_SETTINGS, USB_PARAMETERS, 0x00, 0x00,                                                      // Header
        static_cast<quint8>(parameters.vid), static_cast<quint8>(parameters.vid >> 8),                       // Vendor ID
        static_cast<quint8>(parameters.pid), static_cast<quint8>(parameters.pid >> 8),                       // Product ID
        static_cast<quint8>(!parameters.powmode << 7 | parameters.powmode << 6 | parameters.rmwakeup << 5),  // Chip power options
        parameters.maxpow                                                                                    // Maximum consumption current
    };
    QVector<quint8> response = hidTransfer(command, errcnt, errstr);
    return response.at(1);
}

// Helper function to list devices
QStringList MCP2210::listDevices(quint16 vid, quint16 pid, int &errcnt, QString &errstr)
{
    QStringList devices;
    libusb_context *context;
    if (libusb_init(&context) != 0) {  // Initialize libusb. In case of failure
        ++errcnt;
        errstr += QObject::tr("Could not initialize libusb.\n");
    } else {  // If libusb is initialized
        libusb_device **devs;
        ssize_t devlist = libusb_get_device_list(context, &devs);  // Get a device list
        if (devlist < 0) {  // If the previous operation fails to get a device list
            ++errcnt;
            errstr += QObject::tr("Failed to retrieve a list of devices.\n");
        } else {
            for (ssize_t i = 0; i < devlist; ++i) {  // Run through all listed devices
                libusb_device_descriptor desc;
                if (libusb_get_device_descriptor(devs[i], &desc) == 0 && desc.idVendor == vid && desc.idProduct == pid) {  // If the device descriptor is retrieved, and both VID and PID correspond to the respective given values
                    libusb_device_handle *handle;
                    if (libusb_open(devs[i], &handle) == 0) {  // Open the listed device. If successfull
                        unsigned char str_desc[256];
                        libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, str_desc, static_cast<int>(sizeof(str_desc)));  // Get the serial number string in ASCII format
                        devices += reinterpret_cast<char *>(str_desc);  // Append the serial number string to the list
                        libusb_close(handle);  // Close the device
                    }
                }
            }
            libusb_free_device_list(devs, 1);  // Free device list
        }
        libusb_exit(context);  // Deinitialize libusb
    }
    return devices;
}
