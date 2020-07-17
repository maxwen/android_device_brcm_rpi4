/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.provision;

import android.app.Activity;
import android.content.ComponentName;
import android.content.pm.PackageManager;
import android.os.BatteryManager;
import android.os.Bundle;
import android.os.UserHandle;
import android.provider.Settings;
import android.util.Log;

import com.android.internal.widget.LockPatternUtils;

/**
 * Application that sets the provisioned bit, like SetupWizard does.
 */
public class DefaultActivity extends Activity {

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        Log.d("Provision", "onCreate");
        // Add a persistent setting to allow other apps to know the device has been provisioned.
        Settings.Global.putInt(getContentResolver(), Settings.Global.DEVICE_PROVISIONED, 1);
        Settings.Secure.putInt(getContentResolver(), Settings.Secure.USER_SETUP_COMPLETE, 1);

        // set useful defaults for raspi
        Settings.Secure.putInt(getContentResolver(), "qs_show_brightness", 0);
        Settings.Secure.putInt(getContentResolver(), "sysui_nav_bar_system_keys", 1);
        Settings.Secure.putString(getContentResolver(), "icon_blacklist", "rotate,headset,battery");
        // enable always on -> TODO assumes fake charging is set
        Settings.Global.putInt(getContentResolver(), Settings.Global.STAY_ON_WHILE_PLUGGED_IN,
                BatteryManager.BATTERY_PLUGGED_AC | BatteryManager.BATTERY_PLUGGED_USB
                | BatteryManager.BATTERY_PLUGGED_WIRELESS);

        // disable screen lock -> TODO on first boot it still comes up with initial swipe
        LockPatternUtils lockPatternUtils = new LockPatternUtils(getApplicationContext());
        lockPatternUtils.setLockScreenDisabled(true, UserHandle.myUserId());

        PackageManager pm = getPackageManager();
        ComponentName name = new ComponentName(this, DefaultActivity.class);
        pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                PackageManager.DONT_KILL_APP);

        // terminate the activity.
        finish();
    }
}

