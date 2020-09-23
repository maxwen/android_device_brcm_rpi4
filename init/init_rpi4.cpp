/*
* Copyright (C) 2014-2020 The OmniROM Project
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

#include "uevent.h"

#include <sys/resource.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

namespace android {
namespace init {

using android::base::GetProperty;


template <typename T>
static T get(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;

    file >> result;
    return file.fail() ? def : result;
}

void property_override(const std::string& name, const std::string& value)
{
    size_t valuelen = value.size();

    prop_info* pi = (prop_info*) __system_property_find(name.c_str());
    if (pi != nullptr) {
        __system_property_update(pi, value.c_str(), valuelen);
    }
    else {
        int rc = __system_property_add(name.c_str(), name.size(), value.c_str(), valuelen);
        if (rc < 0) {
            LOG(ERROR) << "property_set(\"" << name << "\", \"" << value << "\") failed: "
                       << "__system_property_add failed";
        }
    }
}

void set_revision_property() {
    std::string cpuinfo;
    android::base::ReadFileToString("/proc/cpuinfo", &cpuinfo);

    std::istringstream in(cpuinfo);
    std::string line;
    while (std::getline(in, line)) {
        std::vector<std::string> pieces = android::base::Split(line, ":");
        if (pieces.size() == 2) {
            if (android::base::Trim(pieces[0]) == "Revision") {
                property_override("ro.revision", android::base::Trim(pieces[1]));
                return;
            }
        }
    }
}

void vendor_load_properties()
{
    // is done from kernel now
    //std::string serial = get("/proc/device-tree/serial-number", std::string(""));
    //property_override("ro.serialno", serial);
    
    set_revision_property();
}

void vendor_create_device_symlinks(const Uevent& uevent, std::vector<std::string>& links)
{
    LOG(INFO) << "vendor_create_device_symlinks: device " << uevent.device_name;

    // TODO raspi hardcode start
    int num = uevent.partition_num;
    if (num == 2 || uevent.device_name.find("sda2") != std::string::npos
                || uevent.device_name.find("mmcblk0p2") != std::string::npos) {
        links.emplace_back("/dev/block/by-name/system");
    } else if (num == 3 || uevent.device_name.find("sda3") != std::string::npos
                || uevent.device_name.find("mmcblk0p3") != std::string::npos) {
        links.emplace_back("/dev/block/by-name/vendor");
    } else if (num == 4 || uevent.device_name.find("sda4") != std::string::npos
                || uevent.device_name.find("mmcblk0p4") != std::string::npos) {
        links.emplace_back("/dev/block/by-name/userdata");
    }
}
}
}
