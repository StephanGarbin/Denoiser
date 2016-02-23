CC=g++
CFLAGS= -Wall -std=c++11
LDFLAGS=
LIBS := -L/usr/local/lib -lboost_thread -lboost_system -lIlmImf -lIlmThread -lImath -lIex -lfftw3
INCLUDES := -IDenoiser -I/usr/local/include/OpenEXR
SOURCES := $(wildcard Denoiser/*.cpp)
#OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.cpp=.o)))
OBJECTS := $(SOURCES:.cpp=.o)
EXECUTABLE_1 = nnBM3D
EXECUTABLE_2 = BM3D

#$(info INCLUDES is $(INCLUDES))
#$(info SOURCES is $(SOURCES))
#$(info OBJECTS is $(OBJECTS))

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:$(EXECUTABLE_1) $(EXECUTABLE_2)

OBJS_1 = $(filter-out Denoiser/main.o, $(OBJECTS))
OBJS_2 = $(filter-out Denoiser/mainNNBM3DTest.o, $(OBJECTS))

$(EXECUTABLE_1): $(OBJS_1)
	$(CC) $(CFLAGS) -o $(EXECUTABLE_1) $(OBJS_1) $(LFLAGS) $(LIBS)

$(EXECUTABLE_2): $(OBJS_2)
	$(CC) $(CFLAGS) -o $(EXECUTABLE_2) $(OBJS_2) $(LFLAGS) $(LIBS)



#$(EXECUTABLE) : $(OBJECTS) 
#	$(CC) $(CFLAGS) $(INCLUDES) -o $(EXECUTABLE) $(OBJECTS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variab

$(OBJECTS): %.o: %.cpp
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -f $(OBJECTS) $(wildcard Denoiser/*.h.gch) $(NN)
