all :  lab0

lab0: lab0.cpp ../common/GL_utilities.c ../common/VectorUtils4.h ../common/LoadTGA.c ../common/LittleOBJLoader.h ../common/Linux/MicroGlut.c
	g++ -Wall -o lab0 -DGL_GLEXT_PROTOTYPES lab0.cpp ../common/GL_utilities.c ../common/LoadTGA.c ../common/Linux/MicroGlut.c ../common/VectorUtils4.h -I../common -I../common/Linux -lm -lGL -lX11

clean :
	rm lab0

