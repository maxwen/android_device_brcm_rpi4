#!system/bin/sh
# called from rom

if [ -u $1 ]; then
    echo "provide argument rom or recovery"
    exit 0
fi

CONFIG_FILE_TARGET=config.txt

if [ $1 == "rom" ]; then
    CONFIG_FILE_SOURCE=config.txt.rom
elif [ $1 == "recovery" ]; then
    CONFIG_FILE_SOURCE=config.txt.twrp
else
    echo "provide argument rom or recovery"
    exit 0
fi

if [ ! -d "/mnt/boot" ]; then
    su root mkdir /mnt/boot
fi


if [ -b "/dev/block/mmcblk0p1" ]; then
    su root mount /dev/block/mmcblk0p1 /mnt/boot
    su root cp /mnt/boot/$CONFIG_FILE_SOURCE /mnt/boot/$CONFIG_FILE_TARGET
    su root umount /mnt/boot
    #/system/bin/reboot
elif [ -b "/dev/block/sda1" ]; then
    su root mount /dev/block/sda1 /mnt/boot
    su root cp /mnt/boot/$CONFIG_FILE_SOURCE /mnt/boot/$CONFIG_FILE_TARGET
    su root umount /mnt/boot
    #/system/bin/reboot
fi