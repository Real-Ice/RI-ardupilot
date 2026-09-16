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
 * I2C driver for Sensirion SHT4x digital temperature/humidity sensor family
 * (SHT40/SHT41/SHT43/SHT45), e.g. as used on an SHT45 wired to AP_Periph.

 https://sensirion.com/media/documents/33FD6CH5/6866F818/HT_DS_Datasheet_SHT4x.pdf

 Unlike SHT3x, SHT4x uses single-byte commands and has no clock-stretch or
 periodic-measurement modes - each reading is a one-shot "start measurement"
 command followed later by a plain read of the 6-byte result. The response
 frame layout and CRC-8 are otherwise identical to SHT3x (handled by the
 shared AP_TemperatureSensor_Sensirion base), but the humidity conversion
 formula differs: RH = -6 + 125 * raw/65535 (vs SHT3x's RH = 100 * raw/65535).

# when plugging into I2C1 on a Mateksys CAN G474:
param set TEMP1_TYPE 10
param set TEMP1_BUS 0
param set TEMP1_ADDR 0x44

*/

#pragma once

#include "AP_TemperatureSensor_config.h"

#if AP_TEMPERATURE_SENSOR_SHT4X_ENABLED

#include "AP_TemperatureSensor_Sensirion.h"

class AP_TemperatureSensor_SHT4x : public AP_TemperatureSensor_Sensirion {

    using AP_TemperatureSensor_Sensirion::AP_TemperatureSensor_Sensirion;

protected:
    const char *name(void) const override { return "SHT4x"; }

    bool send_reset_cmd(void) const override;
    bool read_serial_number(uint8_t sn[6]) const override;
    void start_next_sample() override;

    // RH = -6 + 125 * raw/65535, cropped to the physical 0-100 %RH range
    float convert_humidity(uint16_t raw) const override;

    // datasheet tSR max is 1ms (vs SHT3x's 4ms)
    uint8_t reset_delay_ms(void) const override { return 1; }
};
#endif // AP_TEMPERATURE_SENSOR_SHT4X_ENABLED
