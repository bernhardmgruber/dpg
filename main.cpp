#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <CL/CL.h>
#include <iostream>
#include <fstream>

#include "Camera.h"
#include "Timer.h"
#include "World.h"

#undef main

#define WINDOW_HEIGHT 1024
#define WINDOW_WIDTH 768
#define WINDOW_CAPTION "noise"

using namespace std;

bool g_active = true;
bool g_done = false;
bool g_keys[SDLK_LAST] = {false};
bool g_fullscreen = false;

bool g_coords = false;
bool g_polygonmode = false;

World world;

cl_command_queue cmdqueue;
cl_kernel kernel;
cl_program program;
cl_context context;

void ResizeGLScene(int width, int height)
{
    if (height <= 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (float)width/(float)height, 0.1f, 1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

bool InitGL()
{
    glShadeModel(GL_SMOOTH);
    glClearColor(0.3f, 0.5f, 0.7f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_MULTISAMPLE);

    return true;
}

#define CHECK(error) if(error != CL_SUCCESS) { cout << "Error at line " << __LINE__ << ": " << error << endl; }

bool InitCL()
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

    // create a context to work with OpenCL
    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &error);
    CHECK(error);

    // read the kernel source
    ifstream sourceFile("main.cl");
    string buffer((istreambuf_iterator<char>(sourceFile)), istreambuf_iterator<char>());
    sourceFile.close();
    const char* source = buffer.c_str();

    // create an OpenCL program from the source code
    program = clCreateProgramWithSource(context, 1, &source, nullptr, &error);
    CHECK(error);

    // build the program
    error = clBuildProgram(program, 1, &device, "-w", nullptr, nullptr);
    if(error != CL_SUCCESS)
    {
        cout << "##### Error building CL program #####" << endl;

        // get the error log size
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);

        // allocate enough space and get the log
        char* log = new char[logSize + 1];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, nullptr);
        log[logSize] = '\0';

        // print the build log and delete it
        cout << log << endl;
        delete[] log;

        return false;
    }

    // set the entry point for an OpenCL kernel
    kernel = clCreateKernel(program, "MarchChunk", &error);
    CHECK(error);

    // create a new command queue, where kernels can be executed
    cmdqueue = clCreateCommandQueue(context, device, 0, &error);
    CHECK(error);

    return true;
}

void Update(double interval)
{
    Camera::GetInstance().Update(interval);
    world.Update();
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    Camera::GetInstance().Look();
    world.Render();

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

bool CreateSDLWindow(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "Could not initialize SDL\n");
        return false;
    };

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // enable 2x anti-aliasing
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    unsigned int flags = SDL_OPENGL | SDL_RESIZABLE;
    if(g_fullscreen)
        flags |= SDL_FULLSCREEN;

    SDL_SetVideoMode(height, width, 0, flags);

    SDL_WM_SetCaption(WINDOW_CAPTION, WINDOW_CAPTION);

    if(!InitGL())
        return false;
    //if(!InitCL())
    //    return false;

    ResizeGLScene(height, width);

    return true;
}

void ProcessEvent(SDL_Event event)
{
    switch(event.type)
    {
        case SDL_QUIT:
            g_done = true;
            return;
        case SDL_KEYDOWN:
            g_keys[event.key.keysym.sym] = true;
            switch(event.key.keysym.sym)
            {
                case SDLK_c:
                    g_coords = !g_coords;
                    return;
                default:
                    return;
                case SDLK_p:
                    g_polygonmode = !g_polygonmode;
                    if(g_polygonmode)
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    else
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        case SDL_KEYUP:
            g_keys[event.key.keysym.sym] = false;
            return;
        case SDL_MOUSEBUTTONDOWN:
            switch(event.button.button)
            {
                case SDL_BUTTON_RIGHT:
                    Camera::GetInstance().SetCaptureMouse(true);
                    SDL_ShowCursor(SDL_DISABLE);
                    return;
                default:
                    return;
            }
        case SDL_MOUSEBUTTONUP:
            switch(event.button.button)
            {
                case SDL_BUTTON_RIGHT:
                    Camera::GetInstance().SetCaptureMouse(false);
                    SDL_ShowCursor(SDL_ENABLE);
                    return;
                default:
                    return;
            }
        case SDL_VIDEORESIZE:
            ResizeGLScene(event.resize.w, event.resize.h);
            return;
    }
}

int main(int argc, char **argv )
{
    if (!CreateSDLWindow(WINDOW_WIDTH, WINDOW_HEIGHT))
        return 0;

    Timer timer;

    while (!g_done)
    {
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            ProcessEvent(event);
        }
        else
        {
            if (g_active)
            {
                if (g_keys[SDLK_ESCAPE])
                {
                    g_done = true;
                }
                else
                {
                    timer.Tick();
                    Update(timer.interval);
                    Render();
                    SDL_GL_SwapBuffers();
                }
            }

            if (g_keys[SDLK_F1])
            {
                g_keys[SDLK_F1] = false;
                SDL_Quit();
                g_fullscreen =! g_fullscreen;

                if (!CreateSDLWindow(WINDOW_WIDTH, WINDOW_HEIGHT))
                    return 0;
            }
        }
    }

    SDL_Quit();
    return 0;
}
