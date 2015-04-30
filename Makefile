CXXFLAGS=-Wall -g -O3 -DDEBUG -std=c++0x
SRCS=$(addprefix src/,LedMatrixDMain.cc Beacon.cc DisplaysSequence.cc INIReader.cc)
OBJS=$(SRCS:.cc=.o)
DEPS=$(SRCS:.cc=.d)
INITDIR=/etc/init.d/
BINDIR=/bin/
INI=matrix.ini
CFGDIR=signcfg/
FONTDIR=fonts/
VARDIR=/var/led-matrixd/
BINARIES=led-matrix minimal-example text-example

# Where our library resides. It is split between includes and the binary
# library in lib
RGB_INCDIR=include/
RGB_LIBDIR=lib/
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread -lini_config
CXXFLAGS+=-I$(RGB_INCDIR) -I$(RGB_LIBDIR)

all: led-matrixd

led-matrixd: $(OBJS) $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

install: led-matrixd
	sudo install -v $^ $(BINDIR)
	sudo mkdir -pv $(VARDIR)
	sudo cp -rfv $(INI) $(VARDIR)
	sudo cp -rfv $(CFGDIR) $(VARDIR)
	sudo cp -rfv $(FONTDIR) $(VARDIR)
	sudo cp -rfv spaceupdate.sh $(VARDIR)
	sudo cp -rfv restore.sh $(VARDIR)

uninstall:
	sudo rm -rfv $(VARDIR)
	sudo rm -fv $(BINDIR)led-matrixd

%.cc : %.h

$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR)

led-matrix : src/demo-main.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) demo-main.o -o $@ $(LDFLAGS)

minimal-example : src/minimal-example.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) minimal-example.o -o $@ $(LDFLAGS)

text-example : src/text-example.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) text-example.o -o $@ $(LDFLAGS)

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

%.d: %.cc
	@set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	rm -f $(OBJS) $(BINARIES)
	rm -f $(DEPS)
	$(MAKE) -C lib clean

-include $(DEPS)

