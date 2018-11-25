#include "gl.h"
#include <GLFW/glfw3.h>
#include <CL/CL.h>
#include <CL/cl_gl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>

#include "Shader.h"
#include "Program.h"
#include "ThreadedCommandConsole.h"
#include "Camera.h"
#include "Timer.h"
#include "World.h"
#include "utils.h"
#include "globals.h"

#undef main

using namespace std;
using namespace glm;

const static unsigned int initialWindowWidth = 1024;
const static unsigned int initialWindowHeight = 768;
const static string windowCaption = "noise";

static bool g_active = true;
static bool g_fullscreen = false;
bool captureMouse = false;
glm::dvec2 mouseDownPos;

GLFWwindow* mainwindow;

gl::Program shaderProgram;
gl::Program normalDebuggingProgram;
GLint uViewProjectionMatrixLocation;
GLint uViewProjectionMatrixLocationNormalDebuggingProgram;
GLint uViewMatrixLocation;

Timer timer;
World world;

cl_command_queue cmdqueue;
cl_kernel kernel;
cl_program program;
cl_context context;

mat4 projectionMatrix;

void resizeGLScene(GLFWwindow*, int width, int height)
{
	cout << "Resize " << width << "x" << height << endl;

	if (height <= 0)
		height = 1;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	projectionMatrix = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

bool initGL()
{
	glClearColor(0.3f, 0.5f, 0.7f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_MULTISAMPLE);

	// Shaders
	swap(shaderProgram, gl::Program(
	{
		gl::Shader(GL_VERTEX_SHADER, "shaders/main.vert"),
		gl::Shader(GL_FRAGMENT_SHADER, "shaders/main.frag")
	}));
	swap(normalDebuggingProgram, gl::Program(
	{
		gl::Shader(GL_VERTEX_SHADER, "shaders/normals.vert"),
		gl::Shader(GL_GEOMETRY_SHADER, "shaders/normals.geom"),
		gl::Shader(GL_FRAGMENT_SHADER, "shaders/normals.frag")
	}));

	uViewProjectionMatrixLocation = shaderProgram.getUniformLocation("uViewProjectionMatrix");
	uViewMatrixLocation = shaderProgram.getUniformLocation("uViewMatrix");
	uViewProjectionMatrixLocationNormalDebuggingProgram = normalDebuggingProgram.getUniformLocation("uViewProjectionMatrix");

	shaderProgram.use();

	return true;
}

void update(double interval)
{
	stringstream caption;
	caption << windowCaption << " @ " << fixed << setprecision(1) << timer.tps << " FPS";
	glfwSetWindowTitle(mainwindow, caption.str().c_str());

	// camera
	glm::dvec2 delta{ 0, 0 };
	if (captureMouse) {
		glm::dvec2 pos;
		glfwGetCursorPos(mainwindow, &pos.x, &pos.y);
		delta = mouseDownPos - pos;
		glfwSetCursorPos(mainwindow, mouseDownPos.x, mouseDownPos.y);
	}

	uint8_t moveFlags = 0;
	if (glfwGetKey(mainwindow, GLFW_KEY_SPACE) == GLFW_PRESS) moveFlags |= Up;
	if (glfwGetKey(mainwindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) moveFlags |= Down;
	if (glfwGetKey(mainwindow, GLFW_KEY_W) == GLFW_PRESS) moveFlags |= Forward;
	if (glfwGetKey(mainwindow, GLFW_KEY_S) == GLFW_PRESS) moveFlags |= Backward;
	if (glfwGetKey(mainwindow, GLFW_KEY_A) == GLFW_PRESS) moveFlags |= Left;
	if (glfwGetKey(mainwindow, GLFW_KEY_D) == GLFW_PRESS) moveFlags |= Right;

	camera.update(interval, delta.x, delta.y, moveFlags);

	world.update();
}

void render()
{
	// clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// update shader states
	mat4 viewMatrix = camera.viewMatrix();
	mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

	glUniformMatrix4fv(uViewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
	glUniformMatrix4fv(uViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// render the world
	world.render();

	// render coordinate system
	if (global::coords)
	{
		const int coordLength = 1000;

		glLineWidth(3.0f);
		glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f); //red X+
		glVertex3i(coordLength, 0, 0);
		glVertex3i(0, 0, 0);
		glColor3f(0.0f, 1.0f, 0.0f); //green Y+
		glVertex3i(0, coordLength, 0);
		glVertex3i(0, 0, 0);
		glColor3f(0.0f, 0.0f, 1.0f); //blue Z+
		glVertex3i(0, 0, coordLength);
		glVertex3i(0, 0, 0);
		glEnd();

		glLineWidth(1.0f);
		glBegin(GL_LINES);
		glColor3f(0.0f, 0.4f, 0.0f); //green Y-
		glVertex3i(0, 0, 0);
		glVertex3i(0, -coordLength, 0);
		glColor3f(0.4f, 0.0f, 0.0f); //red X-
		glVertex3i(0, 0, 0);
		glVertex3i(-coordLength, 0, 0);
		glColor3f(0.0f, 0.0f, 0.4f); //blue Z-
		glVertex3i(0, 0, 0);
		glVertex3i(0, 0, -coordLength);
		glEnd();
	}

	// render normals
	if (global::normals)
	{
		normalDebuggingProgram.use();
		glUniformMatrix4fv(uViewProjectionMatrixLocationNormalDebuggingProgram, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		world.render();
		shaderProgram.use();
	}
}

void onMouseButton(GLFWwindow*, int button, int action, int modifiers) {
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_RIGHT:
			captureMouse = true;
			glfwGetCursorPos(mainwindow, &mouseDownPos.x, &mouseDownPos.y);
			glfwSetInputMode(mainwindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			break;
		}
	}

	if (action == GLFW_RELEASE) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_RIGHT:
			captureMouse = false;
			glfwSetInputMode(mainwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		}
	}
}

void onKey(GLFWwindow*, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {

		}
	}
}

bool createSDLWindow(int width, int height)
{
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_FOCUSED, false);
#ifndef NDEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	mainwindow = glfwCreateWindow(width, height, windowCaption.c_str(), nullptr, nullptr);
	if (mainwindow == nullptr)
	{
		cerr << "Unable to create window" << endl;
		return false;
	}

	glfwMakeContextCurrent(mainwindow);

	glfwSetWindowSizeCallback(mainwindow, resizeGLScene);
	glfwSetMouseButtonCallback(mainwindow, onMouseButton);
	glfwSetKeyCallback(mainwindow, onKey);

	// GLEW
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		cerr << "Could not initialize GLEW." << endl;
		return false;
	}

	return true;
}

void destroySDLWindow()
{
	glfwDestroyWindow(mainwindow);
	glfwTerminate();
}

void showPolygons(vector<string> args)
{
	global::polygonmode = args[1] == "true";
	if (global::polygonmode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void showCoordinateSystem(vector<string> args)
{
	global::coords = args[1] == "true";
}

void showNormals(vector<string> args)
{
	global::normals = args[1] == "true";
}

int main(int argc, char **argv) try
{
	if (!createSDLWindow(initialWindowWidth, initialWindowHeight))
		return -1;

	if (!initGL())
		return -1;

	//if(!initCL())
	//	return -1;

	resizeGLScene(mainwindow, initialWindowWidth, initialWindowHeight);

	ThreadedCommandConsole console;
	console.addCommand("polygons", regex("polygons (true|false)"), &showPolygons);
	console.addCommand("coords", regex("coords (true|false)"), &showCoordinateSystem);
	console.addCommand("normals", regex("normals (true|false)"), &showNormals);

	while (true) {
		glfwPollEvents();

		if (glfwGetKey(mainwindow, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(mainwindow))
			break;

		if (g_active) {
			timer.tick();
			update(timer.interval);
			render();
			glfwSwapBuffers(mainwindow);
			console.runHandlers();
		}
	}

	destroySDLWindow();

	return 0;
}
catch (const std::exception& e)
{
	cerr << e.what() << endl;
	return -2;
}
catch (...) {
	cerr << "Unknown exception" << endl;
	return -2;
}
