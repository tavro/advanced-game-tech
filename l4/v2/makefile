# set this variable to the director in which you saved the common files
commondir = ../common/

all: lab4.cpp $(commondir)LoadTGA.c SpriteLight.c $(commondir)VectorUtils4.h $(commondir)GL_utilities.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o lab4 -I$(commondir) $(commondir)LoadTGA.c $(commondir)GL_utilities.c $(commondir)Linux/MicroGlut.c SpriteLight.c lab4.cpp SimpleGUI.c -I../common/Linux -DGL_GLEXT_PROTOTYPES -lXt -lX11 -lGL -lm

clean:
	rm -f lab4

