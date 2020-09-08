/*
   Copyright (c) 2015, The Linux Foundation. All rights reserved.
   Copyright (C) 2019 The OmniRom Project.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#include "property_service.h"
#include "uevent.h"

#include <sys/resource.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

namespace android {
namespace init {

using android::base::GetProperty;
using android::init::property_set;


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
