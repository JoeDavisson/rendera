# Rendera Makefile
#
# The official fltk-1.3.7 source tree must in this directory.
# Please run "make fltk" first.
# Also run "make clean" before "make" the first time to build the images header.

# libxft-dev should be installed before compiling FLTK on linux
# (otherwise you'll have ugly, non-resizable fonts)
PLATFORM=linux
#PLATFORM=mingw32
#PLATFORM=mingw64

VERSION="0.2.3"
#VERSION=$(shell git describe --always --dirty)
#VERSION=$(shell git describe --always)

SRC_DIR=src
SRC_DIR_FX=src/FX 
INCLUDE=-I$(SRC_DIR) -I$(SRC_DIR_FX) -Ifltk-1.3.7
LIBS=$(shell ./fltk-1.3.7/fltk-config --use-images --ldstaticflags)

ifeq ($(PLATFORM),linux)
  HOST=
  CXX=g++
#  CXXFLAGS= -O3 -Wall -Werror -Wfatal-errors -DPACKAGE_STRING=\"v0.2.0\" $(INCLUDE)
  CXXFLAGS= -O3 -std=c++11 -ffast-math -Wall -DPACKAGE_STRING=\"$(VERSION)\" $(INCLUDE)
  EXE=rendera
endif

ifeq ($(PLATFORM),mingw32)
  HOST=i686-w64-mingw32
  CXX=$(HOST)-g++
  CXXFLAGS= -O3 -ffast-math -Wall -static-libgcc -static-libstdc++ -DPACKAGE_STRING=\"$(VERSION)\" $(INCLUDE)
  LIBS+=-lgdi32 -lcomctl32 -static -lpthread
  EXE=rendera.exe
endif

ifeq ($(PLATFORM),mingw64)
  HOST=x86_64-w64-mingw32
  CXX=$(HOST)-g++
  CXXFLAGS= -O3 -ffast-math -Wall -static-libgcc -static-libstdc++ -DPACKAGE_STRING=\"$(VERSION)\" $(INCLUDE)
  LIBS+=-lgdi32 -lcomctl32 -static -lpthread
  EXE=rendera.exe
endif

OBJ= \
  $(SRC_DIR)/FX/FX.o \
  $(SRC_DIR)/FX/Normalize.o \
  $(SRC_DIR)/FX/Equalize.o \
  $(SRC_DIR)/FX/RotateHue.o \
  $(SRC_DIR)/FX/ValueStretch.o \
  $(SRC_DIR)/FX/Saturate.o \
  $(SRC_DIR)/FX/Desaturate.o \
  $(SRC_DIR)/FX/Colorize.o \
  $(SRC_DIR)/FX/PaletteColors.o \
  $(SRC_DIR)/FX/Invert.o \
  $(SRC_DIR)/FX/AlphaInvert.o \
  $(SRC_DIR)/FX/AlphaClear.o \
  $(SRC_DIR)/FX/GaussianBlur.o \
  $(SRC_DIR)/FX/Sharpen.o \
  $(SRC_DIR)/FX/UnsharpMask.o \
  $(SRC_DIR)/FX/BoxFilters.o \
  $(SRC_DIR)/FX/Sobel.o \
  $(SRC_DIR)/FX/Bloom.o \
  $(SRC_DIR)/FX/Randomize.o \
  $(SRC_DIR)/FX/Restore.o \
  $(SRC_DIR)/FX/SideAbsorptions.o \
  $(SRC_DIR)/FX/RemoveDust.o \
  $(SRC_DIR)/FX/StainedGlass.o \
  $(SRC_DIR)/FX/Painting.o \
  $(SRC_DIR)/FX/Marble.o \
  $(SRC_DIR)/FX/ForwardFFT.o \
  $(SRC_DIR)/FX/InverseFFT.o \
  $(SRC_DIR)/FX/Dither.o \
  $(SRC_DIR)/File.o \
  $(SRC_DIR)/FileSP.o \
  $(SRC_DIR)/Transform.o \
  $(SRC_DIR)/Bitmap.o \
  $(SRC_DIR)/Blend.o \
  $(SRC_DIR)/Map.o \
  $(SRC_DIR)/Quadtree.o \
  $(SRC_DIR)/KDtree.o \
  $(SRC_DIR)/Octree.o \
  $(SRC_DIR)/Palette.o \
  $(SRC_DIR)/Quantize.o \
  $(SRC_DIR)/Button.o \
  $(SRC_DIR)/RepeatButton.o \
  $(SRC_DIR)/CheckBox.o \
  $(SRC_DIR)/DialogWindow.o \
  $(SRC_DIR)/Group.o \
  $(SRC_DIR)/InputFloat.o \
  $(SRC_DIR)/InputInt.o \
  $(SRC_DIR)/InputText.o \
  $(SRC_DIR)/Separator.o \
  $(SRC_DIR)/StaticText.o \
  $(SRC_DIR)/ToggleButton.o \
  $(SRC_DIR)/Widget.o \
  $(SRC_DIR)/Brush.o \
  $(SRC_DIR)/Clone.o \
  $(SRC_DIR)/Dialog.o \
  $(SRC_DIR)/Gui.o \
  $(SRC_DIR)/Project.o \
  $(SRC_DIR)/Fractal.o \
  $(SRC_DIR)/Render.o \
  $(SRC_DIR)/ExtraMath.o \
  $(SRC_DIR)/Stroke.o \
  $(SRC_DIR)/Undo.o \
  $(SRC_DIR)/View.o \
  $(SRC_DIR)/Selection.o \
  $(SRC_DIR)/Fill.o \
  $(SRC_DIR)/GetColor.o \
  $(SRC_DIR)/Offset.o \
  $(SRC_DIR)/Paint.o \
  $(SRC_DIR)/Text.o

default: $(OBJ)
	$(CXX) -o ./$(EXE) $(SRC_DIR)/Main.cxx $(OBJ) $(CXXFLAGS) $(LIBS)

fltk:
	@cd ./fltk-1.3.7; \
	make clean; \
	./configure --host=$(HOST) --enable-xft --enable-localjpeg --enable-localzlib --enable-localpng --disable-xdbe; \
	make -j6; \
	cd ..
	@echo "FLTK libs built!"

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cxx $(SRC_DIR)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(SRC_DIR)/*.o 
	@rm -f $(SRC_DIR)/FX/*.o 
#	@rm -f ./$(EXE)
	@echo "Clean!"
	gcc -O3 ./makeheader.c -o ./makeheader
	./makeheader src/Images.H images/*.png
	@echo "Image.H built"

