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
package org.omnirom.device;

import android.content.Context;
import android.content.res.Resources;
import android.content.Intent;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import androidx.preference.PreferenceFragment;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.provider.Settings;
import android.view.Display;
import android.view.View;
import android.view.IWindowManager;
import android.view.Surface;
import android.util.Log;

import com.android.internal.view.RotationPolicy;

import java.util.ArrayList;
import java.util.List;

public class DeviceSettings extends PreferenceFragment implements
        Preference.OnPreferenceChangeListener {
    private static final String TAG = "DeviceSettings";
    private static final String KEY_ROTATION_LOCK = "rotation_lock";
    private static final String KEY_BOOT_MODE = "boot_mode";
    private static final String KEY_AUDIO_CARD = "audio_card";
    private static final String KEY_CPU_GOVERNOR = "cpu_governor";
    private static final String KEY_CPU_MAX_FREQ = "cpu_max_Freq";

    private static final String BOOT_MODE_PROPERTY = "sys.rpi4.boot_mode";
    private static final String AUDIO_CARD_PROPERTY = "audio.pcm.card";
    private static final String AUDIO_CARD_OVERRIDE_PROPERTY = "persist.audio.pcm.card";
    private static final String CPU_GOVERNOR_PROPERTY = "persist.rpi4.cpufreq.governor";
    private static final String CPU_MAX_FREQ_PROPERTY = "persist.rpi4.cpufreq.max_freq";

    private static final String CPU_SYSFS_GOVERNORS = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors";
    private static final String CPU_SYSFS_GOVERNOR = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    private static final String CPU_SYSFS_FREQUENCIES = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies";
    private static final String CPU_SYSFS_MAX_FREQ = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq";

    private IWindowManager mWindowManager;
    private ListPreference mRotationLock;
    private ListPreference mBootMode;
    private ListPreference mAudioCard;
    private ListPreference mCPUGovernor;
    private ListPreference mCPUMaxFreq;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.main, rootKey);
        mWindowManager = IWindowManager.Stub.asInterface(
                ServiceManager.getService(Context.WINDOW_SERVICE));
        mRotationLock = (ListPreference) findPreference(KEY_ROTATION_LOCK);
        mRotationLock.setOnPreferenceChangeListener(this);
        mRotationLock.setSummary(mRotationLock.getEntry());

        mBootMode = (ListPreference) findPreference(KEY_BOOT_MODE);
        mBootMode.setOnPreferenceChangeListener(this);
        String bootMode = getBootMode();
        mBootMode.setValue(bootMode);
        mBootMode.setSummary(mBootMode.getEntry());

        mAudioCard = (ListPreference) findPreference(KEY_AUDIO_CARD);
        mAudioCard.setOnPreferenceChangeListener(this);
        String card = SystemProperties.get(AUDIO_CARD_OVERRIDE_PROPERTY,
                SystemProperties.get(AUDIO_CARD_PROPERTY, ""));
        mAudioCard.setValue(card);
        mAudioCard.setSummary(mAudioCard.getEntry());

        mCPUGovernor = (ListPreference) findPreference(KEY_CPU_GOVERNOR);
        mCPUGovernor.setOnPreferenceChangeListener(this);
        setupGovernors();
        String governor = Utils.readLine(CPU_SYSFS_GOVERNOR);
        if (governor != null) {
            mCPUGovernor.setValue(governor);
            mCPUGovernor.setSummary(mCPUGovernor.getEntry());
        }

        mCPUMaxFreq = (ListPreference) findPreference(KEY_CPU_MAX_FREQ);
        mCPUMaxFreq.setOnPreferenceChangeListener(this);
        setupMaxFreq();
        String maxFreq = Utils.readLine(CPU_SYSFS_MAX_FREQ);
        if (maxFreq != null) {
            mCPUMaxFreq.setValue(maxFreq);
            mCPUMaxFreq.setSummary(mCPUMaxFreq.getEntry());
        }
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        return super.onPreferenceTreeClick(preference);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference == mRotationLock) {
            String value = (String) newValue;
            int rotationLockValue = Integer.valueOf(value);
            try {
                if (rotationLockValue == 0) {
                    mWindowManager.thawRotation();
                    mWindowManager.setFixedToUserRotation(Display.DEFAULT_DISPLAY, 0);
                    Settings.System.putInt(getContext().getContentResolver(),
                            Settings.System.ACCELEROMETER_ROTATION, 0);
                } else if (rotationLockValue == 1) {
                    mWindowManager.freezeRotation(Surface.ROTATION_0);
                    mWindowManager.setFixedToUserRotation(Display.DEFAULT_DISPLAY, 2);
                    Settings.System.putInt(getContext().getContentResolver(),
                            Settings.System.ACCELEROMETER_ROTATION, 1);
                } else if (rotationLockValue == 2) {
                    mWindowManager.freezeRotation(Surface.ROTATION_270);
                    mWindowManager.setFixedToUserRotation(Display.DEFAULT_DISPLAY, 2);
                    Settings.System.putInt(getContext().getContentResolver(),
                            Settings.System.ACCELEROMETER_ROTATION, 1);
                } else if (rotationLockValue == 3) {
                    mWindowManager.freezeRotation(Surface.ROTATION_90);
                    mWindowManager.setFixedToUserRotation(Display.DEFAULT_DISPLAY, 2);
                    Settings.System.putInt(getContext().getContentResolver(),
                            Settings.System.ACCELEROMETER_ROTATION, 1);
                } else if (rotationLockValue == 4) {
                    mWindowManager.freezeRotation(Surface.ROTATION_180);
                    mWindowManager.setFixedToUserRotation(Display.DEFAULT_DISPLAY, 2);
                    Settings.System.putInt(getContext().getContentResolver(),
                            Settings.System.ACCELEROMETER_ROTATION, 1);
                }
                mRotationLock.setSummary(mRotationLock.getEntries()[rotationLockValue]);
            } catch (RemoteException e){
                Log.e(TAG, "freezeRotation", e);
            }
        } else if (preference == mAudioCard) {
            String value = (String) newValue;
            SystemProperties.set(AUDIO_CARD_OVERRIDE_PROPERTY, value);
            mAudioCard.setSummary(mAudioCard.getEntries()[Integer.valueOf(value)]);
        } else if (preference == mCPUGovernor) {
            String value = (String) newValue;
            SystemProperties.set(CPU_GOVERNOR_PROPERTY, value);
            mCPUGovernor.setSummary(value);
        } else if (preference == mCPUMaxFreq) {
            String value = (String) newValue;
            SystemProperties.set(CPU_MAX_FREQ_PROPERTY, value);
            mCPUMaxFreq.setSummary(value);
        } else if (preference == mBootMode) {
            String value = (String) newValue;
            switchBootMode(value);
            mBootMode.setSummary(mBootMode.getEntries()[mBootMode.findIndexOfValue(value)]);
        }
        return true;
    }

    private void switchBootMode(String mode) {
        SystemProperties.set(BOOT_MODE_PROPERTY, mode);
    }

    private String getBootMode() {
        return SystemProperties.get(BOOT_MODE_PROPERTY);
    }

    private void setupGovernors() {
        List<String> governors = Utils.readLineAsArray(CPU_SYSFS_GOVERNORS);
        if (governors != null) {
            String[] g = governors.toArray(new String[governors.size()]);
            mCPUGovernor.setEntryValues(g);
            mCPUGovernor.setEntries(g);
        } else {
            mCPUGovernor.setEnabled(false);
        }
    }

    private void setupMaxFreq() {
        List<String> frequencies = Utils.readLineAsArray(CPU_SYSFS_FREQUENCIES);
        if (frequencies != null) {
            String[] g = frequencies.toArray(new String[frequencies.size()]);
            mCPUMaxFreq.setEntryValues(g);
            mCPUMaxFreq.setEntries(g);
        } else {
            mCPUMaxFreq.setEnabled(false);
        }
    }
}
