CXX = g++
CFLAGS = -O2 -Wall -Wno-unused -fmax-errors=20
LIBS = -lfltk -lfltk_images 
OBJS = Widget.o Button.o Field.o Gui.o Main.o

default: $(OBJS)
	$(CXX) -o rendera $(OBJS) $(CFLAGS) $(LIBS)

%.o: %.cxx
	$(CXX) -c $< -o $*.o $(CFLAGS)

clean:
	@rm -f rendera *.o
	@echo "Clean."

