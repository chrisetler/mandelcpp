# mandelcpp
A lightweight mandelbrot viewer written in C++ with freeglut and opgengl. OpenMP is also used for multithreading in computing the mandelbrot values. 

## To run
Make sure openGl and freeglut are installed. Those should be the only two dependencies. OpenMP is optional; without it the program will compile but will be limited to one thread.
If you are using linux and g++, then you can use the following makefile:
```
all:
	g++ -I/usr/include -L/usr/lib/x86_64-linux-gnu ./mandelcpp.cpp -lGL -lglut -lGLU  -lX11 -lm -lrt   -o main -fopenmp
```
You make have to tweak this a little if you're on a 32bit OS or don't have openMP installed.


## Controls

1. To move around, use the arrow keys
2. To zoom, use plus and minus
3. Note that the image is not automatically refreshed each time. This is because it takes a lot of computing power to do so. To refresh, press F5 or Space
4. To return to the original position, press 'r'
5. Use the number keys to select a color setting (need to refresh to see new colors)

## Examples

![Image One](http://i.imgur.com/LXX0BDn.png)
![Image Two](http://i.imgur.com/vBMwZhz.png)
