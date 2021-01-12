Builds as 64bit only so make sure you have propper firmware!

# sync omni source

Refer to http://source.android.com/source/downloading.html
Refer to https://github.com/omnirom/android

$ repo init -u git://github.com/omnirom/android.git -b android-11

If you want to use the contents of omni-private.xml please check
https://github.com/omnirom/android/README and head over to send us a short
email notice. Otherwise you can simple remove the include of that xml
file and build without

If you want to build with microG add the contents of <device>/repo/local_manifest.xml

$ repo sync

# Build Android source

Refer to http://source.android.com/source/building.html

# prepare environment

$ export ROM_BUILDTYPE=<MICROG|WEEKLY>
$ export TEMPORARY_DISABLE_PATH_RESTRICTIONS=true
$ export ALLOW_MISSING_DEPENDENCIES=true
$ source build/envsetup.sh
$ lunch omni_rpi4-userdebug

# build

Use -j[n] option with make, if build host has a good number of CPU cores.

# prepare boot device (sdcard or USB device)

Devices with GPT partition table cant be used as an USB boot device
You must use MBR (msdos) parition table

The boot devices MUST contain 4 partitions with the following layout
(parition sizes here are the minimum - userdata should be made bigger
if you have the space)

Device          Start      End  Sectors  Size Id Type
boot               2    262145   262144  128M  c W95 FAT32 (LBA)
system        264192   4458495  4194304    2G 83 Linux
vendor        4458496  4982783   524288  256M 83 Linux
userdata      4982784 16777215 11794432  5.6G 83 Linux

THE PARTITION ORDER MUST BE LIKE THIS

Starting with android-11 project quotas are mandatory in userdata
parition. Prebuilt images have it enabled but if you recreate your
userdata you MUST enable project quotas with e.g.

tune2fs -O project,quota <userdata partition device>

# boot order

Switching between booting from sdcard and USB is done in config.txt
by enabling the needed overlay

...
# booting from sdcard
dtoverlay=rpi-android-sdcard

# booting from usb
dtoverlay=rpi-android-usb
....

You can still mess with the fstab if you want and dont use the overlay

Note

the files in scripts are what I use to create and update
feel free to adjust and update to your needs if you want

# flash to boot device with e.g. dd
