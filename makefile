PROG := vlla-shader

SRCDIR := src
OBJDIR := obj
BINDIR := bin

SOURCES = vlla-shader.c \
	  kiss_fft.c \
	  kiss_fftr.c \
	  common/esUtil.c \

INCLUDES = -Isrc/inc/common -Isrc/inc -I/usr/local/include -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
OBJECTS = $(patsubst %,$(OBJDIR)/%,$(SOURCES:.c=.o))

CFLAGS := -DRPI_NO_X -g
LFLAGS = -lm -L/usr/local/lib -lGLESv2 -lEGL -L/opt/vc/lib -L./src/common -lasound -pthread -lvlla -lbcm_host
CC := gcc

all: $(PROG)

run: $(PROG)
	bin/$(PROG)
	
debug: $(PROG)
	gdb bin/$(PROG)

# linking the program.
$(PROG): $(OBJECTS) $(BINDIR)
	$(CC) $(OBJECTS) -o $(BINDIR)/$(PROG) $(LFLAGS)

$(BINDIR):
	mkdir -p $(BINDIR)

# compiling source files.
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -s -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)/common

clean:
ifeq ($(OS),Windows_NT)
	del $(OBJECTS)
else
	rm $(OBJECTS)
endif

.PHONY: all clean

