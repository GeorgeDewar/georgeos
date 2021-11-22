#!/bin/bash
outfile=$1
size_mb=$2
size_bytes=$(($2 << 20))

# Leave room for MBR and a bit extra
partition_1_start=2048
partition_1_start_sectors=$(($partition_1_start >> 9))

echo "Creating image of ${size_mb}MB"

partition_size=$(expr ${size_bytes} \- ${partition_1_start})

# Create the partition
echo > /tmp/partition
truncate -s $partition_size /tmp/partition
mkfs.vfat -F 32 -R 16 -b 15 /tmp/partition
# Copy the stage 1 bootloader code into the Volume Boot Record (sector 0 of the partition) without overwriting the BPB
dd if=build/boot/stage1.bin of=/tmp/partition bs=1 conv=notrunc count=3
dd if=build/boot/stage1.bin of=/tmp/partition bs=1 seek=90 skip=90 conv=notrunc count=422

echo Copying files to disk image...
mcopy -i /tmp/partition src/data/example.txt ::/

# Create an output file of the appropriate size
echo Creating partition layout...
set -v
echo > $outfile
truncate -s $size_bytes $outfile
# Create a partition table
parted -s ${outfile} mklabel msdos
# Create a partition
parted -s ${outfile} unit B mkpart primary fat32 $partition_1_start $(expr ${size_bytes} \- 1)
# Make bootable
parted -s ${outfile} toggle 1 boot
# Copy the partition into the whole file system
dd if=/tmp/partition of=$outfile bs=512 seek=$partition_1_start_sectors
