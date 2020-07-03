#!/bin/sh

OUT_DIR_ROOT=$OUT_DIR/target/product/rpi4/
OUT_IMAGE_FILE=$HOME/raspberrypi/omni.img

if  [ ! -f "$OUT_DIR_ROOT/system.img" ]; then
    echo "no system.img"
    exit 0
fi

if  [ ! -f "$OUT_DIR_ROOT/vendor.img" ]; then
    echo "no vendor.img"
    exit 0
fi

if  [ ! -f "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm/boot/zImage" ]; then
    echo "no zImage"
    exit 0
fi

rm $OUT_IMAGE_FILE
dd if=/dev/zero of="$OUT_IMAGE_FILE" bs=1M count=4096

sudo sfdisk "$OUT_IMAGE_FILE"  << EOF
2,262144,0xC,*
264192,2097152,0x83,-
2361344,524288,0x83,-
2885632,,-
EOF

sudo kpartx -av "$OUT_IMAGE_FILE"
sudo mkfs.vfat /dev/mapper/loop0p1 -n boot
sudo mkfs.ext4 /dev/mapper/loop0p2 -L system
sudo mkfs.ext4 /dev/mapper/loop0p3 -L vendor
sudo mkfs.ext4 /dev/mapper/loop0p4 -L userdata


sudo dd if="$OUT_DIR_ROOT/system.img" of=/dev/mapper/loop0p2 bs=1M
sudo dd if="$OUT_DIR_ROOT/vendor.img" of=/dev/mapper/loop0p3 bs=1M

sudo mkdir /mnt/tmp
sudo mount /dev/mapper/loop0p1 /mnt/tmp
sudo cp "$OUT_DIR_ROOT/ramdisk.img" /mnt/tmp/
sudo cp "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm/boot/zImage" /mnt/tmp/
sudo cp "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm/boot/dts/bcm2711-rpi-4-b.dtb" /mnt/tmp/
sudo mkdir /mnt/tmp/overlays/
sudo cp "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm/boot/dts/overlays/vc4-kms-v3d-pi4.dtbo" /mnt/tmp/overlays/
sudo cp "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm/boot/dts/overlays/dwc2.dtbo" /mnt/tmp/overlays/
sudo cp vendor/brcm/rpi4/proprietary/boot/* /mnt/tmp/
sync
sudo umount /mnt/tmp
sudo rmdir /mnt/tmp

sudo kpartx -dv "$OUT_IMAGE_FILE"
exit 1
