CXX = g++
CFLAGS = -O3 -march=native -ffast-math -fomit-frame-pointer -Wall -Wno-unused -fmax-errors=5
#CFLAGS = -g -O0 -Wall -Wno-unused -fmax-errors=5
LIBS = -lm -lfltk -lfltk_images
OBJS = Blend.o Clone.o Bitmap.o Map.o Brush.o Stroke.o View.o Widget.o Palette.o Button.o ToggleButton.o Field.o Separator.o Gui.o check.o main.o

default: $(OBJS)
	$(CXX) -o rendera $(OBJS) $(CFLAGS) $(LIBS)

%.o: %.cxx
	$(CXX) -c $< -o $*.o $(CFLAGS)

clean:
	@rm -f rendera *.o
	@echo "Clean."

