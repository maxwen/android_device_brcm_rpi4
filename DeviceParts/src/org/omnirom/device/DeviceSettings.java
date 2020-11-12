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

public class DeviceSettings extends PreferenceFragment implements
        Preference.OnPreferenceChangeListener {
    private static final String TAG = "DeviceSettings";
    private static final String KEY_ROTATION_LOCK = "rotation_lock";
    private static final String KEY_BOOT_MODE = "boot_mode";
    private static final String KEY_AUDIO_CARD = "audio_card";

    private static final String BOOT_MODE_PROPERTY = "sys.rpi4.boot_mode";
    private static final String AUDIO_CARD_PROPERTY = "audio.pcm.card";
    private static final String AUDIO_CARD_OVERRIDE_PROPERTY = "persist.audio.pcm.card";

    private IWindowManager mWindowManager;
    private ListPreference mRotationLock;
    private ListPreference mBootMode;
    private ListPreference mAudioCard;

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
}
