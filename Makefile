CXX = g++
CFLAGS = -O3 -march=native -ffast-math -fomit-frame-pointer -Wall -Wno-unused -fmax-errors=5

#valgrind
#CFLAGS = -g -O0 -Wall -Wno-unused -fmax-errors=5

LIBS = -lm -lfltk -lfltk_images -ljpeg -lpng
# static link
#LIBS = -lm -Wl,-Bsymbolic-functions /usr/lib/x86_64-linux-gnu/libfltk_images.a -lpng -lz /usr/lib/x86_64-linux-gnu/libfltk.a -lXext -lXft -lfontconfig -lfontconfig -lXinerama -ldl -lm -lX11

OBJS = Blend.o Clone.o Bitmap.o Map.o Brush.o Stroke.o View.o Widget.o Palette.o Button.o ToggleButton.o Field.o Separator.o Dialog.o Gui.o check.o load.o save.o main.o

default: $(OBJS)
	$(CXX) -o rendera $(OBJS) $(LIBS)

%.o: %.cxx
	$(CXX) -c $< -o $*.o $(CFLAGS)

clean:
	@rm -f rendera *.o
	@echo "Clean."

