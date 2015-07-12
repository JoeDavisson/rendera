# Makefile for mingw cross-compiler
# put fltk-1.3.3 local to this directory and run "make fltk" first!

HOST=i686-w64-mingw32

# source directories
SRC_DIR=src

INCLUDE=-I$(SRC_DIR) -Ifltk-1.3.3

CXX=$(HOST)-g++

# obtain the current version number from git
NAME="Rendera "
VERSION=$(shell git describe --always --dirty)

# static compile stuff
CXXFLAGS=-O2 -static-libgcc -static-libstdc++ -DPACKAGE_STRING=\"$(NAME)$(VERSION)\" $(INCLUDE)
LIBS=$(shell ./fltk-1.3.3/fltk-config --use-images --ldflags)
LIBS+=-lgdi32 -lcomctl32 -static -lpthread

OBJ= \
  $(SRC_DIR)/File.o \
  $(SRC_DIR)/FileSP.o \
  $(SRC_DIR)/FX.o \
  $(SRC_DIR)/Transform.o \
  $(SRC_DIR)/Bitmap.o \
  $(SRC_DIR)/Blend.o \
  $(SRC_DIR)/Map.o \
  $(SRC_DIR)/Octree.o \
  $(SRC_DIR)/Palette.o \
  $(SRC_DIR)/Quantize.o \
  $(SRC_DIR)/Button.o \
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
  $(SRC_DIR)/Math.o \
  $(SRC_DIR)/Project.o \
  $(SRC_DIR)/Render.o \
  $(SRC_DIR)/Stroke.o \
  $(SRC_DIR)/Undo.o \
  $(SRC_DIR)/View.o \
  $(SRC_DIR)/Crop.o \
  $(SRC_DIR)/Fill.o \
  $(SRC_DIR)/GetColor.o \
  $(SRC_DIR)/Offset.o \
  $(SRC_DIR)/Paint.o \
  $(SRC_DIR)/Text.o

# static link
default: $(OBJ)
	$(CXX) -o ./rendera.exe $(SRC_DIR)/Main.cxx $(OBJ) $(CXXFLAGS) $(LIBS)

# builds fltk for static linking
fltk:
	@cd ./fltk-1.3.3; \
	./configure --host=$(HOST) --enable-localjpeg --enable-localzlib --enable-localpng; \
	make; \
	cd ..
	@echo "FLTK libs built!"

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cxx $(SRC_DIR)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(SRC_DIR)/*.o 
	@rm -f ./rendera.exe
	@echo "Clean!"

