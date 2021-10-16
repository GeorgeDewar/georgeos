#include "system.h"

#define BUFFER_SIZE 160 * 80 // 160 cols * 80 rows
char console_buffer[BUFFER_SIZE];
static uint32_t cursor = 0; // defines current position and size (i.e. can't move cursor without deleting content)

static void console_write(char* data, int length);
static void console_print_char(char character);

struct StreamDevice sd_screen_console = {
    .read = &get_string, // TODO: Move in here?
    .write = &console_write
};

static Color ansi_colors[] = {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
};

static void console_write(char* data, int length) {
    //fprintf(stddebug, "Console write\n");
    for(int i=0; i<length; i++) {
        console_print_char(data[i]);
    }
}

/** Print the specified character to the screen console */
static void console_print_char(char character) {
    if (character == '\b') {
        if (cursor > 0) {
            cursor -= 1;
            return;
        }
    } else {
        console_buffer[cursor] = character;
    }
    
    // Advance the cursor
    cursor++;
    console_modified = true;

    if (cursor > BUFFER_SIZE) cursor = 0;
}

/** Draw the specified character at a certain character position */
static void console_putchar(int x_offset, int y_offset, int row, int col, char char_num, Color color) {
    uint16_t start_x = x_offset + col * (CONSOLE_CHAR_WIDTH + CONSOLE_CHAR_SPACE);
    uint16_t start_y = y_offset + row * (CONSOLE_CHAR_HEIGHT + CONSOLE_CHAR_SPACE);
    draw_char(start_x, start_y, char_num, color);
}

/** Draw the contents of the buffer onto the screen at the specified location */
void console_render(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    uint16_t num_cols = width / (CONSOLE_CHAR_WIDTH + CONSOLE_CHAR_SPACE);

    uint16_t row = 0;
    uint16_t col = 0;
    Color color = COLOR_WHITE;
    for(uint16_t i=0; i<cursor; i++) {
        if (console_buffer[i] == '\1') { // start of an ANSI color escape
            i += 2; // advance past bracket
            if (console_buffer[i++] == '0') { // check for reset and skip the first digit
                color = COLOR_WHITE;
            } else {
                char color_chr = console_buffer[i++]; // read the second digit
                color = ansi_colors[color_chr - '0']; // set the color
            }
            i++; // skip the 'm'
        }

        char character = console_buffer[i];

        // Note: We should not have backspace characters in the buffer
        if (character == '\n') {
            row++;
            col = 0;
            continue;
        }
        if (character == '\r') {
            col = 0;
            continue;
        }

        console_putchar(x, y, row, col, character, color);
        col++;

        if (col == num_cols) {
//            fprintf(stddebug, "row=%d col=%d", row, col);
            row++;
            col = 0;
        }
    }

    uint8_t cursor_visible = timer_ticks % 20 == 0;
    if (cursor_visible) console_putchar(x, y, row, col, '_', color);
}
