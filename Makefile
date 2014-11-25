CXXFLAGS=-Wall -g
BINARIES=led-matrix minimal-example text-example

# Where our library resides. It is split between includes and the binary
# library in lib
RGB_INCDIR=include
RGB_LIBDIR=lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread -lini_config

all : $(BINARIES)


$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR)

led-matrixd : led-matrixd-main.o sign-long-sequence.o ini-reader.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) $^  -o $@ $(LDFLAGS)

led-matrixd-main.o: sign-long-sequence.o ini-reader.o

sign-long-sequence.o: ini-reader.o

led-matrix : demo-main.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) demo-main.o -o $@ $(LDFLAGS)

minimal-example : minimal-example.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) minimal-example.o -o $@ $(LDFLAGS)

text-example : text-example.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) text-example.o -o $@ $(LDFLAGS)

%.o : %.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(BINARIES)
	$(MAKE) -C lib clean
