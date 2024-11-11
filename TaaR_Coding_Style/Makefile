######### C++ Makefile for Tools Coding Convension ###########
######### TaaR_Coding_Style ###############
######### Author : Nghia Taarabt ##########
######### Created Date : 06/08/2023 #######

# list of source files for astyle
SRC = astyle_main.cpp \
      ASBeautifier.cpp \
      ASFormatter.cpp \
      ASEnhancer.cpp \
      ASLocalizer.cpp \
      ASResource.cpp

# source directories
vpath %.cpp ./src
vpath %.h   ./src

# set prefix if not defined on the command line
ifndef prefix
    prefix=/usr
endif

# define macros
# CXX defaults to g++
CXX = g++
bindir = bin
objdir = obj
ipath=$(prefix)/bin
CBASEFLAGS = -Wall -Wextra -fno-rtti -fno-exceptions -std=c++17
INSTALL=install -o $(USER) -g $(USER)

# define compile options for each build
ifdef CFLAGS
    CFLAGSr   = -DNDEBUG $(CBASEFLAGS) $(CFLAGS)
else
    CFLAGSr   = -DNDEBUG -O3 $(CBASEFLAGS)
endif

# object files are built from the source list $(SRC)
OBJ   = $(patsubst %.cpp,$(objdir)/%.o,$(SRC))

# define object file rule (with the suffix) for each build
$(objdir)/%.o:  %.cpp  astyle.h  astyle_main.h
	@ mkdir -p $(objdir)
	$(CXX) $(CFLAGSr) -c $< -o $@

##################################################
# define build dependencies for each command

# Target to build the release version of astyle
release:  astyle
astyle:  $(OBJ)
	@ mkdir -p $(bindir)
	$(CXX) $(LDFLAGSr) -o $(bindir)/$@ $^
	@ echo

clean:
	rm -f $(objdir)/*.o $(bindir)/astyle*

install:
	$(INSTALL) -m 755 -d $(ipath)
	@$(INSTALL) -m 755 $(bindir)/astyle  $(ipath)

uninstall:
	rm -f $(ipath)/astyle
