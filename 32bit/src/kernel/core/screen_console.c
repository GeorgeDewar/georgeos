#include "system.h"

#define console_cells (console_rows * console_cols)
#define console_buffer_size console_cells * sizeof(struct console_char)

struct console_char* console_buffer;
static int console_x;
static int console_y;
static int console_width;
static int console_height;
static int console_rows;
static int console_cols;
static Color current_color;

uint32_t cursor = 0; // defines current position and size (i.e. can't move cursor without deleting content)

static void console_write(char* data, int length);
static int console_get_offset(char row, char col);
void console_put_char_fixed(int offset, char character, Color color);
void console_redraw_cell(int offset);

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

// Represent each character along with its color
struct console_char {
    char character;
    Color color;
};

static struct console_char empty_cell = {
    .character = ' ',
    .color = COLOR_WHITE
};

void console_init(int x, int y, int width, int height) {
    console_x = x;
    console_y = y;
    console_width = width;
    console_height = height;

    console_rows = height / (CONSOLE_CHAR_HEIGHT);
    console_cols = width / (CONSOLE_CHAR_WIDTH + CONSOLE_CHAR_SPACE);

    // Initialise the buffer
    console_buffer = malloc(console_buffer_size);
    memset(console_buffer, ' ', console_buffer_size); // 0 shall be interpreted as an empty cell

    cursor = 0;
    current_color = COLOR_WHITE;
}

static void console_scroll(int how_much) {
    if (how_much < 1) die("Scroll amount must be greater than 0");
    int cell_line_size = console_cols * sizeof(struct console_char); // do not understand how it doesn't need to be this * sizeof(struct console_char)
    fprintf(stddebug, "Moving console buffer %d bytes up (%d cols, %d bytes per cell)\n", cell_line_size, console_cols, sizeof(struct console_char));
    memcpy(console_buffer + console_cols, console_buffer, console_buffer_size - cell_line_size);
    memset(console_buffer + (console_cells - console_cols), 0, cell_line_size);

    for(int i=0; i<console_cells; i++) {
        console_redraw_cell(i);
    }
}

void console_write_char(char character) {
    if (character == '\n') {
        // Move the cursor to the beginning of the next row
        int row = cursor / console_cols;
        console_put_char_fixed(cursor, ' ', current_color); // erase the cursor
        cursor = console_get_offset(row + 1, 0);
    } else if (character == '\r') {
        // Return the cursor to the beginning of the current row
        int row = cursor / console_cols;
        console_put_char_fixed(cursor, ' ', current_color); // erase the cursor
        cursor = console_get_offset(row, 0);
    } else if (character == '\b') {
        if (cursor > 0) {
            console_put_char_fixed(cursor, ' ', current_color);
            cursor -= 1;
            console_put_char_fixed(cursor, ' ', current_color);
        }
    }
    else {
        // Otherwise, write the character and its attribute byte to
        // video memory at our calculated offset.
        console_put_char_fixed(cursor, character, current_color);
        cursor++;
    }

    if (cursor >= console_cells) {
        console_scroll(1);
        cursor -= console_cols; // i.e. one row up
    }
    //fprintf(stddebug, "Cursor: %d\n", cursor);
}

void console_redraw_cell(int offset) {
    int row = offset / console_cols;
    int col = offset % console_cols;
    int x = console_x + (col * CONSOLE_CHAR_WIDTH);
    int y = console_y + (row * CONSOLE_CHAR_HEIGHT);
    //fprintf(stddebug, "Drawing character at offset %d, cell %d,%d, pixel %d,%d\n", offset, col, row, x, y);
    draw_char(x, y, console_buffer[offset].character, console_buffer[offset].color);
}

void console_put_char_fixed(int offset, char character, Color color) {
    console_buffer[offset].character = character;
    console_buffer[offset].color = color;
    console_redraw_cell(offset);
}

static int console_get_offset(char row, char col) {
    return (row * console_cols) + col;
}

static void console_write(char* data, int length) {
    int start = timer_ticks;
    for(int i=0; i<length; i++) {
        if (data[i] == '\1') { // start of an ANSI color escape
            i += 2; // advance past bracket
            if (data[i++] == '0') { // check for reset and skip the first digit
                current_color = COLOR_WHITE;
            } else {
                char color_chr = data[i++]; // read the second digit
                current_color = ansi_colors[color_chr - '0']; // set the color
            }
            i++; // skip the 'm'
        }
        console_write_char(data[i]);
    }
    int end = timer_ticks;
    int dur1 = end - start;
    start = timer_ticks;
    default_graphics_device->copy_buffer();
    end = timer_ticks;
    int dur2 = end - start;
    //fprintf(stddebug, "Writing %d chars; Drew in %d, refreshed in %d ticks\n", length, dur1, dur2);
}

void console_update_cursor(bool on) {
    if (on) {
        console_put_char_fixed(cursor, '_', current_color);
    } else {
        console_put_char_fixed(cursor, ' ', current_color);
    }
    default_graphics_device->copy_buffer();
}
