#!/bin/bash

TARGET_VOLUME=sdc
IMAGEDIR="/tftpboot"

echo "Target volume is ${TARGET_VOLUME}"
echo "Image dir is ${IMAGEDIR}"

if [ -e "/dev/${TARGET_VOLUME}1" ]
then
	umount /dev/${TARGET_VOLUME}1
fi
if [ -e "/dev/${TARGET_VOLUME}2" ]
then
	umount /dev/${TARGET_VOLUME}2
fi

#initialize SDCARD
sudo dd if=/dev/zero of=/dev/${TARGET_VOLUME} bs=4k count=32768
sync
echo "First sector: msdos"
sudo parted /dev/${TARGET_VOLUME} mklabel msdos
# Create the 1st partition (rootfs), start 64MB, length 256MB
sudo parted /dev/${TARGET_VOLUME} mkpart primary ext4 131072s 655359s
# Create the 2sd partition (usrfs), start 320MB, length 256MB
sudo parted /dev/${TARGET_VOLUME} mkpart primary ext4 655360s 1179647s
#Format ext4 1st and 2sd partition
sudo mkfs.ext4 /dev/${TARGET_VOLUME}2 -L usrfs
sync

# Show patition table
echo "Patition table is"
sudo fdisk -l /dev/${TARGET_VOLUME}

sleep 3

if [ -e "/dev/${TARGET_VOLUME}1" ]
then
	umount /dev/${TARGET_VOLUME}1
	
fi
if [ -e "/dev/${TARGET_VOLUME}2" ]
then
	umount /dev/${TARGET_VOLUME}2
fi


#copy firmware & bl1.bin, bl2.bin, tzsw.bin
echo "Copy bl1"
sudo dd if=${IMAGEDIR}/xu3-bl1.bin of=/dev/${TARGET_VOLUME} bs=512 seek=1

echo "Copy  bl2"
sudo dd if=${IMAGEDIR}/xu3-bl2.bin of=/dev/${TARGET_VOLUME} bs=512 seek=31

#copy u-boot
echo "Copy U-Boot"
sudo dd if=${IMAGEDIR}/xu3-u-boot.bin of=/dev/${TARGET_VOLUME} bs=512 seek=63

echo "Copy trustZone"
sudo dd if=${IMAGEDIR}/xu3-tzsw.bin of=/dev/${TARGET_VOLUME} bs=512 seek=2111

#copy kernel & flattened device tree
echo "Copy kernel"
sudo dd if=${IMAGEDIR}/xu3-uImage of=/dev/${TARGET_VOLUME} bs=512 seek=6304

echo "copy device tree"
sudo dd if=${IMAGEDIR}/exynos5422-odroidxu3.dtb of=/dev/${TARGET_VOLUME} bs=512 seek=22688

#copy rootfs
echo "Copy rootfs"
sudo dd if=${IMAGEDIR}/xu3-rootfs.ext4 of=/dev/${TARGET_VOLUME} bs=512 seek=131072

# Show patition table
echo "Patition table is"
sudo fdisk -l /dev/${TARGET_VOLUME}

