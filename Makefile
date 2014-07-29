include config.mak

CXX = g++

default: $(OBJS)
	$(CXX) -o rendera $(OBJS) $(LIBS)

%.o: %.cxx
	$(CXX) -c $< -o $*.o $(CFLAGS)

clean:
	@rm -f rendera *.o
	@echo "Clean."

