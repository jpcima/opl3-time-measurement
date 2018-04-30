CXX = clang++
CXXFLAGS = -Wall -O2 -g -fsanitize=address
LDFLAGS = -fsanitize=address

MEASURER_CXX_OBJS = measure.o nukedopl3_eg.o
WAVES_CXX_OBJS = waves.o opl3_waves.o envelope.o

all: measurer waves

clean:
	rm -f \
	    measurer $(MEASURER_CXX_OBJS) \
	    waves $(WAVES_CXX_OBJS)

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

measurer: $(MEASURER_CXX_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

waves: $(WAVES_CXX_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
