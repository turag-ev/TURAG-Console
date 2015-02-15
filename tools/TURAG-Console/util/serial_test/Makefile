
# Project setup
SRCDIR := src
TMPDIR := tmp
BINDIR := bin

TARGET := serial_test


# Tools
CC := $(TOOLPREFIX)gcc
CXX := $(TOOLPREFIX)g++
ASM := $(TOOLPREFIX)gcc
LD := $(TOOLPREFIX)gcc
OBJCOPY := $(TOOLPREFIX)objcopy
OBJDUMP := $(TOOLPREFIX)objdump
NM := $(TOOLPREFIX)nm
SIZE := $(TOOLPREFIX)size
AR := $(TOOLPREFIX)ar
STRIP := $(TOOLPREFIX)strip

# File lists
CPPSOURCES = $(wildcard $(SRCDIR)/*.cpp)
CSOURCES = $(wildcard $(SRCDIR)/*.c)
INCDIRS = src 

COBJECTS = $(patsubst $(SRCDIR)/%.c, $(TMPDIR)/%.o, $(CSOURCES))
CPPOBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(TMPDIR)/%.o, $(CPPSOURCES))
OBJECTS = $(COBJECTS) $(CPPOBJECTS) 


# default settings
FLAGS = -O2 $(addprefix -I, $(INCDIRS)) -Wall 
CFLAGS := $(FLAGS) -std=gnu99
CPPFLAGS := $(FLAGS) -std=gnu++0x
LDFLAGS = 

# Rules
all: $(BINDIR)/$(TARGET)


$(BINDIR)/$(TARGET):$(OBJECTS)
	-@[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) -lstdc++ -lm

$(TMPDIR)/%.o:$(SRCDIR)/%.cpp Makefile
	-@[ -d $(TMPDIR) ] || mkdir -p $(TMPDIR)
	$(CC) $(CPPFLAGS) -MMD -MP -MT $@ -MF $@.d -o $@ -c $< 

$(TMPDIR)/%.o:$(SRCDIR)/%.c Makefile
	-@[ -d $(TMPDIR) ] || mkdir -p $(TMPDIR)
	$(CC) $(CFLAGS) -MMD -MP -MT $@ -MF $@.d -o $@ -c $< 



.PHONY: clean
clean:
	-rm -f $(BINDIR)/$(TARGET)
	-rm -f $(TMPDIR)/*.o
	-rm -f $(TMPDIR)/*.o.d


-include $(wildcard $(TMPDIR)/*.o.d)
