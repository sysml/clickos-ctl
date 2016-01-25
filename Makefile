CXXFLAGS	:=
CXXFLAGS	+= -Wall -MD -MP -g -O3 -std=gnu++11

LXXFLAGS	:=
LXXFLAGS	+= -lxenstore


OBJECTS		:=
OBJECTS		+= args.o
OBJECTS		+= clickos.o
OBJECTS		+= util.o
OBJECTS		+= xs.o


all: clickos

clickos: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LXXFLAGS) -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o *.d

distclean: clean
	rm -f clickos


.PHONY: clean distclean

-include *.d
