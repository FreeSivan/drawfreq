CXX = g++
CFLAGS +=-c -O2 -g -std=gnu++0x -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD -I../include
LDFLAGS += -L../lib -lopencv_flann -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_features2d -lmad -lpthread 
OBJS = mixfft.o Mp3FreqAnaly.o drawfreq.o


drawfreq : $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o ../bin/$@
	@rm -rf $(OBJS)

clean:
	@rm -rf $(OBJS) ../bin/drawfreq

install:
	cp ../bin/drawfreq /usr/local/bin
	cp ../lib/libopencv* /usr/local/lib
$(OBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $< -o $@ -DFCDB_CONN 

