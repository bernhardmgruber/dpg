#include <cmath>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <iostream>

#include "Camera.h"

using namespace std;

extern bool g_keys[SDLK_LAST];

Camera::Camera()
{
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;

    yaw = 0.0f;
    pitch = 0.0f;

    moveSens = CAMERA_MOVE_SENS;
    lookSens = CAMERA_LOOK_SENS;
}

Vector3D Camera::GetPosition()
{
    return position;
}

void Camera::SetPosition(float fx, float fy, float fz)
{
    position.x = fx;
    position.y = fy;
    position.z = fz;
}

void Camera::SetPosition(Vector3D v)
{
    position = v;
}

Vector2D Camera::GetViewAngles()
{
    Vector2D vec;
    vec.x = pitch;
    vec.y = yaw;
    return vec;
}

Vector3D Camera::GetViewVector()
{
    Vector3D v;
    v.x = 1;
    v.y = 0;
    v.z = 0;

    // rotate pitch along -y
    v = RotateY(-pitch, v);

    // rotate yaw along z
    v = RotateZ(yaw, v);

    return v;
}

void Camera::SetViewAngles(float pitch, float yaw)
{
    this->pitch = pitch;
    this->yaw = yaw;
}

void Camera::SetPosition(Vector2D v)
{
    pitch = v.x;
    yaw = v.y;
}

float Camera::GetMoveSens()
{
    return moveSens;
}

void Camera::SetMoveSens(float moveSens)
{
    this->moveSens = moveSens;
}

void Camera::SetCaptureMouse(bool captureMouse)
{
    this->captureMouse = captureMouse;

    if(captureMouse)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        mouseOrigin.x = x;
        mouseOrigin.y = y;
    }
}

void Camera::Update(double interval)
{
    // else update camera object
    if(captureMouse)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        // Update rotation based on mouse input
        yaw += lookSens * (float)(int)(mouseOrigin.x - x);

        // Correct z angle to interval [0;360]
        if(yaw >= 360.0f)
            yaw -= 360.0f;

        if(yaw < 0.0f)
            yaw += 360.0f;

        // Update up down view
        pitch += lookSens * (float)(int)(mouseOrigin.y - y);

        // Correct x angle to interval [-90;90]
        if (pitch < -90.0f)
            pitch = -90.0f;

        if (pitch > 90.0f)
            pitch = 90.0f;

        // Reset cursor
        SDL_WarpMouse(mouseOrigin.x, mouseOrigin.y);
    }

    float tmpMoveSens = moveSens * interval;

    if (g_keys[SDLK_SPACE]) // UP
    {
        position.y += tmpMoveSens;
    }

    if (g_keys[SDLK_LCTRL]) // DOWN
    {
        position.y -= tmpMoveSens;
    }

    // TODO: If strafing and moving reduce speed to keep total move per frame constant
    if (g_keys[SDLK_w]) // FORWARD
    {
        position.x -= sin(DEGTORAD(yaw)) * tmpMoveSens;
        position.z -= cos(DEGTORAD(yaw)) * tmpMoveSens;
    }

    if (g_keys[SDLK_s]) // BACKWARD
    {
        position.x += sin(DEGTORAD(yaw)) * tmpMoveSens;
        position.z += cos(DEGTORAD(yaw)) * tmpMoveSens;
    }

    if (g_keys[SDLK_a]) // LEFT
    {
        position.x -= sin(DEGTORAD(yaw + 90.0f)) * tmpMoveSens;
        position.z -= cos(DEGTORAD(yaw + 90.0f)) * tmpMoveSens;
    }

    if (g_keys[SDLK_d]) // RIGHT
    {
        position.x -= sin(DEGTORAD(yaw - 90.0f)) * tmpMoveSens;
        position.z -= cos(DEGTORAD(yaw - 90.0f)) * tmpMoveSens;
    }
}

void Camera::Look()
{
    glRotatef(-pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-yaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-position.x, -position.y, -position.z);

    //cout << "Look: " << "Pitch " << pitch << " Yaw " << yaw << " Position " << position << endl;
}
