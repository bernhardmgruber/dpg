#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <noise/noise.h>
#include <iostream>

#undef main

#define WINDOW_HEIGHT 1024
#define WINDOW_WIDTH 768
#define WINDOW_CAPTION "noise"

using namespace noise;
using namespace std;

bool g_active = true;
bool g_done = false;
bool g_keys[SDLK_LAST] = {false};
bool g_fullscreen = false;

bool g_coords = false;



// Camera
#define LOOK_SENS 1.0f

float fXAngle = 0.0f;
float fYAngle = 0.0f;
float g_fZoom = -10.0f;
bool g_captureMouse = false;
int g_mouseOriginX;
int g_mouseOriginY;


void CaptureMouse(bool bCapture)
{
    g_captureMouse = bCapture;

    if(bCapture)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        g_mouseOriginX = x;
        g_mouseOriginY = y;
    }
}

void SetViewByMouse()
{
    if(g_captureMouse)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        // Update rotation based on mouse input
        fYAngle += LOOK_SENS * (float)(int)(g_mouseOriginX - x);

        // Correct z angle to interval [0;360]
        if(fYAngle >= 360.0f)
            fYAngle -= 360.0f;

        if(fYAngle < 0.0f)
            fYAngle += 360.0f;

        // Update up down view
        fXAngle += LOOK_SENS * (float)(int)(g_mouseOriginY - y);

        // Correct x angle to interval [0;360]
        if(fXAngle >= 360.0f)
            fXAngle -= 360.0f;

        if(fXAngle < 0.0f)
            fXAngle += 360.0f;

        // Reset cursor
        SDL_WarpMouse(g_mouseOriginX, g_mouseOriginY);
    }

    glRotatef(-fXAngle, 1.0f, 0.0f, 0.0f);
    glRotatef(-fYAngle, 0.0f, 1.0f, 0.0f);
}

void ResizeGLScene(int width, int height)
{
    if (!height)
    {
        height = 1;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (float)width/(float)height, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

const int CUBE_SIZE = 64;
float cube[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE];

void InitGL()
{
    glShadeModel(GL_SMOOTH);
    glClearColor(0.3f, 0.5f, 0.7f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_MULTISAMPLE);

    glEnable(GL_LIGHT0);

    module::Perlin perlin;

    // gen noise cube
    cout << "Generating noise cube ... ";
    for(int x = 0; x < CUBE_SIZE; x++)
        for(int y = 0; y < CUBE_SIZE; y++)
            for(int z = 0; z < CUBE_SIZE; z++)
                cube[x][y][z] = perlin.GetValue(x / 16.0, y / 16.0, z / 16.0);

    cout << "DONE" << endl;

    // create geometry using marching cubes
    cout << "Generating geometry ... ";


    cout << "DONE";
}

int DrawGLScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, g_fZoom);

    SetViewByMouse();

    glEnable(GL_LIGHTING);




    glDisable(GL_LIGHTING);

    if(g_coords)
    {
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glColor3f(1.0f,0.0f,0.0f); //red X+
        glVertex3i(5,0,0);
        glVertex3i(0,0,0);
        glColor3f(0.0f,1.0f,0.0f); //green Y+
        glVertex3i(0,5,0);
        glVertex3i(0,0,0);
        glColor3f(0.0f,0.0f,1.0f); //blue Z+
        glVertex3i(0,0,5);
        glVertex3i(0,0,0);
        glEnd();

        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glColor3f(0.0f,0.4f,0.0f); //green Y-
        glVertex3i(0,0,0);
        glVertex3i(0,-5,0);
        glColor3f(0.4f,0.0f,0.0f); //red X-
        glVertex3i(0,0,0);
        glVertex3i(-5,0,0);
        glColor3f(0.0f,0.0f,0.4f); //blue Z-
        glVertex3i(0,0,0);
        glVertex3i(0,0,-5);
        glEnd();
    }

    return true;
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

    InitGL();
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
        }
    case SDL_KEYUP:
        g_keys[event.key.keysym.sym] = false;
        return;
    case SDL_MOUSEBUTTONDOWN:
        switch(event.button.button)
        {
        case SDL_BUTTON_RIGHT:
            CaptureMouse(true);
            SDL_ShowCursor(SDL_DISABLE);
            return;
        case SDL_BUTTON_WHEELDOWN:
            g_fZoom--;
            return;
        case SDL_BUTTON_WHEELUP:
            g_fZoom++;
            return;
        default:
            return;
        }
    case SDL_MOUSEBUTTONUP:
        switch(event.button.button)
        {
        case SDL_BUTTON_RIGHT:
            CaptureMouse(false);
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
                    DrawGLScene();
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
