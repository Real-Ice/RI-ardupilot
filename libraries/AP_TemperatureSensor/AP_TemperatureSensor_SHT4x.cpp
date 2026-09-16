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

#include "AP_TemperatureSensor_SHT4x.h"

#if AP_TEMPERATURE_SENSOR_SHT4X_ENABLED
#include <AP_HAL/I2CDevice.h>
#include <AP_Math/AP_Math.h>

bool AP_TemperatureSensor_SHT4x::send_reset_cmd(void) const
{
    static const uint8_t soft_reset_cmd[1] { 0x94 };
    return _dev->transfer(soft_reset_cmd, ARRAY_SIZE(soft_reset_cmd), nullptr, 0);
}

bool AP_TemperatureSensor_SHT4x::read_serial_number(uint8_t sn[6]) const
{
    static const uint8_t read_sn_cmd[1] { 0x89 };
    return _dev->transfer(read_sn_cmd, ARRAY_SIZE(read_sn_cmd), sn, 6);
}

void AP_TemperatureSensor_SHT4x::start_next_sample()
{
    // measure T & RH with high repeatability (max 8.3ms conversion time)
    static const uint8_t start_measurement_command[1] { 0xFD };
    _dev->transfer(start_measurement_command, ARRAY_SIZE(start_measurement_command), nullptr, 0);
}

float AP_TemperatureSensor_SHT4x::convert_humidity(uint16_t raw) const
{
    const float rh = -6 + 125 * (raw / 65535.0);
    return constrain_float(rh, 0.0f, 100.0f);
}

#endif // AP_TEMPERATURE_SENSOR_SHT4X_ENABLED
