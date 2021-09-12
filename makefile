all:
	g++ -I/usr/include -L/usr/lib/x86_64-linux-gnu ./mandelcpp.cpp -lGL -lglut -lGLU  -lX11 -lm -lrt   -o main -fopenmp
