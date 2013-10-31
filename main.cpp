#include "gl.h"
#include <SDL.h>
#include <SDL_opengl.h>
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
static bool g_done = false;
static bool g_fullscreen = false;

SDL_Window* mainwindow;
SDL_GLContext maincontext;

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

void resizeGLScene(int width, int height)
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
    SDL_SetWindowTitle(mainwindow, caption.str().c_str());

    Camera::getInstance().update(interval);
    world.update();
}

void render()
{
    // clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update shader states
    mat4 viewMatrix = Camera::getInstance().getViewMatrix();
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

bool createSDLWindow(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        cerr << "Could not initialize SDL." << endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // enable 2x anti-aliasing
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    unsigned int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (g_fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;

    mainwindow = SDL_CreateWindow(windowCaption.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (mainwindow == nullptr)
    {
        cerr << "Unable to create window" << endl;
        return false;
    }

    maincontext = SDL_GL_CreateContext(mainwindow);

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
    SDL_GL_DeleteContext(maincontext);
    SDL_DestroyWindow(mainwindow);
    SDL_Quit();
}

void processEvent(SDL_Event event)
{
    switch (event.type)
    {
    case SDL_QUIT:
        g_done = true;
        return;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
            //case SDLK_c:
            //	g_coords = !g_coords;
            //	return;
            //case SDLK_p:
            //	g_polygonmode = !g_polygonmode;
            //	if(g_polygonmode)
            //		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            //	else
            //		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            //	return;
        case SDLK_ESCAPE:
            g_done = true;
            return;

        case SDLK_F1:
            SDL_Quit();
            g_fullscreen = !g_fullscreen;

            if (!createSDLWindow(initialWindowWidth, initialWindowHeight))
                g_done = true;
            return;
        default:
            return;
        }
    case SDL_KEYUP:
        return;
    case SDL_MOUSEBUTTONDOWN:
        switch (event.button.button)
        {
        case SDL_BUTTON_RIGHT:
            Camera::getInstance().setCaptureMouse(true);
            //SDL_ShowCursor(SDL_DISABLE);
            return;
        default:
            return;
        }
    case SDL_MOUSEBUTTONUP:
        switch (event.button.button)
        {
        case SDL_BUTTON_RIGHT:
            Camera::getInstance().setCaptureMouse(false);
            //SDL_ShowCursor(SDL_ENABLE);
            return;
        default:
            return;
        }
    case SDL_WINDOWEVENT:
        switch (event.window.event)
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

int main(int argc, char **argv)
{
    if (!createSDLWindow(initialWindowWidth, initialWindowHeight))
        return -1;

    try
    {
        if (!initGL())
            return -1;
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
        return -2;
    }
    //if(!initCL())
    //	return -1;

    resizeGLScene(initialWindowWidth, initialWindowHeight);

    ThreadedCommandConsole console;
    console.addCommand("polygons", regex("polygons (true|false)"), &showPolygons);
    console.addCommand("coords", regex("coords (true|false)"), &showCoordinateSystem);
    console.addCommand("normals", regex("normals (true|false)"), &showNormals);

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
            console.runHandlers();
        }
    }

    destroySDLWindow();

    return 0;
}
