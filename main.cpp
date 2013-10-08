#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <CL/CL.h>
#include <CL/cl_gl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "Camera.h"
#include "Timer.h"
#include "World.h"
#include "utils.h"

#undef main

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define WINDOW_CAPTION "noise"

using namespace std;
using namespace glm;

bool g_active = true;
bool g_done = false;
bool g_fullscreen = false;

bool g_coords = false;
bool g_polygonmode = false;

SDL_Window* mainwindow;
SDL_GLContext maincontext;

GLuint shaderProgram;
GLuint uModelViewProjectionMatrixLocation;
GLuint uModelViewMatrixLocation;

Timer timer;

World world;

cl_command_queue cmdqueue;
cl_kernel kernel;
cl_program program;
cl_context context;

mat4 projectionMatrix;

void resizeGLScene(int width, int height)
{
	cout << "Resize " << width << "x" << height << endl;

	if (height <= 0)
		height = 1;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	projectionMatrix = glm::perspective(45.0f, (float)width/(float)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

bool initGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.3f, 0.5f, 0.7f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_MULTISAMPLE);

	// Shaders
	GLuint vsMain = glCreateShader(GL_VERTEX_SHADER);
	GLuint fsMain = glCreateShader(GL_FRAGMENT_SHADER);

	string buffer;
	const char* source;
	if (!readFile("shaders/main.vert", buffer))
		return false;
	source = buffer.c_str();
	glShaderSource(vsMain, 1, (const char**)&source, nullptr);
	if (!readFile("shaders/main.frag", buffer))
		return false;
	source = buffer.c_str();
	glShaderSource(fsMain, 1, (const char**)&source, nullptr);

	glCompileShader(vsMain);
	glCompileShader(fsMain);

	// check shader status
	GLuint shaders[] = {vsMain, fsMain};
	for(GLuint shader : shaders)
	{
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

			char* infoLog = new char[length];
			glGetShaderInfoLog(shader, length, nullptr, infoLog);

			cout << infoLog << endl;
			delete[] infoLog;
			return false;
		}
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vsMain);
	glAttachShader(shaderProgram, fsMain);

	glLinkProgram(shaderProgram);

	// check program status
	{
		GLint status;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint length;
			glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);
			char* infoLog = new char[length];
			glGetProgramInfoLog(shaderProgram, length, nullptr, infoLog);

			cout << infoLog << endl;
			delete[] infoLog;
			return false;
		}
	}

	uModelViewProjectionMatrixLocation = glGetUniformLocation(shaderProgram, "uModelViewProjectionMatrix");
	uModelViewMatrixLocation = glGetUniformLocation(shaderProgram, "uModelViewMatrix");

	glUseProgram(shaderProgram);

	return true;
}

#define CHECK(error) if(error != CL_SUCCESS) { cout << "Error at line " << __LINE__ << ": " << error << endl; }

bool initCL()
{
	cl_int error;

	// get the first available platform
	cl_platform_id platform;
	error = clGetPlatformIDs(1, &platform, nullptr);
	CHECK(error);

	// get a GPU from this platform
	cl_device_id device;
	error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
	CHECK(error);

	// set properties to work with OpenGL (WINDOWS only) from: http://www.dyn-lab.com/articles/cl-gl.html
	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
		0};

	// create a context to work with OpenCL
	context = clCreateContext(properties, 1, &device, nullptr, nullptr, &error);
	CHECK(error);

	// read the kernel source
	string buffer;
	if(!readFile("kernels/main.cl", buffer))
		return false;
	const char* source = buffer.c_str();

	// create an OpenCL program from the source code
	program = clCreateProgramWithSource(context, 1, &source, nullptr, &error);
	CHECK(error);

	// build the program
	error = clBuildProgram(program, 1, &device, "-w", nullptr, nullptr);
	if(error != CL_SUCCESS)
	{
		cerr << "##### Error building CL program #####" << endl;

		// get the error log size
		size_t logSize;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);

		// allocate enough space and get the log
		char* log = new char[logSize + 1];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, nullptr);
		log[logSize] = '\0';

		// print the build log and delete it
		cerr << log << endl;
		delete[] log;

		return false;
	}

	// set the entry point for an OpenCL kernel
	kernel = clCreateKernel(program, "GenChunk", &error);
	CHECK(error);

	// create a new command queue, where kernels can be executed
	cmdqueue = clCreateCommandQueue(context, device, 0, &error);
	CHECK(error);

	return true;
}

void update(double interval)
{
	stringstream caption;
	caption << WINDOW_CAPTION << " @ " << fixed << setprecision(1) << timer.tps << " FPS";
	SDL_SetWindowTitle(mainwindow, caption.str().c_str());

	Camera::getInstance().update(interval);
	world.update();
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	Camera::getInstance().look();
	world.render();

	if(g_coords)
	{
		const int coordLength = 1000;

		glLineWidth(3.0f);
		glBegin(GL_LINES);
		glColor3f(1.0f,0.0f,0.0f); //red X+
		glVertex3i(coordLength,0,0);
		glVertex3i(0,0,0);
		glColor3f(0.0f,1.0f,0.0f); //green Y+
		glVertex3i(0,coordLength,0);
		glVertex3i(0,0,0);
		glColor3f(0.0f,0.0f,1.0f); //blue Z+
		glVertex3i(0,0,coordLength);
		glVertex3i(0,0,0);
		glEnd();

		glLineWidth(1.0f);
		glBegin(GL_LINES);
		glColor3f(0.0f,0.4f,0.0f); //green Y-
		glVertex3i(0,0,0);
		glVertex3i(0,-coordLength,0);
		glColor3f(0.4f,0.0f,0.0f); //red X-
		glVertex3i(0,0,0);
		glVertex3i(-coordLength,0,0);
		glColor3f(0.0f,0.0f,0.4f); //blue Z-
		glVertex3i(0,0,0);
		glVertex3i(0,0,-coordLength);
		glEnd();
	}
}

bool createSDLWindow(int width, int height)
{
	if(SDL_Init(SDL_INIT_VIDEO))
	{
		cerr << "Could not initialize SDL." << endl;
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// enable 2x anti-aliasing
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	unsigned int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
	if(g_fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;

	mainwindow = SDL_CreateWindow(WINDOW_CAPTION, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
	if (mainwindow == nullptr)
	{
		cerr << "Unable to create window" << endl;
		return false;
	}

	maincontext = SDL_GL_CreateContext(mainwindow);

	// GLEW
	GLenum error = glewInit();
	if(error != GLEW_OK)
	{
		cerr << "Could not initialize GLEW." << endl;
		return false;
	}

	return true;
}

void destroySDLWindow()
{
	SDL_GL_DeleteContext(maincontext);
	SDL_DestroyWindow(mainwindow);
	SDL_Quit();
}

void processEvent(SDL_Event event)
{
	switch(event.type)
	{
	case SDL_QUIT:
		g_done = true;
		return;
	case SDL_KEYDOWN:
		switch(event.key.keysym.sym)
		{
		case SDLK_c:
			g_coords = !g_coords;
			return;
		case SDLK_p:
			g_polygonmode = !g_polygonmode;
			if(g_polygonmode)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			return;
		case SDLK_ESCAPE:
			g_done = true;
			return;

		case SDLK_F1:
			SDL_Quit();
			g_fullscreen = !g_fullscreen;

			if (!createSDLWindow(WINDOW_WIDTH, WINDOW_HEIGHT))
				g_done = true;
			return;
		default:
			return;
		}
	case SDL_KEYUP:
		return;
	case SDL_MOUSEBUTTONDOWN:
		switch(event.button.button)
		{
		case SDL_BUTTON_RIGHT:
			Camera::getInstance().setCaptureMouse(true);
			//SDL_ShowCursor(SDL_DISABLE);
			return;
		default:
			return;
		}
	case SDL_MOUSEBUTTONUP:
		switch(event.button.button)
		{
		case SDL_BUTTON_RIGHT:
			Camera::getInstance().setCaptureMouse(false);
			//SDL_ShowCursor(SDL_ENABLE);
			return;
		default:
			return;
		}
	case SDL_WINDOWEVENT:
		switch(event.window.event)
		{
		case SDL_WINDOWEVENT_RESIZED:
			resizeGLScene(event.window.data1, event.window.data2);
			return;
		default:
			return;
		}
	default:
		return;
	}
}

int main(int argc, char **argv)
{
	if (!createSDLWindow(WINDOW_WIDTH, WINDOW_HEIGHT))
		return -1;

	if(!initGL())
		return -1;
	//if(!initCL())
	//	return -1;

	resizeGLScene(WINDOW_WIDTH, WINDOW_HEIGHT);

	while (!g_done)
	{
		SDL_Event event;
		if (SDL_PollEvent(&event))
			processEvent(event);
		else if (g_active)
		{
			timer.tick();
			update(timer.interval);
			render();
			SDL_GL_SwapWindow(mainwindow);
		}
	}

	destroySDLWindow();

	return 0;
}
