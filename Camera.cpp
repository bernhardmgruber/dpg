#include <cmath>
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Camera.h"

using namespace std;
using namespace glm;

extern SDL_Window* mainwindow;
extern GLuint uModelViewProjectionMatrixLocation;
extern GLuint uModelViewMatrixLocation;
extern mat4 projectionMatrix;

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

Camera& Camera::GetInstance()
{
    static Camera camera;

    return camera;
}

Vector3F Camera::GetPosition()
{
    return position;
}

void Camera::SetPosition(float fx, float fy, float fz)
{
    position.x = fx;
    position.y = fy;
    position.z = fz;
}

void Camera::SetPosition(Vector3F v)
{
    position = v;
}

Vector2F Camera::GetViewAngles()
{
    Vector2F vec;
    vec.x = pitch;
    vec.y = yaw;
    return vec;
}

Vector3F Camera::GetViewVector()
{
    Vector3F v;
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

void Camera::SetPosition(Vector2F v)
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
        SDL_WarpMouseInWindow(mainwindow, mouseOrigin.x, mouseOrigin.y);
    }

    double tmpMoveSens = moveSens * interval;

    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_SPACE]) // UP
    {
        position.y += (float)tmpMoveSens;
    }

    if (keys[SDL_SCANCODE_LCTRL]) // DOWN
    {
        position.y -= (float)tmpMoveSens;
    }

    // TODO: If strafing and moving reduce speed to keep total move per frame constant
    if (keys[SDL_SCANCODE_W]) // FORWARD
    {
        position.x -= (float)(sin(DEGTORAD(yaw)) * tmpMoveSens);
        position.z -= (float)(cos(DEGTORAD(yaw)) * tmpMoveSens);
    }

    if (keys[SDL_SCANCODE_S]) // BACKWARD
    {
        position.x += (float)(sin(DEGTORAD(yaw)) * tmpMoveSens);
        position.z += (float)(cos(DEGTORAD(yaw)) * tmpMoveSens);
    }

    if (keys[SDL_SCANCODE_A]) // LEFT
    {
        position.x -= (float)(sin(DEGTORAD(yaw + 90.0f)) * tmpMoveSens);
        position.z -= (float)(cos(DEGTORAD(yaw + 90.0f)) * tmpMoveSens);
    }

    if (keys[SDL_SCANCODE_D]) // RIGHT
    {
        position.x -= (float)(sin(DEGTORAD(yaw - 90.0f)) * tmpMoveSens);
        position.z -= (float)(cos(DEGTORAD(yaw - 90.0f)) * tmpMoveSens);
    }
}

void Camera::Look()
{
    mat4 modelViewMatrix = rotate(mat4(1.0), -pitch, vec3(1.0, 0.0, 0.0));
    modelViewMatrix = rotate(modelViewMatrix, -yaw, vec3(0.0, 1.0, 0.0));
    modelViewMatrix = translate(modelViewMatrix, vec3(-position.x, -position.y, -position.z));

    glUniformMatrix4fv(uModelViewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix * modelViewMatrix));
    glUniformMatrix4fv(uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));

    //cout << "Look: " << "Pitch " << pitch << " Yaw " << yaw << " Position " << position << endl;
}
