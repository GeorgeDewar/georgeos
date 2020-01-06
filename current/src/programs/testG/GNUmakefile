BD = ../../

CC = bcc
CFLAGS = -c -Md -ansi -O -I

LD = ld86
LDFLAGS = -y -d -T8000 -HD800 -0

# ----------------------------------------------------------------------------
SRC = test.c

TARGET = $(addsuffix .bin, $(basename $(SRC)))
OBJ = $(addsuffix .o, $(basename $(SRC)))

HEADER = $(BD)/include
LIBDIR = $(BD)/lib

# ----------------------------------------------------------------------------
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(TARGET) $(OBJ)
