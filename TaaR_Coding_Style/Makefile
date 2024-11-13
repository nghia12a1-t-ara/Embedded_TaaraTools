######### C++ Makefile for Tools Coding Convension ###########
######### TaaR_Coding_Style ###############
######### Author : Nghia Taarabt ##########
######### Created Date : 06/08/2023 #######
PROJECT = .

# list of source files for astyle
SRC_FILES = main.cpp \
			ASBeautifier.cpp \
			TRFormatter.cpp \
			ASEnhancer.cpp \
			TRLanguage.cpp \
			TRResource.cpp

# source directories
vpath %.cpp $(PROJECT)/src
vpath %.h   $(PROJECT)/src

# define macros
# CXX defaults to g++
CXX = g++
BIN_DIR = bin
OBJ_DIRS = obj
INC_DIRS = $(PROJECT)/src
CXXFLAGS = 	-Wall -Wextra -fno-rtti -fno-exceptions -std=c++17 \
			-I$(INC_DIRS) \
			-D_WIN32 -DWIN32_LEAN_AND_MEAN -DNOMINMAX -UUNICODE \
			-DNDEBUG

# object files are built from the source list $(SRC_FILES)
OBJ_FILES = $(patsubst %.cpp,$(OBJ_DIRS)/%.o,$(SRC_FILES))

.PHONY: main clean $(OBJ_DIRS)/%.o

# define object file rule (with the suffix) for each build
$(OBJ_DIRS)/%.o:  %.cpp
	@ mkdir -p $(OBJ_DIRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Target to build the release version
main: $(OBJ_FILES)
	@ mkdir -p $(BIN_DIR)
	$(CXX) -o $(BIN_DIR)/$@ $^
	@ echo "Build Successfully!... >>> $(PROJECT)/$(BIN_DIR)/main.exe"

clean:
	rm -f $(OBJ_DIRS)/*.o $(BIN_DIR)/*.exe

print-%:
	@echo $($(subst print-,,$@))
