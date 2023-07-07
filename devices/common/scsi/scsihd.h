/*
DingusPPC - The Experimental PowerPC Macintosh emulator
Copyright (C) 2018-22 divingkatae and maximum
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

/** @file SCSI hard drive definitions. */

#ifndef SCSI_HD_H
#define SCSI_HD_H

#include <devices/common/scsi/scsi.h>

#include <cinttypes>
#include <fstream>
#include <memory>
#include <stdio.h>
#include <string>

class ScsiHardDisk : public ScsiDevice {
public:
    ScsiHardDisk(int my_id);
    ~ScsiHardDisk() = default;

    static std::unique_ptr<HWComponent> create() {
        return std::unique_ptr<ScsiHardDisk>(new ScsiHardDisk(0));
    }

    void insert_image(std::string filename);
    void process_command();
    bool prepare_data();

protected:
    int test_unit_ready();
    int req_sense(uint16_t alloc_len);
    int send_diagnostic();
    int mode_select_6(uint8_t param_len);

    void mode_sense_6(uint8_t page_code, uint8_t subpage_code, uint8_t alloc_len);
    void format();
    void inquiry(uint16_t alloc_len);
    void read_capacity_10();
    void read(uint32_t lba, uint16_t transfer_len, uint8_t cmd_len);
    void write(uint32_t lba, uint16_t transfer_len, uint8_t cmd_len);
    void seek(uint32_t lba);
    void rewind();

private:
    std::fstream    hdd_img;
    uint64_t        img_size;
    int             total_blocks;
    uint64_t        file_offset = 0;

    char            img_buffer[1 << 21]; // TODO: add proper buffer management!

    uint32_t    cur_buf_cnt = 0;
    uint8_t     error = ScsiError::NO_ERROR;
    uint8_t     msg_code = 0;

    //inquiry info
    char vendor_info[8] = {'D', 'i', 'n', 'g', 'u', 's', 'D', '\0'};
    char prod_info[16]  = {'E', 'm', 'u', 'l', 'a', 't', 'e', 'd', ' ', 'D', 'i', 's', 'k', '\0'};
    char rev_info[8]    = {'d', 'i', '0', '0', '0', '0', '0', '1'};
    char serial_info[8] = {'0', '0', '0', '0', '0', '0', '0', '0'};
};

#endif // SCSI_HD_H