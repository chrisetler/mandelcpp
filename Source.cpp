// my first program in C++
#include <iostream>
#include <Windows.h>
#include <string>
#define NDEBUG 0;
#include "freeglut.h"
#include <math.h>



//functions
void set_pixel(int x_pos, int y_pos, int R, int G, int B);
void render_screen();
void keyPressed(unsigned char key, int x, int y);
void keySpecial(int key, int x, int y);
void zoom(double change, char* pixel);
void pan(int direction, int offset);

//screen size
int height = 480;
int width = 640;

//location of pixel data
char** pixel_loc = (char**)malloc(sizeof(char*));


//color modes: 0-rainbow, 1-blue, 2-red, 3-green, 4-grayscale, 5- bw
int color_mode = 0;

//starting pos.
double const DEF_STEP = .005;
double const DEF_REAL = -0.7;
double const DEF_IM = 0;
double step_size = DEF_STEP;
double real_corner = DEF_REAL;
double im_corner = DEF_IM;
int max_count = 340;
//double step_size = 0.000169328;
//double real_corner = -1.25248;
//double im_corner = 0.384268;

boolean size_has_changed = false;

void RenderSceneCB()
{
	
	//adjust the screen size if changed
	//width must be an integer multiple of 4.
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	if (width != m_viewport[2] / 4 * 4 + 4 || height != m_viewport[3]) {
		width = (m_viewport[2] / 4) * 4 + 4;
		height = m_viewport[3];
		size_has_changed = true;
	}

	//clear the buffer out
	glClear(GL_COLOR_BUFFER_BIT);
	render_screen();
	glFlush();
	std::cout << "Position: " << real_corner << " + " << im_corner << "i " << "Step Size: " << step_size << "Max iterations : " << max_count << "\n";
}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(RenderSceneCB);
}


int main(int argc, char** argv)
{
	//GLUT Setup
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	//set screen size and viewport
	glutInitWindowSize(width, height);
	glViewport(0, 0, width, height);
	glutInitWindowPosition(100, 100);
	//create the window
	glutCreateWindow("mandelbtoglcpp");
	//init the program
	InitializeGlutCallbacks();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(keySpecial);
	glutMainLoop();

	return 0;
}

void set_pixel(int x_pos, int y_pos, int R, int G, int B) {
	int* pixel = (int*)malloc(sizeof(int));
	*pixel = 255 << 16;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width, 0, height, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRasterPos2i(0, 0);
	glDrawPixels(1, 1, GL_RGB, GL_UNSIGNED_INT, pixel);
}

//Mandelbrot Recursive Alogrithm
double mandel(double real, double imaginary) {
	//return the number of iterations required to unbound the function
	//zero means it remains unbounded
	//mandelbrot at real + imaginary(i)
	int count = 0;
	double z = 0;
	double oldz = 0;
	double rl = 0;
	double im = 0;
	bool isBound = false;
	while (z < 2 && count < max_count) {
		//while (count < 50){
		//get the new real and imaginary values for zn+1
		double oldz = z;
		double a = rl;
		double b = im;
		rl = a*a - b*b + real;
		im = 2 * a * b + imaginary;
		z = sqrt(rl*rl + im*im);
		count++;
	}
	if (z < 2) {
		return 0;
	}
	return (double)count;
}

//use HSV values for rainbow color scheme
int hsv_to_rgb(double h) {
	double s = 1.0;
	double v = 1.0;
	double c = v*s;

	double x = c*(1 - abs(fmod((h / 60), 2) - 1));
	double m = v - c;
	double r_prime, g_prime, b_prime;

	if (h < 60) {
		r_prime = c;
		g_prime = x;
		b_prime = 0.0;
	}
	else if (h < 120) {
		r_prime = x;
		g_prime = c;
		b_prime = 0.0;
	}
	else if (h < 180) {
		r_prime = 0;
		g_prime = c;
		b_prime = x;
	}
	else if (h < 240) {
		r_prime = x;
		g_prime = 0.0;
		b_prime = c;
	}
	else {
		r_prime = c;
		g_prime = 0.0;
		b_prime = x;
	}
	int r = (int)((r_prime + m) *255.0) & 255;
	int g = (int)((g_prime + m) *255.0) & 255;
	int b = (int)((b_prime + m) *255.0) & 255;
	return (r << 16) + (g << 8) + (b);
}

//get the color for a specific pixel in the fractal
int get_color(int x, int y) {
	double x_pos = real_corner - (width*step_size) / 2 + x*step_size;
	double y_pos = im_corner - (height*step_size) / 2 + y*step_size;
	double bound = mandel(x_pos, y_pos);

	if (bound == 0) return 0;
	if (color_mode == 5) return 0xffffff;//skip all the nonsense for b&w mode - returns #FFFFFFF

	//color intensity values for the single color color modes (not rainbow mode)
	//int color = (int)((log((double)bound) / log((double)max_count))*255.0);
	int color = (int)round(bound* 255 / max_count);

	//color modes: 0-rainbow, 1-blue, 2-red, 3-green, 4-grayscale, 5-b&w
	if (color_mode == 0) {
		int color_rgb = hsv_to_rgb(bound * 360.0 / (double)max_count);
		return color_rgb;
	}
	else if (color_mode == 1) {
		//non-hsv
		//return color;
		//hsv
		int color_rgb = hsv_to_rgb(bound*120.0 / (double)(max_count)+180);
		return color_rgb;
	}
	else if (color_mode == 2) {
		//non-hsv
		//return color << 16;
		//hsv
		int color_rgb = hsv_to_rgb(bound*60.0 / (double)(max_count)+240);
		return color_rgb;
	}
	else if (color_mode == 3) {
		//non-hsv
		//return color <<8;
		//hsv
		int color_rgb = hsv_to_rgb(bound*60.0 / (double)(max_count)+60);
		return color_rgb;
	}
	else if (color_mode == 4) {
		return (color << 16) + (color << 8) + color;
	}	
	
	return 0;
}

//render the pixels on the screen by running the mandelbrot algo
void render_screen() {
	if (size_has_changed) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, width, 0, height, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glRasterPos2i(0, 0);
	}
	size_has_changed = false;
	max_count = round(sqrt(abs(2 * sqrt(abs(1 - sqrt(5 * 1 / step_size)))))*66.5);

	char* pixel = (char*)malloc(sizeof(char)*width*height * 3);
	for (int i = 0; i < width; i++){
		for (int j = 0; j < height; j++) {
			int color = get_color(i, j);
			pixel[3 * (j*(width)+i)] = ((color >> 16) & 255);
			pixel[3 * (j*(width)+i) + 1] = ((color >> 8) & 255);
			pixel[3 * (j*(width)+i) + 2] = ((color)& 255);
		}
	}
	glRasterPos2i(0, 0);
	*pixel_loc = pixel;
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	glFlush();
}

void keyPressed(unsigned char key, int x, int y) {
	if (key == '1') color_mode = 0;
	else if (key == '2') color_mode = 1;
	else if (key == '3') color_mode = 2;
	else if (key == '4') color_mode = 3;
	else if (key == '5') color_mode = 4;
	else if (key == '6') color_mode = 5;
	else if (key == '+') {
		step_size *= .9;
		zoom(1/.9, *pixel_loc);
	}
	else if (key == '-') {
		step_size *= 1.1;
		zoom(1/1.1, *pixel_loc);
	}
	else if (key == ' ') {
		keySpecial(GLUT_KEY_F5, x, y);
	}
	else if (key == 'r') {
		real_corner = DEF_REAL;
		im_corner = DEF_IM;
		step_size = DEF_STEP;
		RenderSceneCB();
	}

	//RenderSceneCB();

}

void keySpecial(int key, int x, int y) {
	boolean update = true;
	if (key == GLUT_KEY_LEFT) {
		real_corner -= abs(step_size * 20);
		pan(3, 20);
		update = false;
	}
	else if (key == GLUT_KEY_RIGHT) {
		real_corner += abs(step_size * 20);
		pan(2, 20);
		update = false;
	}
	else if (key == GLUT_KEY_DOWN) {
		im_corner -= abs(step_size * 20);
		pan(1, 20);
		update = false;
	}
	else if (key == GLUT_KEY_UP) {
		im_corner += abs(step_size * 20);
		pan(0, 20);
		update = false;
	}
	else if (key == GLUT_KEY_F5);
	else (update = false);
	if (update) RenderSceneCB();

}

//zoom in by change
//change > 1 means zoom in
//change < 1 means zoom out
//does so by creating a bitmap from the old pixels rather than re rendering -- makes it pixelated but at least gives a preview
void zoom(double change, char* pixel) {
	//create a double array from the pixel data
	char* new_array = (char*)malloc(sizeof(char)*width*height * 3);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			//zoom in
			if (change >= 1) {
				/*Get the offsets needed to zoom around the center of the old image*/
				int x_offset = round((width*(1 - 1 / change) / 2.0));
				int y_offset = round((height*(1 - 1 / change) / 2.0));

				//get corresponding array locations for the old and new
				int new_array_loc = 3 * (j*(width)+i);
				int old_i = round(((i / change) + x_offset ) + round(change / 2)-1); //+ 
				int old_j = round(((j / change) + y_offset ) + round(change / 2)-1);
				int old_array_loc = 3 * (old_j*(width)+old_i);
				if (old_array_loc > width*height * 3) old_array_loc = 0;
				new_array[new_array_loc] = pixel[old_array_loc];
				new_array[new_array_loc + 1] = pixel[old_array_loc + 1];
				new_array[new_array_loc + 2] = pixel[old_array_loc + 2];

			}
			else {
				/*Zoom out. Center the current data and add a black background around it*/
				//make things centered 
				int x_offset = round(width*(1 - change) / 2);
				int y_offset = round(height*(1 - change) / 2);
				//get the corresponding array locations
				int new_loc = 3 * (j*width + i);
				if (i < x_offset || i >= (width - x_offset) || j < y_offset || j >= (height - y_offset)) {
					new_array[new_loc] = 0;
					new_array[new_loc + 1] = 0;
					new_array[new_loc + 2] = 0;
				}
				else {
					int old_i = (int)round((i - x_offset) / change);
					int old_j = (int)round((j - y_offset) / change);
					int old_loc = 3 * (old_j*width + old_i);
					new_array[new_loc] = pixel[old_loc];
					new_array[new_loc +1] = pixel[old_loc +1];
					new_array[new_loc +2] = pixel[old_loc +2];
				}
			}




		}
	}
	/*Render it and save the location of the new pixel data, and clear the old*/
	glRasterPos2i(0, 0);
	*pixel_loc = new_array;
	free(pixel);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, new_array);
	glFlush();
}

void pan(int direction, int offset) {
	/*pan over without re computing fractal. */
	/*0- up, 1- down, 3- left, 2-right*/
	char* new_array = (char*)malloc(sizeof(char)*width*height * 3);
	char* pixel = *pixel_loc;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			int old_j = j;
			int old_i = i;
			switch (direction){
				/*UP*/
			case 0:
				old_j += offset;
				break;
				/*DOWN*/
			case 1:
				old_j -= offset;
				break;
				/*RIGHT*/
			case 2:
				old_i += offset;
				break;
				/*LEFT*/
			case 3:
				old_i -= offset;
				break;
			}

			int new_loc = 3 * (j*width + i);
			int old_loc = 3 * (old_j*width + old_i);
			if (old_j < 0 || old_i < 0 || old_j >= height || old_i >= width) {
				new_array[new_loc] = 0;
				new_array[new_loc + 1] = 0;
				new_array[new_loc + 2] = 0;
			}
			else {
				new_array[new_loc] = pixel[old_loc];
				new_array[new_loc + 1] = pixel[old_loc + 1];
				new_array[new_loc + 2] = pixel[old_loc + 2];
			}
		}

	}

	/*Render it*/
	glRasterPos2i(0, 0);
	free(pixel);
	*pixel_loc = new_array;
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, new_array);
	glFlush();
}