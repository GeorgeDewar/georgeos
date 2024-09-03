#!/bin/bash
outfile=$1
size_mb=$2
size_bytes=$(($2 << 20))

# Leave room for MBR and a bit extra
partition_1_start=512
partition_1_start_sectors=$(($partition_1_start >> 9))

echo "Creating image of ${size_mb}MB"

partition_size=$(expr ${size_bytes} \- ${partition_1_start})

echo > /tmp/partition
truncate -s $partition_size /tmp/partition
#mkfs.vfat /tmp/partition
mkfs.vfat -F 32 -R 16 /tmp/partition
echo Copying files to disk image...
mcopy -i /tmp/partition src/data/example.txt ::/

# Create an output file of the appropriate size
echo > $outfile
truncate -s $size_bytes $outfile
# Create a partition table
parted -s ${outfile} mklabel msdos
# Create a partition
parted -s ${outfile} unit B mkpart primary fat32 $partition_1_start $(expr ${size_bytes} \- 1)
# Copy the partition into the whole file system
dd if=/tmp/partition of=$outfile bs=512 seek=$partition_1_start_sectors
# Copy the MBR
dd if=build/mbr.bin of=$outfile bs=512 seek=0 count=1 conv=notrunc
# Copy the VBR
dd if=build/bootload.bin of=$outfile bs=512 seek=1 count=1 conv=notrunc
# Copy the rest of the bootloader to the reserved area
dd if=build/bootload.bin of=$outfile bs=512 seek=3 skip=1 conv=notrunc
