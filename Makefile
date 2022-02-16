#
# Install XCode and XQuartz on OSX to compile this
#

CXXFLAGS = -DXLIB -fPIC -DPIC -g -Wall -Wextra -Wno-unused-parameter

lib-objs := RCom.o RConfig.o RFile.o RHexFile.o RString.o RTreeNode.o

#all: libRtk-base.a
all: libRtk-base.so

clean:
	rm -fv *~
	rm -fv *.o
	rm -fv *.a
	rm -fv *.so
	rm -fv *.dep


#=========================================================#
#dependencies

%.dep: %.cpp
	@echo Making $@
	@echo -n "$@ " > $@
	@$(CXX) -M $(CXXFLAGS) $< >> $@

-include $(subst .o,.dep,$(lib-objs))

#=========================================================#

libRtk-base.a: $(lib-objs)
	rm -f $@
	ar rs $@ $^

libRtk-base.so: $(lib-objs)
	g++ -shared -fPIC -g -o $@ $^ -Wl,-soname -Wl,$@.0.1
# -lefence

linkso:
	ln -s `pwd`/libRtk-base.so /usr/lib/libRtk-base.so
	ln -s `pwd`/libRtk-base.so /usr/lib/libRtk-base.so.1
	ln -s `pwd`/libRtk-base.so /usr/lib/libRtk-base.so.0.1

rmlinkso:
	rm /usr/lib/libRtk-base.so
	rm /usr/lib/libRtk-base.so.1
	rm /usr/lib/libRtk-base.so.0.1

#=========================================================#

bz: clean
	tar -cjvf ../Rtk-base.tbz *.h *.cpp Makefile
wc:
	wc *.h *.cpp

#=========================================================#

