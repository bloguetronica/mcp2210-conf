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


#ifndef MCP2210_H
#define MCP2210_H

// Includes
#include <QString>
#include <QStringList>
#include <QVector>
#include <libusb-1.0/libusb.h>

class MCP2210
{
private:
    libusb_context *context_;
    libusb_device_handle *handle_;
    bool disconnected_, kernelWasAttached_;

    QString getDescGeneric(quint8 subcomid, int &errcnt, QString &errstr);
    void interruptTransfer(quint8 endpointAddr, unsigned char *data, int length, int *transferred, int &errcnt, QString &errstr);
    quint8 writeDescGeneric(const QString &descriptor, quint8 subcomid, int &errcnt, QString &errstr);

public:
    // Class definitions
    static const quint16 VID = 0x04d8;                                   // Default USB vendor ID
    static const quint16 PID = 0x00de;                                   // Default USB product ID
    static const int SUCCESS = 0;                                        // Returned by open() if successful
    static const int ERROR_INIT = 1;                                     // Returned by open() in case of a libusb initialization failure
    static const int ERROR_NOT_FOUND = 2;                                // Returned by open() if the device was not found
    static const int ERROR_BUSY = 3;                                     // Returned by open() if the device is already in use
    static const size_t COMMAND_SIZE = 64;                               // HID command size
    static const size_t PREAMBLE_SIZE = 4;                               // HID command preamble size
    static const size_t SPIDATA_MAXSIZE = COMMAND_SIZE - PREAMBLE_SIZE;  // Maximum size of the data vector [60] for a single SPI transfer (only applicable to basic SPI transfers)
    static const size_t PASSWORD_MAXLEN = 8;                             // Maximum length for the password

    // Descriptor specific definitions
    static const size_t DESC_MAXLEN = 28;  // Maximum length for any descriptor

    // EEPROM specific definitions
    static const size_t EEPROM_SIZE = 256;    // EEPROM size in bytes
    static const quint8 EEPROM_BEGIN = 0x00;  // EEPROM first address
    static const quint8 EEPROM_END = 0xff;    // EEPROM last address

    // HID command IDs
    static const quint8 GET_CHIP_STATUS = 0x10;      // Get chip status
    static const quint8 CANCEL_SPI_TRANSFER = 0x11;  // Cancel ongoing SPI transfer
    static const quint8 GET_EVENT_COUNT = 0x12;      // Get event count
    static const quint8 GET_CHIP_SETTINGS = 0x20;    // Get chip settings
    static const quint8 SET_CHIP_SETTINGS = 0x21;    // Set chip settings
    static const quint8 SET_GPIO_VALUES = 0x30;      // Set GPIO pin values
    static const quint8 GET_GPIO_VALUES = 0x31;      // Get GPIO pin values
    static const quint8 SET_GPIO_DIRECTIONS = 0x32;  // Set GPIO pin directions
    static const quint8 GET_GPIO_DIRECTIONS = 0x33;  // Get GPIO pin directions
    static const quint8 SET_SPI_SETTINGS = 0x40;     // Set SPI transfer settings
    static const quint8 GET_SPI_SETTINGS = 0x41;     // Get SPI transfer settings
    static const quint8 TRANSFER_SPI_DATA = 0x42;    // Transfer SPI data
    static const quint8 READ_EEPROM = 0x50;          // Read EEPROM
    static const quint8 WRITE_EEPROM = 0x51;         // Write EEPROM
    static const quint8 SET_NVRAM_SETTINGS = 0x60;   // Set NVRAM settings
    static const quint8 GET_NVRAM_SETTINGS = 0x61;   // Get NVRAM settings
    static const quint8 SEND_PASSWORD = 0x70;        // Send password

    // NVRAM settings sub-command IDs
    static const quint8 NV_SPI_SETTINGS = 0x10;    // Power-up (non-volatile) SPI transfer settings
    static const quint8 NV_CHIP_SETTINGS = 0x20;   // Power-up (non-volatile) chip settings
    static const quint8 USB_PARAMETERS = 0x30;     // USB parameters
    static const quint8 PRODUCT_NAME = 0x40;       // USB product name
    static const quint8 MANUFACTURER_NAME = 0x50;  // USB manufacturer name

    // HID command responses
    static const quint8 COMPLETED = 0x00;       // Command completed successfully
    static const quint8 BUSY = 0xf7;            // SPI bus not available
    static const quint8 IN_PROGRESS = 0xf8;     // USB or SPI transfer in progress (settings not written)
    static const quint8 UNKNOWN = 0xf9;         // Response to unknown command
    static const quint8 WRITE_FAILURE = 0xfa;   // EEPROM write failure
    static const quint8 BLOCKED = 0xfb;         // Access not allowed or blocked, or EEPROM is password protected
    static const quint8 REJECTED = 0xfc;        // Access rejected
    static const quint8 WRONG_PASSWORD = 0xfd;  // Wrong password (number of attempts is still within the limit)
    static const quint8 OTHER_ERROR = 0xff;     // Other error (check errcnt and errstr for details)

    // SPI transfer engine status, returned by spiTransfer()
    static const quint8 TRANSFER_FINISHED = 0x10;      // SPI transfer finished (no more data to send)
    static const quint8 TRANSFER_STARTED = 0x20;       // SPI transfer started (no data to receive)
    static const quint8 TRANSFER_NOT_FINISHED = 0x30;  // SPI transfer not finished (received data available)

    // Access control modes, returned by getAccessControlMode()
    static const quint8 ACNONE = 0x00;      // Chip settings not protected (no access control)
    static const quint8 ACPASSWORD = 0x40;  // Chip settings protected by password access
    static const quint8 ACLOCKED = 0x80;    // Chip settings permanently locked

    // The following values are applicable to ChipSettings/configureChipSettings()/getChipSettings()/getNVChipSettings()/writeNVChipSettings()
    static const quint8 PCGPIO = 0x00;   // Pin configured as GPIO
    static const quint8 PCCS = 0x01;     // Pin configured as chip select
    static const quint8 PCFUNC = 0x02;   // Pin configured as a dedicated function pin
    static const quint8 IMNOCNT = 0x00;  // Interrupt mode disabled (no interrupt counting)
    static const quint8 IMCNTFE = 0x01;  // Interrupt mode set to count falling edges
    static const quint8 IMCNTRE = 0x02;  // Interrupt mode set to count rising edges
    static const quint8 IMCNTLP = 0x03;  // Interrupt mode set to count low pulses
    static const quint8 IMCNTHP = 0x04;  // Interrupt mode set to count high pulses

    // The following values are applicable to ChipStatus/getChipStatus()
    static const bool REQNO = false;   // No external request for SPI bus release
    static const bool REQPEND = true;  // Pending external request for SPI bus release
    static const quint8 BONO = 0x00;   // SPI bus has no owner
    static const quint8 BOOWN = 0x01;  // SPI bus owned by this master
    static const quint8 BOEXT = 0x02;  // SPI bus owned by external master
    static const bool PWNO = false;    // Password not guessed
    static const bool PWOK = true;     // Password guessed

    // The following values are applicable to SPISettings/configureSPISettings()/getSPISettings()/getNVSPISettings()/writeNVSPISettings()
    static const quint32 BRT1K464 = 1464;    // Value corresponding to a bit rate of 1.464 Kib/s
    static const quint32 BRT1K5 = 1500;      // Value corresponding to a bit rate of 1.5 Kib/s
    static const quint32 BRT1K875 = 1875;    // Value corresponding to a bit rate of 1.875 Kib/s
    static const quint32 BRT2K5 = 2500;      // Value corresponding to a bit rate of 2.5 Kib/s
    static const quint32 BRT3K = 3000;       // Value corresponding to a bit rate of 3 Kib/s
    static const quint32 BRT3K125 = 3125;    // Value corresponding to a bit rate of 3.125 Kib/s
    static const quint32 BRT3K75 = 3750;     // Value corresponding to a bit rate of 3.75 Kib/s
    static const quint32 BRT5K = 5000;       // Value corresponding to a bit rate of 5 Kib/s
    static const quint32 BRT6K = 6000;       // Value corresponding to a bit rate of 6 Kib/s
    static const quint32 BRT6K25 = 6250;     // Value corresponding to a bit rate of 6.25 Kib/s
    static const quint32 BRT7K5 = 7500;      // Value corresponding to a bit rate of 7.5 Kib/s
    static const quint32 BRT9K375 = 9375;    // Value corresponding to a bit rate of 9.375 Kib/s
    static const quint32 BRT10K = 10000;     // Value corresponding to a bit rate of 10 Kib/s
    static const quint32 BRT12K = 12000;     // Value corresponding to a bit rate of 12 Kib/s
    static const quint32 BRT12K5 = 12500;    // Value corresponding to a bit rate of 12.5 Kib/s
    static const quint32 BRT15K = 15000;     // Value corresponding to a bit rate of 15 Kib/s
    static const quint32 BRT15K625 = 15625;  // Value corresponding to a bit rate of 15.625 Kib/s
    static const quint32 BRT18K75 = 18750;   // Value corresponding to a bit rate of 18.750 Kib/s
    static const quint32 BRT20K = 20000;     // Value corresponding to a bit rate of 20 Kib/s
    static const quint32 BRT24K = 24000;     // Value corresponding to a bit rate of 24 Kib/s
    static const quint32 BRT25K = 25000;     // Value corresponding to a bit rate of 25 Kib/s
    static const quint32 BRT30K = 30000;     // Value corresponding to a bit rate of 30 Kib/s
    static const quint32 BRT31K25 = 31250;   // Value corresponding to a bit rate of 31.25 Kib/s
    static const quint32 BRT37K5 = 37500;    // Value corresponding to a bit rate of 37.5 Kib/s
    static const quint32 BRT40K = 40000;     // Value corresponding to a bit rate of 40 Kib/s
    static const quint32 BRT46K875 = 46875;  // Value corresponding to a bit rate of 46.875 Kib/s
    static const quint32 BRT48K = 48000;     // Value corresponding to a bit rate of 48 Kib/s
    static const quint32 BRT50K = 50000;     // Value corresponding to a bit rate of 50 Kib/s
    static const quint32 BRT60K = 60000;     // Value corresponding to a bit rate of 60 Kib/s
    static const quint32 BRT62K5 = 62500;    // Value corresponding to a bit rate of 62.5 Kib/s
    static const quint32 BRT75K = 75000;     // Value corresponding to a bit rate of 75 Kib/s
    static const quint32 BRT80K = 80000;     // Value corresponding to a bit rate of 80 Kib/s
    static const quint32 BRT93K75 = 93750;   // Value corresponding to a bit rate of 93.75 Kib/s
    static const quint32 BRT100K = 100000;   // Value corresponding to a bit rate of 100 Kib/s
    static const quint32 BRT120K = 120000;   // Value corresponding to a bit rate of 120 Kib/s
    static const quint32 BRT125K = 125000;   // Value corresponding to a bit rate of 125 Kib/s
    static const quint32 BRT150K = 150000;   // Value corresponding to a bit rate of 150 Kib/s
    static const quint32 BRT187K5 = 187500;  // Value corresponding to a bit rate of 187.5 Kib/s
    static const quint32 BRT200K = 200000;   // Value corresponding to a bit rate of 200 Kib/s
    static const quint32 BRT240K = 240000;   // Value corresponding to a bit rate of 240 Kib/s
    static const quint32 BRT250K = 250000;   // Value corresponding to a bit rate of 250 Kib/s
    static const quint32 BRT300K = 300000;   // Value corresponding to a bit rate of 300 Kib/s
    static const quint32 BRT375K = 375000;   // Value corresponding to a bit rate of 375 Kib/s
    static const quint32 BRT400K = 400000;   // Value corresponding to a bit rate of 400 Kib/s
    static const quint32 BRT500K = 500000;   // Value corresponding to a bit rate of 500 Kib/s
    static const quint32 BRT600K = 600000;   // Value corresponding to a bit rate of 600 Kib/s
    static const quint32 BRT750K = 750000;   // Value corresponding to a bit rate of 750 Kib/s
    static const quint32 BRT1M = 1000000;    // Value corresponding to a bit rate of 1 Mib/s
    static const quint32 BRT1M2 = 1200000;   // Value corresponding to a bit rate of 1.2 Mib/s
    static const quint32 BRT1M5 = 1500000;   // Value corresponding to a bit rate of 1.5 Mib/s
    static const quint32 BRT2M = 2000000;    // Value corresponding to a bit rate of 2 Mib/s
    static const quint32 BRT3M = 3000000;    // Value corresponding to a bit rate of 3 Mib/s
    static const quint32 BRT12M = 12000000;  // Value corresponding to a bit rate of 12 Mib/s
    static const quint8 SPIMODE0 = 0x00;     // Value corresponding to SPI mode 0
    static const quint8 SPIMODE1 = 0x01;     // Value corresponding to SPI mode 1
    static const quint8 SPIMODE2 = 0x02;     // Value corresponding to SPI mode 2
    static const quint8 SPIMODE3 = 0x03;     // Value corresponding to SPI mode 3

    // The following values are useful as valid GPIO pin numbers
    static const int GPIO0 = 0;  // Pin number of GPIO0
    static const int GPIO1 = 1;  // Pin number of GPIO1
    static const int GPIO2 = 2;  // Pin number of GPIO2
    static const int GPIO3 = 3;  // Pin number of GPIO3
    static const int GPIO4 = 4;  // Pin number of GPIO4
    static const int GPIO5 = 5;  // Pin number of GPIO5
    static const int GPIO6 = 6;  // Pin number of GPIO6
    static const int GPIO7 = 7;  // Pin number of GPIO7
    static const int GPIO8 = 8;  // Pin number of GPIO8 (only valid for getGPIO(), since GPIO8 is an input only pin)

    // The following values are applicable to getGPIO() and setGPIO()
    static const bool PINLOW = false;
    static const bool PINHIGH = true;

    // The following values are applicable to getGPIODirection() and setGPIODirection()
    static const bool DIROUTPUT = false;
    static const bool DIRINPUT = true;

    // The following values are applicable to USBParameters/getUSBParameters()/writeUSBParameters()
    static const bool PMBUS = false;  // Value corresponding to USB bus-powered mode
    static const bool PMSELF = true;  // Value corresponding to USB self-powered mode

    struct ChipSettings {
        quint8 gp0;      // GP0 pin configuration
        quint8 gp1;      // GP1 pin configuration
        quint8 gp2;      // GP2 pin configuration
        quint8 gp3;      // GP3 pin configuration
        quint8 gp4;      // GP4 pin configuration
        quint8 gp5;      // GP5 pin configuration
        quint8 gp6;      // GP6 pin configuration
        quint8 gp7;      // GP7 pin configuration
        quint8 gp8;      // GP8 pin configuration
        quint8 gpdir;    // Default GPIO directions (CS7 to CS0)
        quint8 gpout;    // Default GPIO outputs (CS7 to CS0)
        bool rmwakeup;   // Remote wake-up
        quint8 intmode;  // Interrupt counting mode
        bool nrelspi;    // SPI bus release (negated)

        bool operator ==(const ChipSettings &other) const;
        bool operator !=(const ChipSettings &other) const;
    };

    struct ChipStatus {
        bool busreq;      // SPI bus release external request status (false for no request, true for pending request)
        quint8 busowner;  // SPI bus current owner
        quint8 pwtries;   // Number of NVRAM password tries
        bool pwok;        // Password validation status

        bool operator ==(const ChipStatus &other) const;
        bool operator !=(const ChipStatus &other) const;
    };

    struct SPISettings {
        quint16 nbytes;    // Number of bytes per SPI transaction
        quint32 bitrate;   // Bit rate
        quint8 mode;       // SPI mode (0, 1, 2 or 3)
        quint8 actcs;      // Active chip select value (CS7 to CS0)
        quint8 idlcs;      // Idle chip select value (CS7 to CS0)
        quint16 csdtdly;   // Chip select to data delay (100us units)
        quint16 dtcsdly;   // Data to chip select (de-asserted) delay (100us units)
        quint16 itbytdly;  // Inter-byte delay (100us units)

        bool operator ==(const SPISettings &other) const;
        bool operator !=(const SPISettings &other) const;
    };

    struct USBParameters {
        quint16 vid;    // Vendor ID
        quint16 pid;    // Product ID
        quint8 maxpow;  // Maximum consumption current (raw value in 2 mA units)
        bool powmode;   // Power mode (false for bus-powered, true for self-powered)
        bool rmwakeup;  // Remote wake-up capability

        bool operator ==(const USBParameters &other) const;
        bool operator !=(const USBParameters &other) const;
    };

    MCP2210();
    ~MCP2210();

    bool disconnected() const;
    bool isOpen() const;

    quint8 cancelSPITransfer(int &errcnt, QString &errstr);
    void close();
    quint8 configureChipSettings(const ChipSettings &settings, int &errcnt, QString &errstr);
    quint8 configureSPISettings(const SPISettings &settings, int &errcnt, QString &errstr);
    quint8 getAccessControlMode(int &errcnt, QString &errstr);
    ChipSettings getChipSettings(int &errcnt, QString &errstr);
    ChipStatus getChipStatus(int &errcnt, QString &errstr);
    quint16 getEventCount(int &errcnt, QString &errstr);
    bool getGPIO(int gpio, int &errcnt, QString &errstr);
    bool getGPIODirection(int gpio, int &errcnt, QString &errstr);
    quint8 getGPIODirections(int &errcnt, QString &errstr);
    quint16 getGPIOs(int &errcnt, QString &errstr);
    QString getManufacturerDesc(int &errcnt, QString &errstr);
    ChipSettings getNVChipSettings(int &errcnt, QString &errstr);
    SPISettings getNVSPISettings(int &errcnt, QString &errstr);
    QString getProductDesc(int &errcnt, QString &errstr);
    SPISettings getSPISettings(int &errcnt, QString &errstr);
    USBParameters getUSBParameters(int &errcnt, QString &errstr);
    QVector<quint8> hidTransfer(const QVector<quint8> &data, int &errcnt, QString &errstr);
    int open(quint16 vid, quint16 pid, const QString &serial = QString());
    quint8 readEEPROMByte(quint8 address, int &errcnt, QString &errstr);
    QVector<quint8> readEEPROMRange(quint8 begin, quint8 end, int &errcnt, QString &errstr);
    quint8 resetEventCounter(int &errcnt, QString &errstr);
    quint8 setGPIO(int gpio, bool value, int &errcnt, QString &errstr);
    quint8 setGPIODirection(int gpio, bool direction, int &errcnt, QString &errstr);
    quint8 setGPIODirections(quint8 directions, int &errcnt, QString &errstr);
    quint8 setGPIOs(quint16 values, int &errcnt, QString &errstr);
    QVector<quint8> spiTransfer(const QVector<quint8> &data, quint8 &status, int &errcnt, QString &errstr);
    quint8 toggleGPIO(int gpio, int &errcnt, QString &errstr);
    quint8 usePassword(const QString &password, int &errcnt, QString &errstr);
    quint8 writeEEPROMByte(quint8 address, quint8 value, int &errcnt, QString &errstr);
    quint8 writeEEPROMRange(quint8 begin, quint8 end, const QVector<quint8> &values, int &errcnt, QString &errstr);
    quint8 writeManufacturerDesc(const QString &manufacturer, int &errcnt, QString &errstr);
    quint8 writeNVChipSettings(const ChipSettings &settings, quint8 accessControlMode, const QString &password, int &errcnt, QString &errstr);
    quint8 writeNVChipSettings(const ChipSettings &settings, int &errcnt, QString &errstr);
    quint8 writeNVSPISettings(const SPISettings &settings, int &errcnt, QString &errstr);
    quint8 writeProductDesc(const QString &product, int &errcnt, QString &errstr);
    quint8 writeUSBParameters(const USBParameters &parameters, int &errcnt, QString &errstr);

    static QStringList listDevices(quint16 vid, quint16 pid, int &errcnt, QString &errstr);
};

#endif  // MCP2210_H
