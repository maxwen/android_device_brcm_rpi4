/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define LOG_TAG "rpi4 PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#include "utils.h"

#define UNUSED_ARGUMENT __attribute((unused))

// in seconds
#define PERFORMANCE_BOOT_TIMEOUT 2

static pthread_t tid = -1;
static char min_freq[1024];
static char max_freq[1024];
static int DEBUG = 0;
static void *performance_boost_thread(UNUSED_ARGUMENT void *data)
{
    get_scaling_max_freq(max_freq, 1024);
    get_scaling_min_freq(min_freq, 1024);

    if (!strcmp(min_freq, max_freq)) {
        tid = -1;
        return NULL;
    }
    if (DEBUG) ALOGD("performnace_boost start %s -> %s", min_freq, max_freq);

    sysfs_write(SCALING_MIN_FREQ, max_freq);
    sleep(PERFORMANCE_BOOT_TIMEOUT);
    sysfs_write(SCALING_MIN_FREQ, min_freq);

    if (DEBUG) ALOGD("performnace_boost end");

    tid = -1;
    return NULL;
}

static int start_performance_boost()
{
    if (tid != -1) {
        return 0;
    }

    return pthread_create(&tid, NULL, performance_boost_thread, NULL);
}

static void power_init(struct power_module *module UNUSED_ARGUMENT)
{
}

static void power_set_interactive(struct power_module *module UNUSED_ARGUMENT,
                  int on UNUSED_ARGUMENT)
{
}

static void power_hint(struct power_module *module UNUSED_ARGUMENT,
               power_hint_t hint,
               void *data UNUSED_ARGUMENT) {
    switch (hint) {
    case POWER_HINT_INTERACTION:
        //ALOGD("POWER_HINT_INTERACTION");
        break;
    case POWER_HINT_LAUNCH:
        if (tid != -1) {
            break;
        }
        if (DEBUG) ALOGD("POWER_HINT_LAUNCH");
        start_performance_boost();
        break;
    default:
        break;
    }
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = POWER_MODULE_API_VERSION_0_2,
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = POWER_HARDWARE_MODULE_ID,
    .name = "rpi4 Power HAL",
    .author = "The Android Open Source Project",
    .methods = &power_module_methods,
    },

    .init = power_init,
    .setInteractive = power_set_interactive,
    .powerHint = power_hint,
};
