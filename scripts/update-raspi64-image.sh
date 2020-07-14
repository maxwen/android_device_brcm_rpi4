#!/bin/sh

OUT_DIR_ROOT=$OUT_DIR/target/product/rpi4/

if  [ ! -f "$OUT_DIR_ROOT/system.img" ]; then
    echo "no system.img"
    exit 0
fi

if  [ ! -f "$OUT_DIR_ROOT/vendor.img" ]; then
    echo "no vendor.img"
    exit 0
fi

if  [ ! -f "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm64/boot/Image" ]; then
    echo "no Image"
    exit 0
fi

sudo mkdir /mnt/tmp
sudo mount /dev/sdc1 /mnt/tmp
sudo cp "$OUT_DIR_ROOT/ramdisk.img" /mnt/tmp
sudo cp "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm64/boot/Image" /mnt/tmp/Image
sudo cp "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm64/boot/dts/broadcom/bcm2711-rpi-4-b.dtb" /mnt/tmp
sudo mkdir /mnt/tmp/overlays/
sudo cp "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/vc4-kms-v3d-pi4.dtbo" /mnt/tmp/overlays/
sudo cp "$OUT_DIR_ROOT/obj/KERNEL_OBJ/arch/arm64/boot/dts/overlays/dwc2.dtbo" /mnt/tmp/overlays/
sudo cp vendor/brcm/rpi4/proprietary/boot/* /mnt/tmp
sync

sudo dd if=$OUT_DIR_ROOT/system.img of=/dev/sdc2 bs=1M
sudo dd if=$OUT_DIR_ROOT/vendor.img of=/dev/sdc3 bs=1M

sudo umount /mnt/tmp
sudo rm -fr /mnt/tmp

exit 1
