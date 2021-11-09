/*
DingusPPC - The Experimental PowerPC Macintosh emulator
Copyright (C) 2018-21 divingkatae and maximum
                      (theweirdo)     spatium

(Contact divingkatae#1017 or powermax#2286 on Discord for more info)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file DisplayID class implementation. */

#include <devices/video/displayid.h>
#include <loguru.hpp>

DisplayID::DisplayID(Disp_Id_Kind id_kind)
{
    this->id_kind = id_kind;

    /* Initialize Apple monitor codes */
    this->std_sense_code = 6;
    this->ext_sense_code = 0x2B;

    /* Initialize DDC I2C bus */
    this->next_state = I2CState::STOP;
    this->prev_state = I2CState::STOP;
    this->last_sda   = 1;
    this->last_scl   = 1;
    this->data_out   = 0x3000;
    this->data_ptr   = 0;
}


uint16_t DisplayID::set_result(uint8_t sda, uint8_t scl)
{
    this->last_sda = sda;
    this->last_scl = scl;

    if (scl) {
        this->data_out |= 0x1000;
    } else {
        this->data_out &= ~0x1000U;
    }

    if (sda) {
        this->data_out |= 0x2000;
    } else {
        this->data_out &= ~0x2000U;
    }

    return this->data_out;
}


uint16_t DisplayID::read_monitor_sense(uint16_t data, uint16_t dirs)
{
    uint8_t scl, sda;
    uint16_t result;

    if ((dirs & 0x3100) == 0 && (data & 0x3100) == 0x3100) {
        LOG_F(WARNING, "DisplayID: Monitor sense lines tristated!");
    }

    switch(this->id_kind) {
    case Disp_Id_Kind::DDC2B:
        /* if GPIO pins are in the output mode, pick up their values
           In the input mode, GPIO pins will be read "high" */
        scl = (dirs & 0x1000) ? !!(data & 0x1000) : 1;
        sda = (dirs & 0x2000) ? !!(data & 0x2000) : 1;

        return update_ddc_i2c(sda, scl);
    case Disp_Id_Kind::AppleSense:
        switch (dirs & 0x3100) {
        case 0:
            result = ((this->std_sense_code & 6) << 11) | ((this->std_sense_code & 1) << 8);
            break;
        case 0x2000: /* Sense line 2 is low */
            result = ((this->ext_sense_code & 0x20) << 7) | ((this->ext_sense_code & 0x10) << 4);
            break;
        case 0x1000: /* Sense line 1 is low */
            result = ((this->ext_sense_code & 8) << 10) | ((this->ext_sense_code & 4) << 6);
            break;
        case 0x100: /* Sense line 0 is low */
            result = ((this->ext_sense_code & 2) << 12) | ((this->ext_sense_code & 1) << 12);
            break;
        default:
            result = 0x3100U;
        }
        return result;
    }
}


uint16_t DisplayID::update_ddc_i2c(uint8_t sda, uint8_t scl)
{
    bool clk_gone_high = false;

    if (scl != this->last_scl) {
        this->last_scl = scl;
        if (scl) {
            clk_gone_high = true;
        }
    }

    if (sda != this->last_sda) {
        /* START = SDA goes high to low while SCL is high */
        /* STOP  = SDA goes low to high while SCL is high */
        if (this->last_scl) {
            if (!sda) {
                LOG_F(9, "DDC-I2C: START condition detected!");
                this->next_state = I2CState::DEV_ADDR;
                this->bit_count  = 0;
            } else {
                LOG_F(9, "DDC-I2C: STOP condition detected!");
                this->next_state = I2CState::STOP;
            }
        }
        return set_result(sda, scl);
    }

    if (!clk_gone_high) {
        return set_result(sda, scl);
    }

    switch (this->next_state) {
    case I2CState::STOP:
        break;

    case I2CState::ACK:
        this->bit_count = 0;
        this->byte      = 0;
        switch (this->prev_state) {
        case I2CState::DEV_ADDR:
            if ((dev_addr & 0xFE) == 0xA0) {
                sda = 0; /* send ACK */
            } else {
                LOG_F(ERROR, "DDC-I2C: unknown device address 0x%X", this->dev_addr);
                sda = 1; /* send NACK */
            }
            if (this->dev_addr & 1) {
                this->next_state = I2CState::DATA;
                this->data_ptr   = this->edid;
                this->byte       = *(this->data_ptr++);
            } else {
                this->next_state = I2CState::REG_ADDR;
            }
            break;
        case I2CState::REG_ADDR:
            this->next_state = I2CState::DATA;
            if (!this->reg_addr) {
                sda = 0; /* send ACK */
            } else {
                LOG_F(ERROR, "DDC-I2C: unknown register address 0x%X", this->reg_addr);
                sda = 1; /* send NACK */
            }
            break;
        case I2CState::DATA:
            this->next_state = I2CState::DATA;
            if (dev_addr & 1) {
                if (!sda) {
                    /* load next data byte */
                    this->byte = *(this->data_ptr++);
                } else {
                    LOG_F(ERROR, "DDC-I2C: Oops! NACK received");
                }
            } else {
                sda = 0; /* send ACK */
            }
            break;
        }
        break;

    case I2CState::DEV_ADDR:
    case I2CState::REG_ADDR:
        this->byte = (this->byte << 1) | this->last_sda;
        if (this->bit_count++ >= 7) {
            this->bit_count  = 0;
            this->prev_state = this->next_state;
            this->next_state = I2CState::ACK;
            if (this->prev_state == I2CState::DEV_ADDR) {
                LOG_F(9, "DDC-I2C: device address received, addr=0x%X", this->byte);
                this->dev_addr = this->byte;
            } else {
                LOG_F(9, "DDC-I2C: register address received, addr=0x%X", this->byte);
                this->reg_addr = this->byte;
            }
        }
        break;

    case I2CState::DATA:
        sda = (this->byte >> (7 - this->bit_count)) & 1;
        if (this->bit_count++ >= 7) {
            this->bit_count  = 0;
            this->prev_state = this->next_state;
            this->next_state = I2CState::ACK;
        }
        break;
    }

    return set_result(sda, scl);
}
