all :  lab1-2

lab1-2: lab1-2.cpp ../common/GL_utilities.c ../common/VectorUtils4.h ../common/LoadTGA.c ../common/LittleOBJLoader.h ../common/Linux/MicroGlut.c
	g++ -Wall -o lab1-2 -DGL_GLEXT_PROTOTYPES lab1-2.cpp ../common/GL_utilities.c ../common/LoadTGA.c ../common/Linux/MicroGlut.c -I../common -I../common/Linux -lXt -lX11 -lm -lGL

clean :
	rm lab1-2

