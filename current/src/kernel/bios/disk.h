char readSectorsLBA(int lba, char numSectors, char* buffer);

static char readSectors(char head, char cylinder, char sector, char numSectors, char *buffer);
