
include ../Makeheader

INCLUDE = -I.. -I../libs/fft/include -I../libs
CPPFLAGS += $(INCLUDE)

OBJDIR = ../obj
SRCS = $(wildcard *.cpp)
OBJECTS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

$(OBJDIR)/%.o : %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all: $(OBJECTS)