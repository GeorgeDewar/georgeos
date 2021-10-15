; -----------------------------------------------------------------------------
; Disk description table
; 
; This makes the floppy identifiable. The values are mostly those used by IBM
; for a 1.44 MB, 3.5" diskette, but I have notably set ReservedForBoot to 2 so
; that we can have a larger bootloader.
; -----------------------------------------------------------------------------

%define BYTES_PER_SECTOR        512
%define NUM_RESERVED_SECTORS    3
%define NUMBER_OF_FATS          2
%define SECTORS_PER_FAT         9
%define ROOT_DIR_ENTRIES        224
%define ROOT_DIR_LOCATION       NUM_RESERVED_SECTORS + (NUMBER_OF_FATS * SECTORS_PER_FAT)
%define SECTORS_PER_DIR         ROOT_DIR_ENTRIES * 32 / BYTES_PER_SECTOR

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
SectorsPerTrack     dw SECTORS_PER_TRACK    ; Varies by disk size, set externally
Sides               dw 2                    ; Number of sides/heads
HiddenSectors       dd 0                    ; Number of hidden sectors
LargeSectors        dd 0                    ; Number of LBA sectors
DriveNo             dw 0                    ; Drive No: 0
Signature           db 41                   ; Drive signature: 41 for floppy
VolumeID            dd 00000000h            ; Volume ID: any number
VolumeLabel         db "GEORGEOS   "        ; Volume Label: any 11 chars
FileSystem          db "FAT12   "           ; File system type: don't change!
