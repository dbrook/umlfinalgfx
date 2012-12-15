##
## Makefile to compile the final Graphics 1 project (Fall 2012 semester)
##
## Author: Dan Brook
##

GCC=-pedantic
LINKS=-lGLEW -lglut -lGL -lXmu -lX11 -lm -lassimp

all: finalproj

finalproj: angel/InitShader.o finalproj.o asset.o
	g++ $(GCC) $(LINKS) $^ -o final

finalproj.o: finalproj.cpp
	g++ $(GCC) -c $^ -o finalproj.o
	
asset.o: asset.hpp asset.cpp

angel/InitShader.o:
	g++ $(GCC) -c angel/InitShader.cpp -o angel/InitShader.o $(LINKS)

clean:
	rm -rf *~ *.o angel/InitShader.o final
