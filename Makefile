CXXFLAGS = -O3 -g -Wall -fmessage-length=0 -std=c++11 

OBJS =		src/main.o

LIBS =

TARGET =	emcpp

CROSS ?= 
AR              := $(CROSS)ar
AS              := $(CROSS)as
CC              := $(CROSS)gcc
CPP             := $(CROSS)gcc -E
CXX             := $(CROSS)g++
LD              := $(CROSS)g++
NM              := $(CROSS)nm
OBJCOPY         := $(CROSS)objcopy
OBJDUMP         := $(CROSS)objdump
SIZE            := $(CROSS)size


$(TARGET):	$(OBJS) Makefile
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	
$(TARGET).s:	$(TARGET)
	$(OBJDUMP) -S --disassemble $(TARGET) > $(TARGET).s

all:	$(TARGET) $(TARGET).s

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).s
