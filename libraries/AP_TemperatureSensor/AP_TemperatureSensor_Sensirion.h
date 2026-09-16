/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Shared I2C driver base for Sensirion SHTx temperature/humidity sensors
 * (SHT3x, SHT4x). Both families reply to a measurement request with the same
 * 6-byte frame (2 temperature bytes + CRC-8, 2 humidity bytes + CRC-8, same
 * CRC-8 polynomial), and share the same temperature conversion formula, but
 * differ in their command bytes/timing and humidity conversion formula -
 * those differences are provided by subclasses.
 */

#pragma once

#include "AP_TemperatureSensor_config.h"

#if AP_TEMPERATURE_SENSOR_SHT3X_ENABLED || AP_TEMPERATURE_SENSOR_SHT4X_ENABLED

#include "AP_TemperatureSensor_Backend.h"
#include <AP_Math/definitions.h>

class AP_TemperatureSensor_Sensirion : public AP_TemperatureSensor_Backend {

    using AP_TemperatureSensor_Backend::AP_TemperatureSensor_Backend;

public:
    __INITFUNC__ void init(void) override;

    void update() override {};

    bool has_humidity(void) const override { return true; }

protected:
    // human readable sensor name, used in GCS messages
    virtual const char *name(void) const = 0;

    // send the sensor-specific soft reset command
    virtual bool send_reset_cmd(void) const = 0;

    // send the sensor-specific serial number read command
    virtual bool read_serial_number(uint8_t sn[6]) const = 0;

    // send the sensor-specific command to start the next measurement
    virtual void start_next_sample() = 0;

    // convert a raw 16 bit humidity reading into %RH; formula differs between sensor families
    virtual float convert_humidity(uint16_t raw) const = 0;

    // time to wait, in ms, after a soft reset before the sensor is ready again
    virtual uint8_t reset_delay_ms(void) const { return 4; }

    // periodic sample interval, in microseconds
    virtual uint32_t sample_interval_us(void) const { return 50 * AP_USEC_PER_MSEC; }

private:
    // read measurements from device:
    bool read_measurements(uint16_t &temp, uint16_t &humidity) const;

    // update the temperature/humidity, called periodically
    void _timer(void);
};
#endif // AP_TEMPERATURE_SENSOR_SHT3X_ENABLED || AP_TEMPERATURE_SENSOR_SHT4X_ENABLED
