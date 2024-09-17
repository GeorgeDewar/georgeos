#include "system.h"

static void vsprintf(char* buffer, char* string, va_list argp);

/** Read from a file into the specified buffer, up to *len* bytes */
int32_t read(int16_t fp, void* buffer, int len) {
    fprintf(stddebug, "Attempting to read from file %d\n", fp);
    FileHandle handle = open_files[fp];
    if (handle.type == NULL) {
        // The handle does not exist
        fprintf(stddebug, "Attempted to read from a NULL handle\n");
        return FAILURE;
    } else if(handle.type == STREAM) {
        handle.stream_device.read(buffer, len, true);
        return SUCCESS; // TODO: Len
    } else if(handle.type == FILE) {
        fprintf(stddebug, "Reading file\n");
        FileDescriptor fd = handle.file_descriptor;
        FileSystem *filesystem = handle.file_descriptor.filesystem;
        uint32_t clusters_read;
        if (filesystem->driver->read_file(filesystem, fd.location_on_disk, buffer, &clusters_read) == FAILURE) {
            return FAILURE;
        }
        return fd.size;
    } else if(handle.type == BLOCK) {
        fprintf(stddebug, "Reading from block device\n");
        BlockDeviceDescriptor bd = handle.block_descriptor;
        DiskDevice *device = bd.block_device;
        device->driver->read_sectors(device, bd.cursor / 512, 1, buffer); // TODO: Len
        return 512;
    } else {
        // Unsupported handle type
        fprintf(stddebug, "Unsupported handle type\n");
        return FAILURE;
    }
}

/** Write to a file from the specified buffer, *len* bytes */
bool write(int16_t fp, char* buffer, int len) {
    FileHandle handle = open_files[fp];
    if (handle.type == NULL) {
        // The handle does not exist
        return FAILURE;
    } else if(handle.type == STREAM) {
        handle.stream_device.write(buffer, len);
        return SUCCESS;
    } else {
        // Unsupported handle type
        return FAILURE;
    }
}

char bchars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
void int_to_string(uint32_t i, uint8_t base, char* buffer) {
    char tbuf[32];
    int pos = 0;
    int opos = 0;
    int top = 0;

    if (i == 0 || base > 16) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (i != 0) {
        tbuf[pos] = bchars[i % base];
        pos++;
        i /= base;
    }
    top=pos--;
    for (opos=0; opos<top; pos--,opos++) {
        buffer[opos] = tbuf[pos];
    }
    buffer[opos] = 0;
}

int string_to_int(char *string) {
    int value = 0;
    int len = strlen(string);
    int multiplier = 1;
    for(int i=0; i<len; i++) {
        char chr = string[len - (i+1)];
        uint8_t digit_val = chr - '0';
        value += digit_val * multiplier;
        multiplier *= 10;
    }
    return value;
}

void printf(char* string, ...) {
    va_list argp;
    va_start(argp, string);

    vfprintf(stdout, string, argp);
    va_end(argp);
}

void fprintf(int16_t fp, char* string, ...) {
    va_list argp;
    va_start(argp, string);

    vfprintf(fp, string, argp);
    va_end(argp);
}

void vfprintf(int16_t fp, char* string, va_list argp) {
    char buffer[128];
    vsprintf(buffer, string, argp);
    write(fp, buffer, strlen(buffer));
}

static void vsprintf(char* buffer, char* string, va_list argp) {
    uint8_t num_length = 0;
    while(*string != 0) {
        if (*string == '%') {
            num_length = 0;
            string++;
            
            // If there is a number, copy it into a buffer and parse it
            uint8_t zero_pad_string_length = 0;
            char zero_pad_length_str[8];
            while (*string >= '0' && *string <= '9') {
                //fprintf(stddebug, "Adding chr %x\n", *string);
                zero_pad_length_str[zero_pad_string_length++] = *string;
                string++; // next digit is num digits
            }
            zero_pad_length_str[zero_pad_string_length] = 0;
            if (zero_pad_string_length > 0) {
                //fprintf(stddebug, "%s\n", zero_pad_length_str);
                num_length = string_to_int(zero_pad_length_str);
                //fprintf(stddebug, "%d\n", num_length);
            }

            if(*string == '%'){ // %% escapes %
                *buffer++ = *string;
            } else if (*string == 's') {
                char* str = va_arg(argp, int);
                strcpy(str, buffer);
                buffer += strlen(str); // erase null byte
            } else if (*string == 'd') {
                int number = va_arg(argp, int);
                char num_string[16] = {0};
                int_to_string(number, 10, num_string);
                strcpy(num_string, buffer);
                buffer += strlen(num_string); // erase null byte
            } else if (*string == 'x') {
                int number = va_arg(argp, int);
                char num_string[16] = {0};
                int_to_string(number, 16, num_string);
                int num_digits = strlen(num_string);
                int padding = num_length - num_digits;
                while (padding > 0) {
                    *buffer++ = '0';
                    padding--;
                }
                strcpy(num_string, buffer);
                buffer += strlen(num_string); // erase null byte
            }
        } else {
            *buffer++ = *string;
        }
        string++;
    }
    *buffer = 0;
}
