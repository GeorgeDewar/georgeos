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

void sprintf(char* buffer, char* string, ...) {
    va_list argp;
    va_start(argp, string);

    vsprintf(buffer, string, argp);
    va_end(argp);
}

void fprintf(int16_t fp, char* string, ...) {
    va_list argp;
    va_start(argp, string);

    vfprintf(fp, string, argp);
    va_end(argp);
}

char* pattern_prefix[32] = {
    0, 0, 0,
    "\1[31m[%6d]\1[0m \1[34m%s\1[0m %s", // 3 = ERROR
    "\1[33m[%6d]\1[0m \1[34m%s\1[0m %s", // 4 = WARN
    0,
    "\1[32m[%6d]\1[0m \1[34m%s\1[0m %s", // 6 = INFO
    "\1[32m[%6d]\1[0m \1[34m%s\1[0m %s", // 7 = DEBUG
    "\1[32m[%6d]\1[0m \1[34m%s\1[0m %s", // 8 = TRACE
};

void kprintf(uint8_t level, char *prefix, char* string, ...) {
    uint8_t debug_level = DEBUG;
    char buffer[1024];

    // First render the message into a buffer
    va_list argp;
    va_start(argp, string);
    vsprintf(buffer, string, argp);
    va_end(argp);

    // Then print it to the appropriate places
    if (level <= log_level) {
        if (prefix) {
            fprintf(stdout, pattern_prefix[level], timer_ticks, prefix, buffer);
        } else {
            fprintf(stdout, "\1[32m[%6d]\1[0m %s", timer_ticks, buffer);
        }
    }

    if (level <= debug_level) {
        if (prefix) {
            fprintf(stddebug, "[%6d] [%s] %s", timer_ticks, prefix, buffer);
        } else {
            fprintf(stddebug, "[%6d] %s", timer_ticks, buffer);
        }
    }
}

void vfprintf(int16_t fp, char* string, va_list argp) {
    char buffer[1024];
    vsprintf(buffer, string, argp);
    write(fp, buffer, strlen(buffer));
}

static void vsprintf(char* buffer, char* string, va_list argp) {
    uint8_t width = 0; // minimum characters to print (zero-pads if required)
    uint8_t precision = 0; // for strings, limit of characters to print
    while(*string != 0) {
        if (*string == '%') {
            width = 0;
            string++;
            
            // If there is a number, copy it into a buffer and parse it
            uint8_t zero_pad_string_length = 0;
            char zero_pad_length_str[8];
            while (*string >= '0' && *string <= '9') {
                zero_pad_length_str[zero_pad_string_length++] = *string;
                string++; // next digit is num digits
            }
            zero_pad_length_str[zero_pad_string_length] = 0;
            if (zero_pad_string_length > 0) {
                width = string_to_int(zero_pad_length_str);
            }

            if (*string == '.') {
                // Collect the precision
                *string++; // skip the dot

                uint8_t precision_string_length = 0;
                char precision_string[8];
                while (*string >= '0' && *string <= '9') {
                    precision_string[precision_string_length++] = *string;
                    string++;
                }
                precision_string[precision_string_length] = 0;
                if (precision_string_length > 0) {
                    precision = string_to_int(precision_string);
                }
            }

            if(*string == '%'){ // %% escapes %
                *buffer++ = *string;
            } else if (*string == 's') {
                char* str = va_arg(argp, int);
                int copylen = precision > 0 ? precision : strlen(str);
                strncpy(str, buffer, copylen);
                buffer += copylen; // erase null byte
            } else if (*string == 'd' || *string == 'x') {
                int base = 16;
                if (*string == 'd') base = 10;

                int number = va_arg(argp, int);
                char num_string[16] = {0};
                int_to_string(number, base, num_string);
                int num_digits = strlen(num_string);
                int padding = width - num_digits;
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
