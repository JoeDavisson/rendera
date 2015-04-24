# obtain the current version number from git
NAME="Rendera "

VERSION=$(shell git describe --always --dirty)

INCLUDE=-Ifile -Ifx -Igraphics -Igui -Imain -Itools -Iimages

CXX=g++
CXXFLAGS=-O2 -DPACKAGE_STRING=\"$(NAME)$(VERSION)\" $(INCLUDE)

LIBS=-lfltk -lpng -ljpeg -lX11

LDFLAGS=$(LIBS)

OBJ_FILE=file
OBJ_FX=fx
OBJ_GRAPHICS=graphics
OBJ_GUI=gui
OBJ_MAIN=main
OBJ_TOOLS=tools
OBJ= \
  graphics/Bitmap.o \
  graphics/Blend.o \
  main/Brush.o \
  gui/Button.o \
  gui/CheckBox.o \
  main/Clone.o \
  tools/Crop.o \
  main/Dialog.o \
  gui/DialogWindow.o \
  tools/Fill.o \
  fx/FX.o \
  file/File.o \
  file/FileSP.o \
  tools/GetColor.o \
  gui/Group.o \
  main/Gui.o \
  gui/InputFloat.o \
  gui/InputInt.o \
  gui/InputText.o \
  graphics/Map.o \
  main/Math.o \
  graphics/Octree.o \
  tools/Offset.o \
  tools/Paint.o \
  graphics/Palette.o \
  main/Project.o \
  graphics/Quantize.o \
  main/Render.o \
  gui/Separator.o \
  main/Stroke.o \
  tools/Text.o \
  gui/ToggleButton.o \
  fx/Transform.o \
  main/Undo.o \
  main/View.o \
  gui/Widget.o

default: $(OBJ)
	$(CXX) -o ./rendera main/Main.cxx $(OBJ) $(CXXFLAGS) $(LDFLAGS)

$(OBJ_FILE)/%.o: $(OBJ_FILE)/%.cxx $(OBJ_FILE)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_FX)/%.o: $(OBJ_FX)/%.cxx $(OBJ_FX)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_GRAPHICS)/%.o: $(OBJ_GRAPHICS)/%.cxx $(OBJ_GRAPHICS)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_GUI)/%.o: $(OBJ_GUI)/%.cxx $(OBJ_GUI)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_MAIN)/%.o: $(OBJ_MAIN)/%.cxx $(OBJ_MAIN)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_TOOLS)/%.o: $(OBJ_TOOLS)/%.cxx $(OBJ_TOOLS)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ_FILE)/*.o 
	@rm -f $(OBJ_FX)/*.o
	@rm -f $(OBJ_GRAPHICS)/*.o
	@rm -f $(OBJ_GUI)/*.o
	@rm -f $(OBJ_MAIN)/*.o
	@rm -f $(OBJ_TOOLS)/*.o
	@rm -f ./rendera 
	@echo "Clean."

