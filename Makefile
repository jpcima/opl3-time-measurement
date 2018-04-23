
CXX = clang++
CXXFLAGS = -Wall -O2 -g -fsanitize=address
LDFLAGS = -fsanitize=address

CXX_OBJS = measure.o nukedopl3_eg.o

all: measurer

clean:
	rm -f measurer $(CXX_OBJS)

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

measurer: $(CXX_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
