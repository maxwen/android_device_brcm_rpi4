#!/bin/sh

#IN_IMAGE_DIR=$OUT_DIR/target/product/rpi4/
#IN_BOOT_FILES=$ANDROID_BUILD_TOP/vendor/brcm/rpi4/proprietary/boot/
#SD_CARD_DEVICE=/dev/sdc
#UPDATE_BOOT_PARTITION=1

if [ -z $ROM_BUILDTYPE ]; then
    echo "missing ROM_BUILDTYPE"
    exit 0
fi

options=$(getopt -o nhi:d:b: -- "$@")
[ $? -eq 0 ] || { 
    echo "Incorrect options provided"
    exit 1
}
eval set -- "$options"
while true; do
    case "$1" in
    -i)
        shift
        IN_IMAGE_DIR=$1
        shift
        ;;
    -d)
        shift
        SD_CARD_DEVICE=$1
        shift
        ;;
    -b)
        shift
        IN_BOOT_FILES=$1
        shift
        ;;
    -n)
        UPDATE_BOOT_PARTITION=0
        shift
        ;;
    -h)
        echo "-i <input folder> -b <boot file dir> -d <sdcard device> -n"
        echo "e.g. -i $OUT_DIR/target/product/rpi4/ -b $ANDROID_BUILD_TOP/vendor/brcm/rpi4/proprietary/boot/ -d /dev/sdc"
        echo "-n dont update boot partition"
        exit 0
        ;;
    --)
        shift
        break
        ;;
    esac
done

if [ -z $IN_IMAGE_DIR ]; then
    echo "missing -i <input folder>"
    exit 0
fi

if [ -z $SD_CARD_DEVICE ]; then
    echo "missing -d <sdcard device> e.g. /dev/sdc"
    exit 0
fi

if  [ ! -f "$IN_IMAGE_DIR/system.img" ]; then
    echo "no <input folder>/system.img"
    exit 0
fi

if  [ ! -f "$IN_IMAGE_DIR/vendor.img" ]; then
    echo "no <input folder>/vendor.img"
    exit 0
fi

if  [ ! -f "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/Image" ]; then
    echo "no <input folder>/obj/KERNEL_OBJ/arch/arm64/boot/Image"
    exit 0
fi

if [ ! -b $SD_CARD_DEVICE ]; then
    echo "no <sdcard device>"
    exit 0
fi

if  [ ! -d $IN_BOOT_FILES ]; then
    echo "no <boot file dir>"
    exit 0
fi

if  [ ! -f "$IN_BOOT_FILES/config.txt" ]; then
    echo "no <boot file dir>/config.txt"
    exit 0
fi

echo "update: images $IN_IMAGE_DIR + boot files $IN_BOOT_FILES -> $SD_CARD_DEVICE"

if [ $UPDATE_BOOT_PARTITION -eq 1 ]; then
    echo update boot partition on $SD_CARD_DEVICE"1"
    sudo mkdir /mnt/tmp
    sudo mount $SD_CARD_DEVICE"1" /mnt/tmp
    sudo cp "$IN_IMAGE_DIR/ramdisk.img" /mnt/tmp
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/Image" /mnt/tmp/Image
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/broadcom/bcm2711-rpi-4-b.dtb" /mnt/tmp
    sudo mkdir /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/vc4-kms-v3d-pi4.dtbo" /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/dwc2.dtbo" /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/rpi-android-sdcard.dtbo" /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/rpi-android-usb.dtbo" /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/rpi-backlight.dtbo" /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/rpi-ft5406.dtbo" /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/ads7846.dtbo" /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/gpio-key.dtbo" /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/gpio-ir.dtbo" /mnt/tmp/overlays/
    sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/i2c-rtc.dtbo" /mnt/tmp/overlays/
    sudo cp $IN_BOOT_FILES/* /mnt/tmp/
    sync
fi

echo write system.img to $SD_CARD_DEVICE"2"
sudo dd if=$IN_IMAGE_DIR/system.img of=$SD_CARD_DEVICE"2" bs=1M
echo write vendor.img to $SD_CARD_DEVICE"3"
sudo dd if=$IN_IMAGE_DIR/vendor.img of=$SD_CARD_DEVICE"3" bs=1M

echo "enable project quota"
sudo tune2fs -O project,quota $SD_CARD_DEVICE"4"

sudo umount /mnt/tmp
sudo rm -fr /mnt/tmp

exit 1
