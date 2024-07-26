CXXFLAGS = -O2 -g -Wall -fmessage-length=0 -std=c++11 -fopenmp   

OBJS =		src/main.o src/fastpool.o  

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


# Add -fopenmp for OMP

$(TARGET):	$(OBJS) Makefile src/Memory.h
	$(CXX) -o $(TARGET)  $(OBJS) $(LIBS)
	
	
$(TARGET).s:	$(TARGET)
	echo $(OBJDUMP) -S --disassemble $(TARGET) > $(TARGET).s

all:	$(TARGET) $(TARGET).s

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).s
