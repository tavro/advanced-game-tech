all :  lab1-1

lab1-1: lab1-1.cpp ../common/GL_utilities.c ../common/LoadTGA.c ../common/Linux/MicroGlut.c ../common/VectorUtils4.h ../common/LittleOBJLoader.h
	g++ -Wall -o lab1-1 -DGL_GLEXT_PROTOTYPES lab1-1.cpp ../common/GL_utilities.c ../common/LoadTGA.c ../common/Linux/MicroGlut.c -I../common -I../common/Linux -lXt -lX11 -lm -lGL

clean :
	rm lab1-1

