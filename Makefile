EXE = dbg.exe
CXX = g++
CXXFLAGS =-W -Wall -g
LIBS = capstone\capstone_x86.a -lPsapi
OBJ = projet.o breakpoint.o userInput.o Disassembler.o Display.o getNameFromHandle.o 

all: $(EXE) 

$(EXE): $(OBJ)
	$(CXX) -o $@ $^  $(LIBS)

%.o: %.c
	$(CXX) -o $@ -c $< $(CXXFLAGS)


clean:
	 del  $(EXE) *.o