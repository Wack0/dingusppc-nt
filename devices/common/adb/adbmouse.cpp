/*
DingusPPC - The Experimental PowerPC Macintosh emulator
Copyright (C) 2018-23 divingkatae and maximum
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

/** @file Apple Desktop Bus Mouse emulation. */

#include <devices/common/adb/adbmouse.h>
#include <devices/deviceregistry.h>

AdbMouse::AdbMouse(std::string name) : AdbDevice(name) {
    EventManager::get_instance()->add_mouse_handler(this, &AdbMouse::event_handler);

    this->reset();
}

void AdbMouse::event_handler(const MouseEvent& event) {
    if (event.flags & MOUSE_EVENT_MOTION) {
        this->x_rel += event.xrel;
        this->y_rel += event.yrel;
    }
}

void AdbMouse::reset() {
    this->my_addr = ADB_ADDR_RELPOS;
    this->dev_handler_id = 1;
    this->exc_event_flag = 1;
    this->srq_flag = 1; // enable service requests
}

bool AdbMouse::get_register_0() {
    if (this->x_rel || this->y_rel) {
        uint8_t* out_buf = this->host_obj->get_output_buf();

        // report Y-axis motion
        if (this->y_rel < -64)
            out_buf[0] = 0x40 | 0x80;
        else if (this->y_rel > 63)
            out_buf[0] = 0x3F | 0x80;
        else
            out_buf[0] = (this->y_rel & 0x7F) | 0x80;

        // report X-axis motion
        if (this->x_rel < -64)
            out_buf[1] = 0x40 | 0x80;
        else if (this->x_rel > 63)
            out_buf[1] = 0x3F | 0x80;
        else
            out_buf[1] = (this->x_rel & 0x7F) | 0x80;

        // reset accumulated motion data
        this->x_rel = 0;
        this->y_rel = 0;

        this->host_obj->set_output_count(2);
        return true;
    }

    return false;
}

void AdbMouse::set_register_3() {
    if (this->host_obj->get_input_count() < 2) // ensure we got enough data
        return;

    const uint8_t*  in_data = this->host_obj->get_input_buf();

    switch (in_data[1]) {
    case 0:
        this->my_addr  = in_data[0] & 0xF;
        this->srq_flag = !!(in_data[0] & 0x20);
        break;
    case 1:
    case 2:
        this->dev_handler_id = in_data[1];
        break;
    case 4: // extended mouse protocol isn't supported yet
        break;
    case 0xFE: // move to a new address if there was no collision
        if (!this->got_collision) {
            this->my_addr = in_data[0] & 0xF;
        }
        break;
    default:
        LOG_F(WARNING, "%s: unknown handler ID = 0x%X", this->name.c_str(), in_data[1]);
    }
}

static const DeviceDescription AdbMouse_Descriptor = {
    AdbMouse::create, {}, {}
};

REGISTER_DEVICE(AdbMouse, AdbMouse_Descriptor);
