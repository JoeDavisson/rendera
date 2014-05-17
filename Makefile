CXX = g++
CFLAGS = -O2 -Wall -Wno-unused -fmax-errors=5
LIBS = -lfltk -lfltk_images 
OBJS = Var.o Blend.o Bmp.o Bitmap.o Map.o Stroke.o View.o Widget.o Button.o Field.o Gui.o main.o

default: $(OBJS)
	$(CXX) -o rendera $(OBJS) $(CFLAGS) $(LIBS)

%.o: %.cxx
	$(CXX) -c $< -o $*.o $(CFLAGS)

clean:
	@rm -f rendera *.o
	@echo "Clean."

