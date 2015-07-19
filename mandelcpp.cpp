#include <iostream>
#include <Windows.h>
#include <string>
#define NDEBUG 0;
#include "freeglut.h"
#include <math.h>

/*LIST OF FUNCTIONS*/

/*Keyboard Control:
	Program is event-driven, so stuff is only re-drawn/ computed when keys are pressed.
	Controls: Pan - up,down,left,right; Zoom: +/-; Colors Modes: 0-6; Render: space of F5; reset to default: r
*/
void keyPressed(unsigned char key, int x, int y);
void keySpecial(int key, int x, int y);

/*The following functions redraw the screen. Each relies on a separte function to create the new pixel mapping*/
void refreshAndRenderMandelbrot();
void refreshAndRenderZoom(double zoomAmount);
void refreshAndRenderPan(int direction, int offset);

/*These functions work as a backend to the above functions to create the new pixel array that is rendered in each*/
/*
	The pointer to the pixel array is stored in **pixel_loc. The pixel array should be in RGB format and uses the glDrawPixels method.
	The method that draws the pixels is glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, *pixel_loc);
	So whatever is stored to the pixel location by these functions must be compatible with that
*/
void drawMandelbrotToPixelArr();
void drawZoomToPixelArr(double change);
void drawPanToPixelArr(int direction, int offset);

/*A new refresh and render function should be set up with the following functions in order:*/
void updateViewportIfScreenChanged(); //to allow for changes in window size
void clearBuffer(); //to allow the rendering of a new image
//one of the draw to pixel array functions, to store the new image
void RenderSceneCB(); //to draw the new image

//The following functions are used to generate the modelbrot set and the corresponding color values
double mandel(double real, double imaginary); //returns the count of how long it took to become unbounded at that point
int get_color(int x, int y); //gets the color at a specific point in the window by converting x and y into the mandelbrot coordinates. The color will depend on the color mode set. 
int hsv_to_rgb(double h); //used to convert a hue value to an RGB value (saturation and value are set to 100%)

//other functions
void resetToDefaultCoordinates();

/*END LIST OF FUNCTIONS*/

/*INITIAL VALUES AND CONSTANTS*/
//screen size (anything bigger makes it run rather slow)
int height = 480;
int width = 640;

/*When ever the screen is updated, the new pixels will be stored here, and then drawn*/
char** pixel_loc = (char**)malloc(sizeof(char*));

//Directionals for panning
const int UP = 0;
const int DOWN = 1;
const int LEFT = 3;
const int RIGHT = 2;

//color modes: 0-rainbow, 1-blue, 2-red, 3-green, 4-grayscale, 5- bw
int color_mode = 0;

//starting position and magnification
double const DEF_STEP = .005;
double const DEF_REAL = -0.7;
double const DEF_IM = 0;
double step_size = DEF_STEP;
double real_corner = DEF_REAL;
double im_corner = DEF_IM;
int max_count = 340; //staring max count, will change based on zoom level
//alternate starting pos
//double step_size = 0.000169328;
//double real_corner = -1.25248;
//double im_corner = 0.384268;


/*GLUT INITIALIZATION*/
static void InitializeGlutCallbacks()
{
	//do a refresh to start
	refreshAndRenderMandelbrot();
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

	//program is event-driven
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(keySpecial);


	glutMainLoop();
	return 0;
}


//controls what happens when keys are pressed
//the mandelbrot is only rendered when the 'r' key, or space key, is pressed
void keyPressed(unsigned char key, int x, int y) {
	//changing the color has no immediate effect
	if (key == '1') color_mode = 0;
	else if (key == '2') color_mode = 1;
	else if (key == '3') color_mode = 2;
	else if (key == '4') color_mode = 3;
	else if (key == '5') color_mode = 4;
	else if (key == '6') color_mode = 5;

	//zooming in an out performs a raster zoom -- much quicker than re-rendering but still too slow when 1080p
	else if (key == '+') {
		step_size *= .9;
		refreshAndRenderZoom(1 / 0.9);
	}
	else if (key == '-') {
		step_size *= 1.1;
		refreshAndRenderZoom(1 / 1.1);
	}
	//refresh -- render the MandelBrot
	else if (key == ' ') {
		refreshAndRenderMandelbrot();
	}
	//resets to default position and re-renders
	else if (key == 'r') {
		resetToDefaultCoordinates();
		refreshAndRenderMandelbrot();
	}
}

//for when special keys are pressed
void keySpecial(int key, int x, int y) {
	//panning does not re-render - simply adds a block border where new mandelbrot will go
	if (key == GLUT_KEY_LEFT) {
		real_corner -= abs(step_size * 20);
		refreshAndRenderPan(LEFT, 20);

	}
	else if (key == GLUT_KEY_RIGHT) {
		real_corner += abs(step_size * 20);
		refreshAndRenderPan(RIGHT, 20);

	}
	else if (key == GLUT_KEY_DOWN) {
		im_corner -= abs(step_size * 20);
		refreshAndRenderPan(DOWN, 20);

	}
	else if (key == GLUT_KEY_UP) {
		im_corner += abs(step_size * 20);
		refreshAndRenderPan(UP, 20);

	}
	else if (key == GLUT_KEY_F5) refreshAndRenderMandelbrot();
}

//Recomputes the mandelbrot and renders it to the screen
void refreshAndRenderMandelbrot() {
	updateViewportIfScreenChanged();
	clearBuffer();
	drawMandelbrotToPixelArr();
	RenderSceneCB();
}

//Zooms the image in or out and renders it to the screen
void refreshAndRenderZoom(double zoomAmount) {
	updateViewportIfScreenChanged();
	clearBuffer();
	drawZoomToPixelArr(zoomAmount);
	RenderSceneCB();
}

//Pans the image and renders it too the screen
void refreshAndRenderPan(int direction, int offset) {
	updateViewportIfScreenChanged();
	clearBuffer();
	drawPanToPixelArr(direction, offset);
	RenderSceneCB();
}


//Draw the pixels to the screen
void RenderSceneCB()
{
	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, *pixel_loc);
	glFlush();
	std::cout << "Position: " << real_corner << " + " << im_corner << "i " << "Step Size: " << step_size << "Max iterations : " << max_count << "\n";
}


//update the viewport
void updateViewportIfScreenChanged() {
	//adjust the screen size if changed
	//width must be an integer multiple of 4.
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	if (width != m_viewport[2] / 4 * 4 + 4 || height != m_viewport[3]) {
		width = (m_viewport[2] / 4) * 4 + 4;
		height = m_viewport[3];
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, width, 0, height, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glRasterPos2i(0, 0);
	}
}

void clearBuffer() {
	glClear(GL_COLOR_BUFFER_BIT);
}

void resetToDefaultCoordinates() {
	real_corner = DEF_REAL;
	im_corner = DEF_IM;
	step_size = DEF_STEP;
}


//draws the mandelbrot to the pixel array
void drawMandelbrotToPixelArr() {
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
}

//zoom by change
//change > 1 means zoom in
//change < 1 means zoom out
//does so by creating a bitmap from the old pixels rather than re rendering -- makes it pixelated but at least gives a preview
//"zoomed" pixels are saved to the pixel array
void drawZoomToPixelArr(double change) {
	//create a double array from the pixel data
	char* new_array = (char*)malloc(sizeof(char)*width*height * 3);
	char* pixel = *pixel_loc;

	//For zooming in:
	if (change >= 1) {
		/*Get the offsets needed to zoom around the center of the old image*/
		int x_offset = round((width*(1 - 1 / change) / 2.0));
		int y_offset = round((height*(1 - 1 / change) / 2.0));
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				//zoom in
				//get corresponding array locations for the old and new
				int new_array_loc = 3 * (j*(width)+i);
				int old_i = round(((i / change) + x_offset) + round(change / 2) - 1); //+ 
				int old_j = round(((j / change) + y_offset) + round(change / 2) - 1);
				int old_array_loc = 3 * (old_j*(width)+old_i);
				if (old_array_loc > width*height * 3) old_array_loc = 0;
				new_array[new_array_loc] = pixel[old_array_loc];
				new_array[new_array_loc + 1] = pixel[old_array_loc + 1];
				new_array[new_array_loc + 2] = pixel[old_array_loc + 2];

			}
		}
	}

	//For zooming out:
	else {
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				/*Zoom out. Center the current data and add a black background around it*/
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
					new_array[new_loc + 1] = pixel[old_loc + 1];
					new_array[new_loc + 2] = pixel[old_loc + 2];
				}
			}
		}
	}

	//Set the pixel location to the new pixels
	*pixel_loc = new_array;
	free(pixel);
}

/*pan over without re computing fractal. */
void drawPanToPixelArr(int direction, int offset) {

	char* new_array = (char*)malloc(sizeof(char)*width*height * 3);
	char* pixel = *pixel_loc;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			int old_j = j;
			int old_i = i;
			switch (direction){
			case UP:
				old_j += offset;
				break;
			case DOWN:
				old_j -= offset;
				break;
			case RIGHT:
				old_i += offset;
				break;
			case LEFT:
				old_i -= offset;
				break;
			}

			//re-compute pixel values
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

	//Set pixel pointer to new location
	free(pixel);
	*pixel_loc = new_array;

}



//The Mandelbrot Alogrithm - retruns the count of times it takes for the bound to diverge, or 0 if it does not
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

//get the color for a specific pixel in the fractal
//color is based on the number of iterations required to determine if the point was unbounded
int get_color(int x, int y) {
	double x_pos = real_corner - (width*step_size) / 2 + x*step_size;
	double y_pos = im_corner - (height*step_size) / 2 + y*step_size;
	double bound = mandel(x_pos, y_pos);

	if (bound == 0) return 0;
	if (color_mode == 5) return 0xffffff;//skip all the nonsense for b&w mode - returns #FFFFFFF

	//color intensity values for the single color color modes (not rainbow mode)
	//int color = (int)((log((double)bound) / log((double)max_count))*255.0);
	int color = (int)round(bound * 255 / max_count);

	//color modes: 0-rainbow, 1-blue, 2-red, 3-green, 4-grayscale, 5-b&w
	if (color_mode == 0) {
		int color_rgb = hsv_to_rgb(bound * 360.0 / (double)max_count);
		return color_rgb;
	}
	else if (color_mode == 1) {
		//non-hsv
		//return color;
		//hsv
		int color_rgb = hsv_to_rgb(fmod((bound*360.0 / (double)(max_count)+180), 360));
		return color_rgb;
	}
	else if (color_mode == 2) {
		//non-hsv
		//return color << 16;
		//hsv
		int color_rgb = hsv_to_rgb(fmod((bound*360.0 / (double)(max_count)+240), 360));
		return color_rgb;
	}
	else if (color_mode == 3) {
		//non-hsv
		//return color <<8;
		//hsv
		int color_rgb = hsv_to_rgb((bound*360.0 / (double)(max_count)+60));
		return color_rgb;
	}
	else if (color_mode == 4) {
		return (color << 16) + (color << 8) + color;
	}
	return 0;
}

//Takes in a hue and returns RGB, with value and saturation both at 100%
//Used for saving the colors to a bitmap to be rendered
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



