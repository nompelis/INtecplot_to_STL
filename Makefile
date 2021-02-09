
include Makefile.in

#############################################################################

INCLUDE = -I $(LIB_DIR)
RPATH = -rpath=$(LIB_DIR)

COPTS += $(INCLUDE)
COPTS += $(DEBUG)
CXXOPTS += $(INCLUDE)
CXXOPTS += $(DEBUG)

###### targets ##############################################################
all: test

test:
	$(CC) $(COPTS) -c stl.c 
	$(CC) $(COPTS) main.c stl.o -lm

clean:
	rm -f *.o a.out *.so 

