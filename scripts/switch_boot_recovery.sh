#!/sbin/sh
# called from recovery

if [ -u $1 ]; then
    # shortcut called from flashable zip
    CONFIG_FILE_SOURCE=config.txt.rom
fi

CONFIG_FILE_TARGET=config.txt

if [ $1 == "rom" ]; then
    CONFIG_FILE_SOURCE=config.txt.rom
elif [ $1 == "recovery" ]; then
    CONFIG_FILE_SOURCE=config.txt.twrp
else
    echo "will switch to rom"
fi

if [ -b "/dev/block/mmcblk0p1" ]; then
    mount /dev/block/mmcblk0p1 /boot
    cp /boot/$CONFIG_FILE_SOURCE /boot/$CONFIG_FILE_TARGET
    umount /boot
    #/system/bin/reboot
elif [ -b "/dev/block/sda1" ]; then
    mount /dev/block/sda1 /boot
    cp /boot/$CONFIG_FILE_SOURCE /boot/$CONFIG_FILE_TARGET
    umount /boot
    #/system/bin/reboot
fi

exit 1