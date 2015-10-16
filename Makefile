# Rendera Makefile
#
# The fltk-1.3.3 source tree must be available in this directory.
# Please run "make fltk" first to build the library before running "make".

PLATFORM=linux
#PLATFORM=mingw32
#PLATFORM=mingw64

NAME="Rendera "
VERSION=$(shell git describe --always --dirty)

SRC_DIR=src
INCLUDE=-I$(SRC_DIR) -Ifltk-1.3.3
LIBS=$(shell ./fltk-1.3.3/fltk-config --use-images --ldstaticflags)

ifeq ($(PLATFORM),linux)
  HOST=
  CXX=g++
  CXXFLAGS=-O3 -DPACKAGE_STRING=\"$(NAME)$(VERSION)\" $(INCLUDE)
  EXE=rendera
endif

ifeq ($(PLATFORM),mingw32)
  HOST=i686-w64-mingw32
  CXX=$(HOST)-g++
  CXXFLAGS=-O3 -static-libgcc -static-libstdc++ -DPACKAGE_STRING=\"$(NAME)$(VERSION)\" $(INCLUDE)
  LIBS+=-lgdi32 -lcomctl32 -static -lpthread
  EXE=rendera.exe
endif

ifeq ($(PLATFORM),mingw64)
  HOST=x86_64-w64-mingw32
  CXX=$(HOST)-g++
  CXXFLAGS=-O3 -static-libgcc -static-libstdc++ -DPACKAGE_STRING=\"$(NAME)$(VERSION)\" $(INCLUDE)
  LIBS+=-lgdi32 -lcomctl32 -static -lpthread
  EXE=rendera.exe
endif

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
  $(SRC_DIR)/Project.o \
  $(SRC_DIR)/Render.o \
  $(SRC_DIR)/ExtraMath.o \
  $(SRC_DIR)/Stroke.o \
  $(SRC_DIR)/Undo.o \
  $(SRC_DIR)/View.o \
  $(SRC_DIR)/Knife.o \
  $(SRC_DIR)/Fill.o \
  $(SRC_DIR)/GetColor.o \
  $(SRC_DIR)/Offset.o \
  $(SRC_DIR)/Paint.o \
  $(SRC_DIR)/Text.o

default: $(OBJ)
	$(CXX) -o ./$(EXE) $(SRC_DIR)/Main.cxx $(OBJ) $(CXXFLAGS) $(LIBS)

fltk:
	@cd ./fltk-1.3.3; \
	make clean; \
	./configure --host=$(HOST) --enable-localjpeg --enable-localzlib --enable-localpng --disable-xdbe; \
	make; \
	cd ..
	@echo "FLTK libs built!"

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cxx $(SRC_DIR)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(SRC_DIR)/*.o 
	@rm -f ./$(EXE)
	@echo "Clean!"

