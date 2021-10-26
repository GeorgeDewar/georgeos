#!/bin/bash
outfile=$1
size_mb=$2
size_bytes=$(($2 << 20))
size_sectors=$(($size_bytes >> 9))

echo "Creating image of ${size_mb}MB (${size_sectors} sectors)"

# Create the file
dd if=/dev/zero of=${outfile} bs=1 count=0 seek=${size_bytes}
# Create a partition table
parted -s ${outfile} mklabel msdos
# Create a partition
parted -s ${outfile} unit B mkpart primary fat32 2048 $(expr ${size_bytes} \- 1)
