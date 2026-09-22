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

Written with reference to the PX4 driver written by Roman Dvorak <dvorakroman@thunderfly.cz>

*/

#include "AP_TemperatureSensor_Sensirion.h"

#if AP_TEMPERATURE_SENSOR_SHT3X_ENABLED || AP_TEMPERATURE_SENSOR_SHT4X_ENABLED
#include <stdio.h>
#include <AP_HAL/I2CDevice.h>
#include <AP_Math/AP_Math.h>

#include <GCS_MAVLink/GCS.h>

extern const AP_HAL::HAL &hal;

void AP_TemperatureSensor_Sensirion::init()
{
    _dev = hal.i2c_mgr->get_device_ptr(_params.bus, _params.bus_address);
    if (!_dev) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "%s device is null!", name());
        printf("%s device is null!\n", name());
        return;
    }

    WITH_SEMAPHORE(_dev->get_semaphore());

    _dev->set_retries(10);

    // read serial number, mostly as confirmation we have the sensor:
    uint8_t sn[6];
    if (!read_serial_number(sn)) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "%s read sn failed", name());
        printf("%s read sn failed\n", name());
        return;
    }
    GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "%s: SN%x%x%x%x%x%x", name(), sn[0], sn[1], sn[2], sn[3], sn[4], sn[5]);
    printf("%s: SN%x%x%x%x%x%x\n", name(), sn[0], sn[1], sn[2], sn[3], sn[4], sn[5]);

    // reset
    if (!send_reset_cmd()) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "%s reset failed", name());
        printf("%s reset failed\n", name());
        return;
    }

    hal.scheduler->delay(reset_delay_ms());

    start_next_sample();

    // lower retries for run
    _dev->set_retries(3);

    printf("%s init OK\n", name());

    _dev->register_periodic_callback(sample_interval_us(),
                                     FUNCTOR_BIND_MEMBER(&AP_TemperatureSensor_Sensirion::_timer, void));
}

bool AP_TemperatureSensor_Sensirion::read_measurements(uint16_t &temp, uint16_t &humidity) const
{
    uint8_t val[6];
    if (!_dev->transfer(nullptr, 1, val, ARRAY_SIZE(val))) {
        return false;
    }

    if (val[2] != crc8_generic(&val[0], 2, 0x31, 0xff)) {
        // temperature CRC is incorrect
        return false;
    }

    if (val[5] != crc8_generic(&val[3], 2, 0x31, 0xff)) {
        // humidity CRC is incorrect
        return false;
    }

    temp = val[0] << 8 | val[1];
    humidity = val[3] << 8 | val[4];
    return true;
}

void AP_TemperatureSensor_Sensirion::_timer(void)
{
    uint16_t encoded_temp;
    uint16_t encoded_humidity;
    if (read_measurements(encoded_temp, encoded_humidity)) {
        const float temp = -45 + 175 * (encoded_temp/65535.0);
        set_temperature(temp);
        set_humidity(convert_humidity(encoded_humidity));
    }

    start_next_sample();
}

#endif // AP_TEMPERATURE_SENSOR_SHT3X_ENABLED || AP_TEMPERATURE_SENSOR_SHT4X_ENABLED
