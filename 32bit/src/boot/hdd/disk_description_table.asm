; -----------------------------------------------------------------------------
; Disk description table
; 
; This uses up the space in the MBR where the disk description table would be,
; since the BIOS may expect that, and also contains some filesystem-specific
; defines.
; TODO: Don't hardcode SECTORS_PER_FAT, it locks the size of the FS to 40MB.
; -----------------------------------------------------------------------------

%define PARTITION_START_SECTOR  2048
%define BYTES_PER_SECTOR        512
%define NUM_RESERVED_SECTORS    16
%define NUMBER_OF_FATS          2
%define SECTORS_PER_FAT         615
%define ROOT_DIR_ENTRIES        16
%define FAT_LOCATION            PARTITION_START_SECTOR + NUM_RESERVED_SECTORS
%define ROOT_DIR_LOCATION       PARTITION_START_SECTOR + NUM_RESERVED_SECTORS + (NUMBER_OF_FATS * SECTORS_PER_FAT)
%define SECTORS_PER_DIR         ROOT_DIR_ENTRIES * 32 / BYTES_PER_SECTOR
%define DATA_START              ROOT_DIR_LOCATION

; TODO: Skip this?
OEMLabel            db "GEORGEOS"           ; Disk label
BytesPerSector      dw BYTES_PER_SECTOR     ; Bytes per sector
SectorsPerCluster   db 1                    ; Sectors per cluster
ReservedForBoot     dw NUM_RESERVED_SECTORS ; Reserved sectors for boot record
NumberOfFats        db NUMBER_OF_FATS       ; Number of copies of the FAT
RootDirEntries      dw ROOT_DIR_ENTRIES     ; Number of entries in root dir
                                            ; (224 * 32 = 7168 = 14 sectors to read)
LogicalSectors      dw 2880                 ; Number of logical sectors
MediumByte          db 0F0h                 ; Medium descriptor byte
SectorsPerFat       dw SECTORS_PER_FAT      ; Sectors per FAT
SectorsPerTrack     dw 0                    ; Varies by disk size, set externally
Sides               dw 2                    ; Number of sides/heads
HiddenSectors       dd 0                    ; Number of hidden sectors
LargeSectors        dd 0                    ; Number of LBA sectors
DriveNo             dw 0                    ; Drive No: 0
Signature           db 41                   ; Drive signature: 41 for floppy
VolumeID            dd 00000000h            ; Volume ID: any number
VolumeLabel         db "GEORGEOS   "        ; Volume Label: any 11 chars
FileSystem          db "FAT12   "           ; File system type: don't change!
