#!/bin/bash
outfile=$1
size_mb=$2
size_bytes=$(($2 << 20))

# Leave room for MBR and a bit extra
partition_1_start=2048

echo "Creating image of ${size_mb}MB"

partition_size=$(expr ${size_bytes} \- ${partition_1_start})

echo > /tmp/partition
truncate -s $partition_size /tmp/partition
mkfs.vfat /tmp/partition
echo Copying files to disk image...
mcopy -i /tmp/partition src/data/example.txt ::/

truncate -s $partition_1_start $outfile
cat /tmp/partition >>$outfile

# Create a partition table
parted -s ${outfile} mklabel msdos
# Create a partition
parted -s ${outfile} unit B mkpart primary fat32 2048 $(expr ${size_bytes} \- 1)
