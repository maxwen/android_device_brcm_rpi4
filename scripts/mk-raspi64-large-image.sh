#!/bin/sh

#IN_IMAGE_DIR=$OUT_DIR/target/product/rpi4/
#IN_BOOT_FILES=$ANDROID_BUILD_TOP/vendor/brcm/rpi4/proprietary/boot/
#OUT_IMAGE_FILE=$HOME/raspberrypi/omni-$ROM_BUILDTYPE.img

if [ -z $ROM_BUILDTYPE ]; then
    echo "missing ROM_BUILDTYPE"
    exit 0
fi

options=$(getopt -o ho:i:b: -- "$@")
[ $? -eq 0 ] || { 
    echo "Incorrect options provided"
    exit 1
}
eval set -- "$options"
while true; do
    case "$1" in
    -o)
        shift
        OUT_IMAGE_FILE=$1
        shift
        ;;
    -i)
        shift
        IN_IMAGE_DIR=$1
        shift
        ;;
    -b)
        shift
        IN_BOOT_FILES=$1
        shift
        ;;
    -h)
        echo "-i <image folder> -b <boot file dir> -o <image file>"
        echo "e.g. -i $OUT_DIR/target/product/rpi4/ -b $ANDROID_BUILD_TOP/vendor/brcm/rpi4/proprietary/boot/ -o /tmp/omni.img"
        exit 0
        ;;
    --)
        shift
        break
        ;;
    esac
done

if [ -z $IN_IMAGE_DIR ]; then
    echo "missing -i <image folder>"
    exit 0
fi

if [ -z $OUT_IMAGE_FILE ]; then
    echo "missing -o <image file>"
    exit 0
fi

if [ -z $IN_BOOT_FILES ]; then
    echo "missing -b <boot file dir>"
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

if  [ ! -d $IN_BOOT_FILES ]; then
    echo "no <boot file dir>"
    exit 0
fi

if  [ ! -f "$IN_BOOT_FILES/config.txt" ]; then
    echo "no <boot file dir>/config.txt"
    exit 0
fi

echo "create: images $IN_IMAGE_DIR + boot files $IN_BOOT_FILES -> $OUT_IMAGE_FILE"

if [ -f $OUT_IMAGE_FILE ]; then
    rm $OUT_IMAGE_FILE
fi

echo "create empty image"
dd if=/dev/zero of="$OUT_IMAGE_FILE" bs=1M count=8192

echo "create partitions"
sudo sfdisk "$OUT_IMAGE_FILE"  << EOF
2,262144,0xC,*
264192,4194304,0x83,-
4458496,524288,0x83,-
4982784,,-
EOF

echo "mount partitions"
sudo kpartx -av "$OUT_IMAGE_FILE"

echo "create file systems"
sudo mkfs.vfat /dev/mapper/loop0p1 -n boot
sudo mkfs.ext4 /dev/mapper/loop0p2 -L system
sudo mkfs.ext4 /dev/mapper/loop0p3 -L vendor
sudo mkfs.ext4 /dev/mapper/loop0p4 -L userdata

echo "write system.img"
sudo dd if="$IN_IMAGE_DIR/system.img" of=/dev/mapper/loop0p2 bs=1M
echo "write vendor.img"
sudo dd if="$IN_IMAGE_DIR/vendor.img" of=/dev/mapper/loop0p3 bs=1M

echo "write boot patition"
sudo mkdir /mnt/tmp
sudo mount /dev/mapper/loop0p1 /mnt/tmp
sudo cp "$IN_IMAGE_DIR/ramdisk.img" /mnt/tmp/
sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/Image" /mnt/tmp/Image
sudo cp "$IN_IMAGE_DIR/obj/KERNEL_OBJ/arch/arm64/boot/dts/broadcom/bcm2711-rpi-4-b.dtb" /mnt/tmp/
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

echo "unmounting"
sudo umount /mnt/tmp
sudo rm -fr /mnt/tmp

sudo kpartx -dv "$OUT_IMAGE_FILE"

echo "now write $OUT_IMAGE_FILE to a sdcard"

exit 1
