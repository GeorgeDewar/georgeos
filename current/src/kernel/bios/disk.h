char readSectorsLBA(int lba, char numSectors, int segment, char* buffer);

static char readSectors(char head, char cylinder, char sector, char numSectors, int segment, char *buffer);
