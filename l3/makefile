# set this variable to the director in which you saved the common files
commondir = ../common/

all : lab3

lab3 : lab3.cpp $(commondir)GL_utilities.c $(commondir)VectorUtils4.h $(commondir)LittleOBJLoader.h $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o lab3 -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES lab3.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

clean :
	rm lab3

