# Makefile for dynamic linking on Linux

DIR_FILE=File
DIR_FX=FX
DIR_GRAPHICS=Graphics
DIR_GUI=Gui
DIR_MAIN=Main
DIR_TOOLS=Tools
DIR_IMAGES=Images

INCLUDE=-I$(DIR_FILE) -I$(DIR_FX) -I$(DIR_GRAPHICS) \
        -I$(DIR_GUI) -I$(DIR_MAIN) -I$(DIR_TOOLS) -I$(DIR_IMAGES)

CXX=g++

# obtain the current version number from git
NAME="Rendera "
VERSION=$(shell git describe --always --dirty)
CXXFLAGS=-O2 -DPACKAGE_STRING=\"$(NAME)$(VERSION)\" $(INCLUDE)

LIBS=-lfltk -lpng -ljpeg -lX11

LDFLAGS=$(LIBS)

OBJ= \
  $(DIR_FILE)/File.o \
  $(DIR_FILE)/FileSP.o \
  $(DIR_FX)/FX.o \
  $(DIR_FX)/Transform.o \
  $(DIR_GRAPHICS)/Bitmap.o \
  $(DIR_GRAPHICS)/Blend.o \
  $(DIR_GRAPHICS)/Map.o \
  $(DIR_GRAPHICS)/Octree.o \
  $(DIR_GRAPHICS)/Palette.o \
  $(DIR_GRAPHICS)/Quantize.o \
  $(DIR_GUI)/Button.o \
  $(DIR_GUI)/CheckBox.o \
  $(DIR_GUI)/DialogWindow.o \
  $(DIR_GUI)/Group.o \
  $(DIR_GUI)/InputFloat.o \
  $(DIR_GUI)/InputInt.o \
  $(DIR_GUI)/InputText.o \
  $(DIR_GUI)/Separator.o \
  $(DIR_GUI)/ToggleButton.o \
  $(DIR_GUI)/Widget.o \
  $(DIR_MAIN)/Brush.o \
  $(DIR_MAIN)/Clone.o \
  $(DIR_MAIN)/Dialog.o \
  $(DIR_MAIN)/Gui.o \
  $(DIR_MAIN)/Math.o \
  $(DIR_MAIN)/Project.o \
  $(DIR_MAIN)/Render.o \
  $(DIR_MAIN)/Stroke.o \
  $(DIR_MAIN)/Undo.o \
  $(DIR_MAIN)/View.o \
  $(DIR_TOOLS)/Crop.o \
  $(DIR_TOOLS)/Fill.o \
  $(DIR_TOOLS)/GetColor.o \
  $(DIR_TOOLS)/Offset.o \
  $(DIR_TOOLS)/Paint.o \
  $(DIR_TOOLS)/Text.o

default: $(OBJ)
	$(CXX) -o ./rendera $(DIR_MAIN)/Main.cxx $(OBJ) $(CXXFLAGS) $(LDFLAGS)

$(DIR_FILE)/%.o: $(DIR_FILE)/%.cxx $(DIR_FILE)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DIR_FX)/%.o: $(DIR_FX)/%.cxx $(DIR_FX)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DIR_GRAPHICS)/%.o: $(DIR_GRAPHICS)/%.cxx $(DIR_GRAPHICS)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DIR_GUI)/%.o: $(DIR_GUI)/%.cxx $(DIR_GUI)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DIR_MAIN)/%.o: $(DIR_MAIN)/%.cxx $(DIR_MAIN)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DIR_TOOLS)/%.o: $(DIR_TOOLS)/%.cxx $(DIR_TOOLS)/%.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(DIR_FILE)/*.o 
	@rm -f $(DIR_FX)/*.o
	@rm -f $(DIR_GRAPHICS)/*.o
	@rm -f $(DIR_GUI)/*.o
	@rm -f $(DIR_MAIN)/*.o
	@rm -f $(DIR_TOOLS)/*.o
	@rm -f ./rendera 
	@echo "Clean."

