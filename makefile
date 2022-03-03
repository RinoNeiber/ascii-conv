##################################################
CC = g++
CFLAGS = -Wall -MD -c -std=c++17
LDFLAGS = -static-libstdc++ -static-libgcc -static
EXECUTABLE = ../../bin/$(build_dir)/conv
VPATH = ../../

##################################################
SUBDIRS = .

LIBS = -lpng -lz
INCPATH = $(addsuffix ",$(addprefix -I"../../,$(SUBDIRS)))

OBJECTS := $(patsubst ../../%, %, $(wildcard $(addsuffix /*.cpp, $(addprefix ../../,$(SUBDIRS)))))
OBJECTS := $(OBJECTS:.cpp=.o)

##################################################
all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

# .cpp -> .o
%.o: %.cpp
	$(CC) $(INCPATH) $(CFLAGS) $(build_flags) $< -o $@

include $(wildcard $(addsuffix /*.d, $(SUBDIRS)))
