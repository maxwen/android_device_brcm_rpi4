#!/sbin/sh
# called from recovery
# based on resize script from https://konstakang.com/

# detect storage device
if [ -b "/dev/block/mmcblk0" ]; then
DEVICE=mmcblk0
PARTITION=p4
else
DEVICE=sda
PARTITION=4
fi

# make sure we have project quotas
tune2fs -O project,quota /dev/block/$DEVICE$PARTITION 1>> /tmp/tune2fs.log 2>&1

EXIT=$?
if [ $EXIT != "0" ]; then
    echo $EXIT >> /tmp/tune2fs.log
    exit $EXIT
fi

exit 0
