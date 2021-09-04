; -----------------------------------------------------------------------------
; Disk description table
; 
; This makes the floppy identifiable. The values are mostly those used by IBM
; for a 1.44 MB, 3.5" diskette, but I have notably set ReservedForBoot to 2 so
; that we can have a larger bootloader.
; -----------------------------------------------------------------------------

OEMLabel            db "GEORGEOS"     ; Disk label
BytesPerSector      dw 512            ; Bytes per sector
SectorsPerCluster   db 1              ; Sectors per cluster
ReservedForBoot     dw 2              ; Reserved sectors for boot record
NumberOfFats        db 2              ; Number of copies of the FAT
RootDirEntries      dw 224            ; Number of entries in root dir
                                      ; (224 * 32 = 7168 = 14 sectors to read)
LogicalSectors      dw 2880           ; Number of logical sectors
MediumByte          db 0F0h           ; Medium descriptor byte
SectorsPerFat       dw 9              ; Sectors per FAT
SectorsPerTrack     dw SECTORS_PER_TRACK ; Varies by disk size, set externally
Sides               dw 2              ; Number of sides/heads
HiddenSectors       dd 0              ; Number of hidden sectors
LargeSectors        dd 0              ; Number of LBA sectors
DriveNo             dw 0              ; Drive No: 0
Signature           db 41             ; Drive signature: 41 for floppy
VolumeID            dd 00000000h      ; Volume ID: any number
VolumeLabel         db "GEORGEOS   "  ; Volume Label: any 11 chars
FileSystem          db "FAT12   "     ; File system type: don't change!
