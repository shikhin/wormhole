CXX=g++
CPPFLAGS=-Iinclude -std=c++11 -O3 -fopenmp
LDFLAGS=-fopenmp

SRCS=main.cc gauss.cc genus.cc virtual.cc moves.cc subdiag.cc search.cc
OBJS=$(subst .cc,.o,$(SRCS))

all: wormhole

wormhole: $(OBJS)
	$(CXX) -o wormhole $(OBJS) $(LDFLAGS)

clean:
	rm $(OBJS)
	rm wormhole
