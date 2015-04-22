CXXFLAGS=-Wall -g -O3 -DDEBUG -Iinclude -std=c++0x
SRCS=led-matrixd-main.cc Beacon.cc DisplaysSequence.cc ini-reader.cc
OBJS=$(SRCS:.cc=.o)
DEPS=$(SRCS:.cc=.d)
BINARIES=led-matrix minimal-example text-example

# Where our library resides. It is split between includes and the binary
# library in lib
RGB_INCDIR=include
RGB_LIBDIR=lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread -lini_config

all: led-matrixd

led-matrixd: $(OBJS) $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.cc : %.h

$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR)

led-matrix : demo-main.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) demo-main.o -o $@ $(LDFLAGS)

minimal-example : minimal-example.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) minimal-example.o -o $@ $(LDFLAGS)

text-example : text-example.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) text-example.o -o $@ $(LDFLAGS)

%.o : %.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

%.d: %.cc
	@set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	rm -f *.o $(OBJECTS) $(BINARIES)
	rm -f *.d
	$(MAKE) -C lib clean

-include $(DEPS)

