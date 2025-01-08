commondir = ../common/

all: project.cpp $(commondir)LoadTGA.c $(commondir)VectorUtils4.h $(commondir)GL_utilities.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o project -I$(commondir) $(commondir)LoadTGA.c $(commondir)GL_utilities.c $(commondir)Linux/MicroGlut.c SimpleGUI.c project.cpp -I $(commondir)Linux -DGL_GLEXT_PROTOTYPES -lXt -lX11 -lGL -lm -lpthread -ldl -latomic

clean:
	rm -f project