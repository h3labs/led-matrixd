CXXFLAGS=-Wall -g -O3 -DDEBUG -std=c++0x
SRCS=$(addprefix src/, Native.cc)
OBJS=$(SRCS:.cc=.o)
DEPS=$(SRCS:.cc=.d)
INITDIR=/etc/init.d/
BINDIR=/bin/
INI=matrix.ini
LMD_CTRL=led-matrixd-ctrl.rb
LMD=led-matrixd.rb
CFGDIR=signcfg/
FONTDIR=fonts/
VARDIR=/var/led-matrixd/
LIBDIR=/lib/
RUBY_LIB=led_matrix_d/

# Where our library resides. It is split between includes and the binary
# library in lib
RGB_INCDIR=rpi-rgb-led-matrix/include/
RGB_LIBDIR=rpi-rgb-led-matrix/lib/
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread
CXXFLAGS+=-I$(RGB_INCDIR) -I$(RGB_LIBDIR) -fpic

all: libled-matrixd.so

install: libled-matrixd.so
	sudo install -v $(LMD_CTRL) $(INITDIR)
	sudo mkdir -pv $(VARDIR)
	sudo install -v $(LMD) $(VARDIR)
	sudo cp -fv $(INI) $(VARDIR)
	sudo cp -rfv $(CFGDIR) $(VARDIR)
	sudo cp -rfv $(FONTDIR) $(VARDIR)
	sudo cp -rfv $(RUBY_LIB) $(VARDIR)
	sudo cp -fv spaceupdate.sh $(VARDIR)
	sudo cp -fv restore.sh $(VARDIR)

libled-matrixd.so: $(OBJS) $(RGB_LIBRARY)
	$(CXX) -shared $(CXXFLAGS) $(OBJS) -o $@ -Wl,--whole-archive $(RGB_LIBRARY) -Wl,--no-whole-archive $(LDFLAGS)

uninstall:
	sudo rm -rfv $(VARDIR)
	sudo rm -fv $(BINDIR)led-matrixd

%.cc : %.h

$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR)

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

